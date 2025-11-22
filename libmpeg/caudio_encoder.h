/******************************************************************************
 *  Copyright (c) 2025 The CRTC project authors . All Rights Reserved.
 *
 *  Please visit https://chensongpoixs.github.io for detail
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 ******************************************************************************/
/***********************************************************************************************
created: 		2025-05-02

author:			chensong

purpose:		audio encoder


节目关联表


输赢不重要，答案对你们有什么意义才重要。

光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
************************************************************************************************/

#ifndef _C_AUDIO_ENCODER_
#define _C_AUDIO_ENCODER_


#include <cstdint>
#include <memory>

#include "libmedia_transfer_protocol/libmpeg/cstream_writer.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>


#include <unordered_map>

#include <functional>
#include <memory>
#include "libmedia_transfer_protocol/libmpeg/cpsi_writer.h"
#include "libmedia_transfer_protocol/libmpeg/cmpeg_type.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/libmpeg/packet.h"
#include "libmedia_transfer_protocol/libmpeg/caudio_demux.h"
namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{
		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief 音频编码器类（Audio Encoder）
		*  
		*  AudioEncoder类用于将音频数据编码为MPEG-TS格式。它支持多种音频格式（如AAC、MP3等），
		*  将音频包转换为TS包的PES（Packetized Elementary Stream）格式并写入TS流。
		*  
		*  音频编码流程（Audio Encoding Process）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  1. Receive Audio Packet (AAC/MP3 raw data)                    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  2. Demux Audio (AudioDemux extracts samples)                  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  3. Encode to PES Format                                       |
		*   |     - AAC: EncodeAAC()                                         |
		*   |     - MP3: EncodeMP3()                                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  4. Write PES to TS Packets                                    |
		*   |     - WriteAudioPes()                                          |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  PES Packet 数据结构（PES Packet Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  packet_start_code_prefix (24 bits = 0x000001)                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  stream_id (8 bits)                                            |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PES_packet_length (16 bits)                                   |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  '10' | PES_scrambling_control | PES_priority | data_alignment|
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  copyright | original_or_copy | PTS_DTS_flags | ESCR_flag     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  ES_rate_flag | DSM_trick_mode_flag | additional_copy_info    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PES_CRC_flag | PES_extension_flag | PES_header_data_length   |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PTS (33 bits, if PTS_DTS_flags='11' or '10')                 |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  DTS (33 bits, if PTS_DTS_flags='11')                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        PES Payload (Audio Data)                :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  TS Packet 分片格式（TS Packet Fragmentation Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  TS Packet Header (4 bytes)                                    |
		*   |  - sync_byte = 0x47                                            |
		*   |  - PID = pid_                                                  |
		*   |  - payload_unit_start_indicator = 1 for first packet          |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PES Packet Data (up to 184 bytes per TS packet)              |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        ... more TS packets ...                  :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note AudioEncoder支持AAC和MP3两种音频格式
		*  @note 音频数据通过AudioDemux进行解复用，提取音频采样
		*  @note PES包会被分片到多个188字节的TS包中发送
		*  
		*  使用示例：
		*  @code
		*  AudioEncoder encoder;
		*  encoder.SetPid(0x1001);
		*  encoder.SetStreamType(kTsStreamAudioAAC);
		*  encoder.EnodeAudio(stream_writer, audio_packet, dts);
		*  @endcode
		*/
		class AudioEncoder
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建AudioEncoder实例。使用默认构造函数，所有成员变量使用默认值。
			*  
			*  初始化说明：
			*  - pid_: 初始化为0xE000（保留值），需要通过SetPid()设置
			*  - type_: 初始化为kTsStreamReserved，需要通过SetStreamType()设置
			*  - cc_: 初始化为-1，表示连续性计数器尚未初始化
			*  - demux_: 初始化AudioDemux实例
			*  
			*  @note 使用默认构造函数，需要在编码前设置PID和流类型
			*/
			AudioEncoder() = default;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理AudioEncoder实例。使用默认析构函数，自动释放成员变量。
			*  
			*  @note 使用默认析构函数，自动释放资源
			*/
			~AudioEncoder() = default;
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 编码音频（Encode Audio）
			*  
			*  该方法用于将音频包编码为MPEG-TS格式。它会解复用音频数据，
			*  根据流类型选择相应的编码方法（AAC或MP3），然后写入PES包格式。
			*  
			*  编码流程（Encoding Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Demux Audio Packet                                        |
			*   |     - AudioDemux::Demux() extracts samples                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Check Stream Type                                          |
			*   |     - If AAC: Call EncodeAAC()                                 |
			*   |     - If MP3: Call EncodeMP3()                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Write PES Packet                                           |
			*   |     - WriteAudioPes() writes PES to TS packets                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  音频数据格式（Audio Data Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Audio Packet Header (format dependent)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                        Audio Samples                           :
			*   |  - AAC: ADTS frame or raw AAC data                            |
			*   |  - MP3: MP3 frame data                                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param writer StreamWriter指针，用于写入TS包数据，不能为空
			*  @param data 指向音频包的共享指针，包含音频原始数据
			*  @param dts 解码时间戳（Decode Time Stamp），单位为90KHz时钟或毫秒
			*  @return 返回编码结果，0表示成功，其他值表示失败
			*  @note 音频包会通过AudioDemux进行解复用
			*  @note 根据流类型（AAC/MP3）选择相应的编码方法
			*  @note PES包会被分片到多个TS包中发送
			*  
			*  使用示例：
			*  @code
			*  auto audio_packet = std::make_shared<Packet>();
			*  int64_t dts = 90000;  // 1 second in 90KHz clock
			*  encoder.EnodeAudio(stream_writer, audio_packet, dts);
			*  @endcode
			*/
			int32_t  EnodeAudio(StreamWriter* writer, std::shared_ptr<Packet> & data, int64_t dts);
			
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置PID（Set PID）
			*  
			*  该方法用于设置音频流的PID（Packet Identifier）。PID用于标识
			*  TS包所属的音频流，每个音频流都有唯一的PID。
			*  
			*  PID格式（PID Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PID[13:8] | TEI|PUSI|TP | PID[7:0]                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |            PID: 13位包标识符 (0x0000-0x1FFF)                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param pid PID值，范围0x0000-0x1FFF（13位）
			*  @note PID用于标识音频流的TS包
			*  @note PID应该与PMT表中的elementary_PID一致
			*  
			*  使用示例：
			*  @code
			*  encoder.SetPid(0x1001);
			*  @endcode
			*/
			void SetPid(uint16_t pid);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置流类型（Set Stream Type）
			*  
			*  该方法用于设置音频流的类型。流类型决定了编码方法（AAC或MP3）和
			*  PMT表中的stream_type字段值。
			*  
			*  流类型说明（Stream Type Description）：
			*  - kTsStreamAudioAAC: AAC音频流，stream_type = 0x0F
			*  - kTsStreamAudioMP3: MP3音频流，stream_type = 0x03
			*  - kTsStreamReserved: 保留类型
			*  
			*  @param type 音频流类型枚举值（TsStreamType）
			*  @note 流类型决定了编码方法的选择
			*  @note 流类型应该与PMT表中的stream_type一致
			*  
			*  使用示例：
			*  @code
			*  encoder.SetStreamType(kTsStreamAudioAAC);
			*  @endcode
			*/
			void SetStreamType(TsStreamType type);
			

		private:

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 编码AAC音频（Encode AAC Audio）
			*  
			*  该方法用于将AAC音频采样列表编码为PES包格式。AAC音频数据会被
			*  封装到PES包中，然后分片到多个TS包中发送。
			*  
			*  AAC PES包格式（AAC PES Packet Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PES Header (variable length)                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                        AAC Frame Data                          :
			*   |  - ADTS header (if present)                                    |
			*   |  - AAC raw data                                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param writer StreamWriter指针，用于写入TS包数据
			*  @param sample_list 音频采样列表引用，包含多个AAC音频采样
			*  @param pts 表示时间戳（Presentation Time Stamp），单位为90KHz时钟或毫秒
			*  @return 返回编码结果，0表示成功，其他值表示失败
			*  @note AAC音频采样会被封装到PES包中
			*  @note PES包会被分片到多个TS包中发送
			*  
			*  使用示例：
			*  @code
			*  std::list<SampleBuf> aac_samples = {...};
			*  encoder.EncodeAAC(stream_writer, aac_samples, pts);
			*  @endcode
			*/
			int32_t  EncodeAAC(StreamWriter*writer, std::list<SampleBuf>&sample_list, int64_t pts);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 编码MP3音频（Encode MP3 Audio）
			*  
			*  该方法用于将MP3音频采样列表编码为PES包格式。MP3音频数据会被
			*  封装到PES包中，然后分片到多个TS包中发送。
			*  
			*  MP3 PES包格式（MP3 PES Packet Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PES Header (variable length)                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                        MP3 Frame Data                          :
			*   |  - MP3 frame header                                            |
			*   |  - MP3 frame data                                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param writer StreamWriter指针，用于写入TS包数据
			*  @param smpale_list 音频采样列表引用，包含多个MP3音频采样
			*  @param pts 表示时间戳（Presentation Time Stamp），单位为90KHz时钟或毫秒
			*  @return 返回编码结果，0表示成功，其他值表示失败
			*  @note MP3音频采样会被封装到PES包中
			*  @note PES包会被分片到多个TS包中发送
			*  
			*  使用示例：
			*  @code
			*  std::list<SampleBuf> mp3_samples = {...};
			*  encoder.EncodeMP3(stream_writer, mp3_samples, pts);
			*  @endcode
			*/
			int32_t  EncodeMP3(StreamWriter*writer, std::list<SampleBuf>&smpale_list, int64_t pts);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 写入音频PES包（Write Audio PES Packet）
			*  
			*  该方法用于将音频采样列表封装为PES包格式并写入TS包。
			*  PES包会被分片到多个188字节的TS包中发送。
			*  
			*  PES包写入流程（PES Packet Write Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Build PES Packet Header                                    |
			*   |     - packet_start_code_prefix = 0x000001                      |
			*   |     - stream_id = 0xC0 (audio stream)                          |
			*   |     - PTS/DTS fields                                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Concatenate Audio Samples                                  |
			*   |     - Append all samples to PES payload                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Fragment PES to TS Packets                                 |
			*   |     - First TS packet: payload_unit_start = 1                  |
			*   |     - Subsequent packets: payload_unit_start = 0               |
			*   |     - Each TS packet: 188 bytes                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  PES包分片格式（PES Packet Fragmentation Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PES Packet (variable length)                                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  TS Packet 0: [Header(4)] + [PES Data (184 bytes)]           |
			*   |  payload_unit_start = 1                                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  TS Packet 1: [Header(4)] + [PES Data (184 bytes)]           |
			*   |  payload_unit_start = 0                                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                        ... more TS packets ...                  :
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param writer StreamWriter指针，用于写入TS包数据
			*  @param smpale_list 音频采样列表引用，包含多个音频采样
			*  @param payload_size PES包负载大小，单位为字节
			*  @param dts 解码时间戳（Decode Time Stamp），单位为90KHz时钟或毫秒
			*  @return 返回写入结果，0表示成功，其他值表示失败
			*  @note PES包会被分片到多个188字节的TS包中发送
			*  @note 第一个TS包的payload_unit_start指示符设置为1
			*  @note 连续性计数器（continuity_counter）会自动递增
			*  
			*  使用示例：
			*  @code
			*  std::list<SampleBuf> samples = {...};
			*  int32_t payload_size = 1024;
			*  int64_t dts = 90000;
			*  encoder.WriteAudioPes(stream_writer, samples, payload_size, dts);
			*  @endcode
			*/
			int32_t  WriteAudioPes(StreamWriter*writer, std::list<SampleBuf>&smpale_list, int32_t payload_size, int64_t dts);


		private:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 音频流PID（Audio Stream PID）
			*  
			*  该成员变量用于存储音频流的PID（Packet Identifier）。PID用于标识
			*  TS包所属的音频流，每个音频流都有唯一的PID。
			*  
			*  PID格式（PID Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PID[13:8] | TEI|PUSI|TP | PID[7:0]                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |            PID: 13位包标识符 (0x0000-0x1FFF)                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0xE000（保留值），需要通过SetPid()设置
			*  @note PID应该与PMT表中的elementary_PID一致
			*/
			uint16_t   pid_{ 0XE000 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 音频流类型（Audio Stream Type）
			*  
			*  该成员变量用于存储音频流的类型。流类型决定了编码方法（AAC或MP3）和
			*  PMT表中的stream_type字段值。
			*  
			*  流类型说明（Stream Type Description）：
			*  - kTsStreamAudioAAC: AAC音频流，stream_type = 0x0F
			*  - kTsStreamAudioMP3: MP3音频流，stream_type = 0x03
			*  - kTsStreamReserved: 保留类型
			*  
			*  @note 初始值为kTsStreamReserved，需要通过SetStreamType()设置
			*  @note 流类型决定了编码方法的选择
			*/
			TsStreamType  type_{ kTsStreamReserved };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 连续性计数器（Continuity Counter）
			*  
			*  该成员变量用于存储TS包的连续性计数器。连续性计数器用于检测TS包丢失，
			*  每个具有相同PID的TS包都会有连续性计数器，按顺序从0到15循环。
			*  
			*  连续性计数器格式（Continuity Counter Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  TSC | AFC | CC (4 bits)                                       |
			*   |  00  | 01  | continuity_counter (0-15)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为-1，表示尚未初始化。第一次使用时会被设置为0
			*  @note 每个TS包发送后，连续性计数器会自动递增
			*/
			int8_t    cc_{ -1 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 音频解复用器（Audio Demuxer）
			*  
			*  该成员变量用于解复用音频数据。AudioDemux负责从音频包中提取音频采样，
			*  将原始音频数据解析为SampleBuf列表，供编码器使用。
			*  
			*  解复用流程（Demux Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Audio Packet (raw data)                                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  AudioDemux::Demux() extracts samples                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  SampleBuf List:                                               |
			*   |  - SampleBuf 0 (AAC/MP3 frame)                                 |
			*   |  - SampleBuf 1 (AAC/MP3 frame)                                 |
			*   |  - ...                                                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note AudioDemux用于从音频包中提取音频采样
			*  @note 支持AAC和MP3两种格式的解复用
			*/
			AudioDemux     demux_;
		};
	}
}

#endif // 