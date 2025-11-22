
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

purpose:		video encoder 
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

#ifndef _C_VIDEO_ENCODER_
#define _C_VIDEO_ENCODER_


#include <cstdint>
#include <memory>

#include "cstream_writer.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <memory>
#include "cpsi_writer.h"
#include "libmedia_transfer_protocol/libmpeg/cpsi_writer.h"
#include "libmedia_transfer_protocol/libmpeg/cstream_writer.h"
#include "libmedia_transfer_protocol/libmpeg/cmpeg_type.h"
#include "libmedia_transfer_protocol/libmpeg/cvideo_demux.h"
#include "libmedia_transfer_protocol/libmpeg/cpsi_writer.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/libmpeg/packet.h"
namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{
		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief 视频编码器类（Video Encoder）
		*  
		*  VideoEncoder类用于将视频编码数据（如H264 NALU）封装为MPEG-TS格式。
		*  它负责将视频帧数据编码为PES包，然后封装为TS包写入到TS流中。
		*  
		*  视频编码流程说明：
		*  - 视频编码器接收H264 NALU数据（如SPS、PPS、IDR、Slice等）
		*  - 将NALU数据封装为PES（Packetized Elementary Stream）包
		*  - PES包包含PTS/DTS时间戳和视频数据
		*  - PES包被分片到多个TS（Transport Stream）包中
		*  - TS包写入到TS流中，供客户端接收和播放
		*  
		*  视频编码数据结构（Video Encoding Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        H264 NALU Data                           |
		*   |  (SPS, PPS, IDR, Slice, etc.)                                   |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PES Packet Encapsulation:                                      |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | PES Header (PTS, DTS, stream_id=0xE0)                      | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | Video Data (H264 NALU with start code)                     | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  TS Packet Encapsulation:                                       |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | TS Header (PID, continuity_counter)                        | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | PES Packet Data (fragmented across multiple TS packets)    | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  PES包数据结构（PES Packet Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  packet_start_code_prefix (0x000001)                           |
		*   |  (24 bits)                                                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  stream_id=0xE0 |  PES_packet_length                           |
		*   |  (8 bits)        |  (16 bits)                                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  marker | scrambling | priority | data_alignment | copyright   |
		*   |  bits   |  control   |          |  indicator     |             |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  original_or_copy | PTS_DTS_flags=11 | ESCR_flag | ES_rate_flag|
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  DSM_trick_mode_flag | additional_copy_info | CRC_flag        |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PES_header_data_length                                        |
		*   |  (8 bits)                                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PTS (33 bits)                                                 |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  DTS (33 bits)                                                 |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Video Data (H264 NALU with start code 0x00000001)            |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  H264 NALU格式（H264 NALU Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  start_code (0x00000001)                                       |
		*   |  (32 bits)                                                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  forbidden_zero_bit | NRI | NALU_type                          |
		*   |  (1 bit=0)          | (2) | (5 bits)                           |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  NALU payload data (variable)                                   |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note VideoEncoder负责将视频编码数据封装为MPEG-TS格式
		*  @note 支持H264视频编码格式
		*  @note 自动处理SPS/PPS插入和start code添加
		*  
		*  使用示例：
		*  @code
		*  VideoEncoder encoder;
		*  encoder.SetPid(0x1011);
		*  encoder.SetStreamType(kTsStreamH264);
		*  encoder.EncodeVideo(writer, is_key_frame, packet, dts);
		*  @endcode
		*/
		class VideoEncoder
		{
		public:

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建VideoEncoder实例。使用默认构造函数，所有成员变量使用默认值。
			*  
			*  初始化说明：
			*  - pid_: 初始化为0xE000（保留值），需要通过SetPid()设置正确的PID
			*  - type_: 初始化为kTsStreamReserved（保留类型），需要通过SetStreamType()设置
			*  - cc_: 初始化为-1，表示连续性计数器尚未初始化
			*  - startcode_inserted_: 初始化为false，表示尚未插入start code
			*  - sps_pps_appended_: 初始化为false，表示尚未附加SPS/PPS
			*  
			*  @note 使用默认构造函数，需要在编码前设置PID和流类型
			*/
			VideoEncoder() = default;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理VideoEncoder实例。使用默认析构函数，自动释放成员变量。
			*  
			*  @note 使用默认析构函数，自动释放资源
			*/
			~VideoEncoder() = default;


		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 编码视频数据（Encode Video Data）
			*  
			*  该方法用于将视频编码数据封装为MPEG-TS格式并写入TS流。
			*  它会处理H264 NALU数据，插入start code，封装为PES包，然后分片到TS包。
			*  
			*  编码流程（Encoding Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Parse Packet data (extract H264 NALU)                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Check if key frame (IDR frame)                            |
			*   |     - If yes, ensure SPS/PPS are included                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Insert start code (0x00000001) if needed                  |
			*   |     - Check startcode_inserted_ flag                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. Build PES packet header (PTS, DTS, stream_id=0xE0)        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  5. Encode AVC (H264) data to PES packet                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  6. Write PES packet to TS stream (fragmented to TS packets)  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param writer StreamWriter指针，用于写入TS包数据，不能为空
			*  @param key 是否为关键帧（IDR帧），true表示关键帧
			*  @param data 指向Packet对象的共享指针，包含视频编码数据
			*  @param dts 解码时间戳（Decode Time Stamp），单位为90KHz时钟单位
			*  @return 返回0表示成功，其他值表示编码失败
			*  @note 关键帧（IDR帧）会自动包含SPS/PPS
			*  @note 会自动处理start code插入
			*  @note PTS和DTS用于视频帧的时间同步
			*  
			*  使用示例：
			*  @code
			*  auto packet = std::make_shared<Packet>();
			*  // ... 填充packet数据 ...
			*  encoder.EncodeVideo(writer, is_key_frame, packet, dts);
			*  @endcode
			*/
			int32_t   EncodeVideo(StreamWriter *writer, bool key, std::shared_ptr<Packet> & data , int64_t dts);
			
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置PID（Set PID）
			*  
			*  该方法用于设置视频流的PID（Packet Identifier）。PID用于标识
			*  视频流TS包，客户端通过PID过滤对应的TS包。
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
			*  @note PID用于标识视频流的TS包
			*  @note PID应该在编码前设置，通常由PMT表指定
			*  
			*  使用示例：
			*  @code
			*  encoder.SetPid(0x1011);  // 设置视频流PID
			*  @endcode
			*/
			void SetPid(uint16_t pid);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置流类型（Set Stream Type）
			*  
			*  该方法用于设置视频流的类型。流类型标识了视频编码格式，
			*  如H264、H265等。
			*  
			*  流类型说明（Stream Type Description）：
			*  - kTsStreamH264: H264视频流（AVC）
			*  - kTsStreamH265: H265视频流（HEVC）
			*  - kTsStreamMpeg2Video: MPEG-2视频流
			*  
			*  流类型格式（Stream Type Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  stream_type                                                  |
			*   |  (8 bits)                                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  0x1B: H.264 Video (AVC)                                      |
			*   |  0x24: H.265 Video (HEVC)                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param type 流类型枚举值，标识视频编码格式
			*  @note 流类型应该在编码前设置
			*  @note 流类型用于PMT表中的stream_type字段
			*  
			*  使用示例：
			*  @code
			*  encoder.SetStreamType(kTsStreamH264);  // 设置H264流类型
			*  @endcode
			*/
			void SetStreamType(TsStreamType type);

		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 编码AVC数据（Encode AVC Data）
			*  
			*  该方法用于将H264 NALU数据封装为PES包并写入TS流。
			*  AVC（Advanced Video Coding）是H264的别名。
			*  
			*  AVC编码流程（AVC Encoding Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Process sample_list (H264 NALU list)                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Insert start code if needed                                |
			*   |     - Check startcode_inserted_ flag                          |
			*   |     - Insert 0x00000001 before each NALU                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Build PES packet with PTS                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. Write PES packet to TS stream                             |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param writer StreamWriter指针，用于写入TS包数据
			*  @param sample_list H264 NALU数据列表，包含多个NALU单元
			*  @param key 是否为关键帧（IDR帧），true表示关键帧
			*  @param pts 表示时间戳（Presentation Time Stamp），单位为90KHz时钟单位
			*  @return 返回0表示成功，其他值表示编码失败
			*  @note 该方法会处理多个NALU单元，将它们封装在一个PES包中
			*  @note 会自动插入start code（0x00000001）
			*  
			*  使用示例：
			*  @code
			*  std::list<SampleBuf> nalu_list;
			*  // ... 填充NALU数据 ...
			*  encoder.EncodeAvc(writer, nalu_list, is_key_frame, pts);
			*  @endcode
			*/
			int32_t   EncodeAvc(StreamWriter*writer, std::list<SampleBuf>& sample_list, bool key, int64_t pts);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 插入AVC Start Code（Insert AVC Start Code）
			*  
			*  该方法用于在H264 NALU数据前插入start code（0x00000001）。
			*  Start code用于标识NALU的开始，是H264标准要求的。
			*  
			*  Start Code格式（Start Code Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  start_code (0x00000001)                                      |
			*   |  (32 bits)                                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  forbidden_zero_bit | NRI | NALU_type                          |
			*   |  (1 bit=0)          | (2) | (5 bits)                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NALU payload data                                             |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  Start Code插入流程（Start Code Insertion Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Check if start code already inserted                       |
			*   |     - If yes, skip insertion                                   |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. For each NALU in sample_list:                              |
			*   |     - Check if NALU starts with 0x00000001                     |
			*   |     - If not, insert 0x00000001 before NALU                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Update startcode_inserted_ flag                            |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param sample_list H264 NALU数据列表，会在适当位置插入start code
			*  @param startcode_inserted 输出参数，指示是否已插入start code
			*  @return 返回0表示成功，其他值表示失败
			*  @note Start code 0x00000001用于标识NALU的开始
			*  @note 如果NALU已有start code，则不会重复插入
			*  
			*  使用示例：
			*  @code
			*  std::list<SampleBuf> nalu_list;
			*  bool inserted = false;
			*  encoder.AvcInsertStartCode(nalu_list, inserted);
			*  @endcode
			*/
			int32_t   AvcInsertStartCode(std::list<SampleBuf> & sample_list, bool &startcode_inserted);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 写入视频PES包（Write Video PES Packet）
			*  
			*  该方法用于将视频数据封装为PES包并写入TS流。PES包包含
			*  PTS/DTS时间戳和视频数据，然后被分片到多个TS包中。
			*  
			*  视频PES包格式（Video PES Packet Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  packet_start_code_prefix (0x000001)                           |
			*   |  (24 bits)                                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  stream_id=0xE0 |  PES_packet_length                           |
			*   |  (8 bits)        |  (16 bits)                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PTS_DTS_flags=11 | PES_header_data_length                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PTS (33 bits)                                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  DTS (33 bits)                                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Video Data (H264 NALU with start code)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  PES包分片到TS包流程（PES Packet Fragmentation Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Build PES packet header (PTS, DTS)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Calculate PES packet total size                            |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Fragment PES packet to TS packets (188 bytes each)        |
			*   |     - First TS packet: payload_unit_start = 1                  |
			*   |     - Subsequent packets: payload_unit_start = 0               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. Write TS packets to StreamWriter                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param writer StreamWriter指针，用于写入TS包数据，不能为空
			*  @param result 视频数据列表，包含H264 NALU数据（已插入start code）
			*  @param payload_size PES包负载数据大小，单位为字节
			*  @param pts 表示时间戳（Presentation Time Stamp），单位为90KHz时钟单位
			*  @param dts 解码时间戳（Decode Time Stamp），单位为90KHz时钟单位
			*  @param key 是否为关键帧（IDR帧），1表示关键帧
			*  @return 返回0表示成功，其他值表示写入失败
			*  @note PES包会被自动分片到多个188字节的TS包中
			*  @note 第一个TS包的payload_unit_start指示符设置为1
			*  @note PTS和DTS用于视频帧的时间同步
			*  
			*  使用示例：
			*  @code
			*  std::list<SampleBuf> video_data;
			*  // ... 填充视频数据 ...
			*  encoder.WriteVideoPes(writer, video_data, payload_size, pts, dts, is_key_frame);
			*  @endcode
			*/
			int32_t   WriteVideoPes(StreamWriter * writer, std::list<SampleBuf> & result, int32_t payload_size, 
				int64_t pts, int64_t  dts, int32_t key);

		private:

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 包标识符（Packet Identifier）
			*  
			*  该成员变量用于存储视频流的PID（Packet Identifier）。PID用于标识
			*  视频流TS包，客户端通过PID过滤对应的TS包。
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
			*  @note 初始值为0xE000（保留值），需要通过SetPid()设置正确的PID
			*  @note PID用于标识视频流的TS包，通常由PMT表指定
			*/
			uint16_t  pid_{ 0XE000 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 流类型（Stream Type）
			*  
			*  该成员变量用于存储视频流的类型。流类型标识了视频编码格式，
			*  如H264、H265等。
			*  
			*  流类型说明（Stream Type Description）：
			*  - kTsStreamH264: H264视频流（AVC），stream_type = 0x1B
			*  - kTsStreamH265: H265视频流（HEVC），stream_type = 0x24
			*  - kTsStreamMpeg2Video: MPEG-2视频流，stream_type = 0x02
			*  
			*  @note 初始值为kTsStreamReserved（保留类型），需要通过SetStreamType()设置
			*  @note 流类型用于PMT表中的stream_type字段
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
			*  @note 每个TS包写入后，连续性计数器会自动递增
			*/
			int8_t cc_{-1};

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief Start Code插入标志（Start Code Inserted Flag）
			*  
			*  该成员变量用于标识是否已为H264 NALU数据插入start code。
			*  如果为true，表示已经插入过start code，不需要重复插入。
			*  
			*  Start Code说明：
			*  - Start code格式为0x00000001（32位）
			*  - Start code用于标识H264 NALU的开始
			*  - 每个NALU前都需要start code
			*  
			*  @note 初始值为false，表示尚未插入start code
			*  @note 在第一次编码时会插入start code，之后保持为true
			*  @note 用于避免重复插入start code
			*/
			bool   startcode_inserted_{ false };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief SPS/PPS附加标志（SPS/PPS Appended Flag）
			*  
			*  该成员变量用于标识是否已为当前视频序列附加SPS/PPS。
			*  如果为true，表示SPS/PPS已经附加过，不需要重复附加。
			*  
			*  SPS/PPS说明：
			*  - SPS（Sequence Parameter Set）：序列参数集，包含视频序列的配置信息
			*  - PPS（Picture Parameter Set）：图像参数集，包含图像编码参数
			*  - SPS/PPS通常附加在IDR帧前，用于解码器初始化
			*  
			*  @note 初始值为false，表示尚未附加SPS/PPS
			*  @note 在编码IDR帧时会附加SPS/PPS，然后设置为true
			*  @note 用于确保每个视频序列都包含SPS/PPS
			*/
			bool sps_pps_appended_{ false };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 视频解复用器（Video Demultiplexer）
			*  
			*  该成员变量用于视频数据的解复用处理。VideoDemux负责解析
			*  视频编码数据，提取NALU单元，处理SPS/PPS等。
			*  
			*  解复用器功能：
			*  - 解析视频编码数据（如H264 NALU）
			*  - 提取SPS/PPS信息
			*  - 处理NALU单元的分片和重组
			*  - 管理视频序列参数
			*  
			*  @note VideoDemux用于处理视频编码数据的解析和提取
			*  @note 解复用器在编码过程中自动使用
			*/
			VideoDemux     demux_;
		};
	}
}
#endif // 