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
 
 *	created: 		2025-05-02
 *
 *	author:			chensong
 *
 *	purpose:		video encoder
 *	输赢不重要，答案对你们有什么意义才重要。
 *
 *	光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。
 *
 *
 *	我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
 *	然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
 *	3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
 *	然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
 *	于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
 *	我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
 *	从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
 *	我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
 *	沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
 *	安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
 ***********************************************************************************************/

 #ifndef _C_EPEGTS_____ENCODER_
 #define _C_EPEGTS_____ENCODER_
 
 
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
  
 #include "libmedia_transfer_protocol/libmpeg/cpat_writer.h"
 #include "libmedia_transfer_protocol/libmpeg/cpmt_writer.h"
 #include "libmedia_transfer_protocol/libmpeg/cpsi_writer.h"
 #include "libmedia_transfer_protocol/libmpeg/cpsi_writer.h"
 #include "libmedia_transfer_protocol/libmpeg/cmpeg_type.h"
 
 #include "libmedia_transfer_protocol/libmpeg/cvideo_encoder.h"
 #include "libmedia_transfer_protocol/libmpeg/caudio_encoder.h"
 #include "rtc_base/copy_on_write_buffer.h"
 #include "libmedia_transfer_protocol/libmpeg/packet.h"
 namespace libmedia_transfer_protocol
 {
	 namespace libmpeg
	 {
		 /**
		 *  @author chensong
		 *  @date 2025-05-02
		 *  @brief MPEG-TS编码器类（MPEG-TS Encoder）
		 *  
		 *  MpegTsEncoder类用于将媒体包（音频包、视频包）编码为MPEG-TS格式。
		 *  它负责生成TS包，包括PAT表、PMT表以及媒体数据的TS包封装。
		 *  
		 *  MPEG-TS编码器说明：
		 *  - MpegTsEncoder用于将媒体数据编码为MPEG-TS格式
		 *  - MPEG-TS（Transport Stream）是MPEG-2标准中定义的传输流格式
		 *  - TS流包含PSI表（PAT、PMT）和媒体数据（视频、音频）
		 *  - 每个TS包188字节，包含头部和负载数据
		 *  
		 *  MPEG-TS编码流程（MPEG-TS Encoding Flow）：
		 *  
		 *    0                   1                   2                   3
		 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  1. WritePatPmt() writes PAT and PMT tables                     |
		 *   |     - PAT table: Lists program and PMT PID                       |
		 *   |     - PMT table: Lists video/audio streams and PIDs              |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  2. Encode() encodes media packets                               |
		 *   |     - Video packets: Encoded to TS video packets                 |
		 *   |     - Audio packets: Encoded to TS audio packets                 |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  3. TS packets are written to StreamWriter                       |
		 *   |     - Each TS packet is 188 bytes                                |
		 *   |     - TS packets contain media data                              |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *  
		 *  TS包结构（TS Packet Structure）：
		 *  
		 *    0                   1                   2                   3
		 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  Sync Byte (0x47) | TEI | PUSI | Priority | PID (13 bits)     |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  TSC | AFC | CC (4 bits) |  Adaptation Field (optional)        |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |                        Payload Data (variable)                    |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   :                                                               :
		 *   |                    ... more payload data ...                      |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *  
		 *  TS包头部字段（TS Packet Header Fields）：
		 *  
		 *    0                   1                   2                   3
		 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  Sync Byte        |  Always 0x47                                 |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  PID              |  Packet Identifier (13 bits)                 |
		 *   |  - Video: video_pid_                                             |
		 *   |  - Audio: audio_pid_                                             |
		 *   |  - PAT: 0x0000                                                   |
		 *   |  - PMT: 0x1001                                                   |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  PUSI             |  Payload Unit Start Indicator (1 bit)        |
		 *   |  - 1: PES packet or PSI section starts                            |
		 *   |  - 0: Continuation of previous packet                             |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  CC               |  Continuity Counter (4 bits)                 |
		 *   |  - Increments for each TS packet with same PID                   |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *  
		 *  PES包封装流程（PES Packet Encapsulation Flow）：
		 *  
		 *    0                   1                   2                   3
		 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  1. Media packet (video/audio)                                   |
		 *   |     - Contains encoded media data                                 |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  2. PES packet header                                            |
		 *   |     - Stream ID (8 bits)                                          |
		 *   |     - PES packet length (16 bits)                                 |
		 *   |     - PTS/DTS timestamps                                          |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  3. PES packet payload                                            |
		 *   |     - Media data (video/audio)                                    |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |  4. PES packet is split into TS packets                           |
		 *   |     - Each TS packet is 188 bytes                                 |
		 *   |     - First TS packet has PUSI=1                                  |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *  
		 *  @note MpegTsEncoder负责整个TS流的生成过程，包括PSI表和媒体数据
		 *  @note TS包固定为188字节，包含4字节头部和184字节负载（可能包含适配字段）
		 *  @note 编码器自动处理PES包封装和TS包分割
		 *  @note 编码器支持多种视频编码格式（H264）和音频编码格式（AAC、MP3）
		 *  
		 *  使用示例：
		 *  @code
		 *  MpegTsEncoder encoder;
		 *  encoder.SetStreamType(stream_writer, kVideoCodecH264, kAudioCodecAAC);
		 *  encoder.WritePatPmt(stream_writer);
		 *  encoder.Encode(stream_writer, video_packet, dts);
		 *  @endcode
		 */
		 class MpegTsEncoder
		 {
		 public:
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 构造函数（Constructor）
			 *  
			 *  该构造函数用于初始化MpegTsEncoder实例。使用默认构造函数，
			 *  所有成员变量使用默认值。
			 *  
			 *  初始化说明：
			 *  - pat_writer_: 默认初始化，table_id=0x00, pid=0x0000
			 *  - pmt_writer_: 默认初始化，table_id=0x02, pid=0x1001
			 *  - audio_encoder_: 默认初始化
			 *  - video_encoder_: 默认初始化
			 *  - audio_type_: 初始化为kTsStreamReserved
			 *  - video_type_: 初始化为kTsStreamReserved
			 *  - audio_pid_: 初始化为0xE000（无效PID）
			 *  - video_pid_: 初始化为0xE000（无效PID）
			 *  
			 *  @note 使用默认构造函数，所有成员变量使用默认初始化值
			 *  @note 流类型和PID需要通过SetStreamType()方法设置
			 *  
			 *  使用示例：
			 *  @code
			 *  MpegTsEncoder encoder;
			 *  // 编码器已初始化，需要设置流类型和PID
			 *  @endcode
			 */
			 MpegTsEncoder() = default;
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 析构函数（Destructor）
			 *  
			 *  该析构函数用于清理MpegTsEncoder实例。使用默认析构函数，
			 *  所有成员变量自动释放。
			 *  
			 *  @note 使用默认析构函数，智能指针自动管理内存
			 */
			 ~MpegTsEncoder() = default;
 
		 public:
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 编码媒体包（Encode Media Packet）
			 *  
			 *  该方法用于将媒体包（音频包或视频包）编码为TS格式。
			 *  编码过程包括PES包封装和TS包分割。
			 *  
			 *  编码流程（Encoding Flow）：
			 *  
			 *    0                   1                   2                   3
			 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  1. Check packet type                                          |
			 *   |     - If video: Use VideoEncoder                               |
			 *   |     - If audio: Use AudioEncoder                               |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  2. Encode packet to PES format                                |
			 *   |     - Create PES packet header                                  |
			 *   |     - Add PTS/DTS timestamps                                    |
			 *   |     - Add PES packet payload (media data)                       |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  3. Split PES packet into TS packets                            |
			 *   |     - Each TS packet is 188 bytes                               |
			 *   |     - First TS packet has PUSI=1                                |
			 *   |     - Middle TS packets have PUSI=0                             |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  4. Write TS packets to StreamWriter                            |
			 *   |     - writer->Write(ts_packet, 188)                             |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *  
			 *  PES包结构（PES Packet Structure）：
			 *  
			 *    0                   1                   2                   3
			 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  Packet Start Code Prefix (24 bits) |  Stream ID (8 bits)     |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  PES Packet Length (16 bits) |  PES Header (variable)         |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  PTS/DTS Flags |  PTS (33 bits) |  DTS (33 bits, if present) |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |                        PES Payload (variable)                    |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   :                                                               :
		 *   |                    ... more payload data ...                      |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *  
		 *  @param writer 指向StreamWriter的指针，用于写入TS包数据，不能为空
		 *  @param data 指向媒体包的共享指针，包含音频或视频数据，不能为空
		 *  @param dts 解码时间戳（Decode Time Stamp），单位为毫秒或90KHz时钟
		 *  @return 如果编码成功，返回0；如果发生错误，返回非0错误码
		 *  @note 媒体包可以是音频包或视频包，编码器会自动识别
		 *  @note 时间戳用于音视频同步和播放控制
		 *  @note 编码后的TS包通过StreamWriter写入
		 *  
		 *  使用示例：
		 *  @code
		 *  auto packet = std::make_shared<Packet>(1024);
		 *  // ... 填充packet数据 ...
		 *  int32_t ret = encoder.Encode(stream_writer, packet, dts);
		 *  if (ret == 0) {
		 *      // 编码成功
		 *  }
		 *  @endcode
		 */
			 int32_t   Encode(StreamWriter *writer, std::shared_ptr<Packet> &data, int64_t  dts);
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 设置流类型（Set Stream Type）
			 *  
			 *  该方法用于设置视频和音频的流类型和PID。流类型用于标识编码格式，
			 *  PID用于标识TS包的类型。
			 *  
			 *  流类型设置流程（Stream Type Setting Flow）：
			 *  
			 *    0                   1                   2                   3
			 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  1. Set video stream type                                       |
			 *   |     - video_type_ = MapVideoCodecToTsType(vc)                   |
			 *   |     - video_pid_ = GetVideoPid()                                 |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  2. Set audio stream type                                       |
			 *   |     - audio_type_ = MapAudioCodecToTsType(ac)                   |
			 *   |     - audio_pid_ = GetAudioPid()                                 |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  3. Update PMT table                                            |
			 *   |     - Add video stream info to PMT                               |
			 *   |     - Add audio stream info to PMT                               |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *  
			 *  流类型映射（Stream Type Mapping）：
			 *  
			 *    0                   1                   2                   3
			 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  Video Codec ID -> TS Stream Type                               |
			 *   |  - kVideoCodecH264 -> 0x1F (H264 Video)                         |
			 *   |  - kVideoCodecH265 -> 0x24 (H265 Video)                         |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  Audio Codec ID -> TS Stream Type                               |
			 *   |  - kAudioCodecAAC -> 0x0F (AAC Audio)                           |
			 *   |  - kAudioCodecMP3 -> 0x03 (MP3 Audio)                           |
			 *   |  - kAudioCodecAC3 -> 0x06 (AC3 Audio)                           |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *  
			 *  @param writer 指向StreamWriter的指针，用于写入配置信息，不能为空
			 *  @param vc 视频编解码器ID，用于标识视频编码格式（如kVideoCodecH264）
			 *  @param ac 音频编解码器ID，用于标识音频编码格式（如kAudioCodecAAC）
			 *  @note 流类型用于PMT表中的stream_type字段
			 *  @note PID用于TS包的PID字段，标识TS包的类型
			 *  @note 设置流类型后，需要调用WritePatPmt()写入PAT和PMT表
			 *  
			 *  使用示例：
			 *  @code
			 *  encoder.SetStreamType(stream_writer, kVideoCodecH264, kAudioCodecAAC);
			 *  encoder.WritePatPmt(stream_writer);
			 *  @endcode
			 */
			 void   SetStreamType(StreamWriter * writer, VideoCodecID vc, AudioCodecID ac);
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 写入PAT和PMT表（Write PAT and PMT Tables）
			 *  
			 *  该方法用于写入PAT（Program Association Table）和PMT（Program Map Table）表。
			 *  PAT表列出所有节目的映射关系，PMT表列出节目的原始流信息。
			 *  
			 *  PAT和PMT表写入流程（PAT/PMT Writing Flow）：
			 *  
			 *    0                   1                   2                   3
			 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  1. Write PAT table                                            |
			 *   |     - PAT table lists program and PMT PID                       |
			 *   |     - PAT table is written to PID 0x0000                         |
			 *   |     - pat_writer_.WritePat(stream_writer)                        |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  2. Write PMT table                                            |
			 *   |     - PMT table lists video/audio streams and PIDs              |
			 *   |     - PMT table is written to PID 0x1001                         |
			 *   |     - pmt_writer_.WritePmt(stream_writer)                        |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  3. PAT and PMT tables are sent periodically                    |
			 *   |     - Typically every 100ms                                      |
			 *   |     - Ensures clients can join stream at any time                |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *  
			 *  PAT表结构（PAT Table Structure）：
			 *  
			 *    0                   1                   2                   3
			 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  Table ID (0x00) |  Section Length |  Transport Stream ID     |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  Version | Current Next |  Section Number |  Last Section     |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  Program Number (16 bits) |  Reserved |  PMT PID (13 bits)    |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |                    ... more programs ...                        |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |                    CRC32 (32 bits)                              |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *  
		 *  @param writer 指向StreamWriter的指针，用于写入PAT和PMT表数据，不能为空
		 *  @return 如果写入成功，返回0；如果发生错误，返回非0错误码
		 *  @note PAT表写入到PID 0x0000，PMT表写入到PID 0x1001
		 *  @note PAT和PMT表需要定期发送，确保客户端能够随时加入流
		 *  @note 在开始编码媒体数据之前，必须先写入PAT和PMT表
		 *  
		 *  使用示例：
		 *  @code
		 *  encoder.SetStreamType(stream_writer, kVideoCodecH264, kAudioCodecAAC);
		 *  int32_t ret = encoder.WritePatPmt(stream_writer);
		 *  if (ret == 0) {
		 *      // PAT和PMT表已写入，可以开始编码媒体数据
		 *  }
		 *  @endcode
		 */
			 int32_t   WritePatPmt(StreamWriter * writer);
		 private:
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief PAT表写入器（PAT Writer）
			 *  
			 *  该成员变量用于写入PAT（Program Association Table）表。
			 *  PAT表列出所有节目的映射关系，用于定位PMT表。
			 *  
			 *  PAT表说明：
			 *  - PAT（Program Association Table）是节目关联表
			 *  - PAT表列出传输流中所有节目的映射关系
			 *  - PAT表通过PID 0x0000传输
			 *  - PAT表用于定位PMT表（通过PMT PID）
			 *  
			 *  @note PAT表写入到PID 0x0000
			 *  @note PAT表固定使用table_id 0x00
			 *  @note PAT表通过WritePat()方法写入
			 */
			 PatWriter   pat_writer_;
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief PMT表写入器（PMT Writer）
			 *  
			 *  该成员变量用于写入PMT（Program Map Table）表。
			 *  PMT表列出节目的所有原始流信息（视频流、音频流等）。
			 *  
			 *  PMT表说明：
			 *  - PMT（Program Map Table）是节目映射表
			 *  - PMT表列出节目的所有原始流（视频流、音频流等）及其PID
			 *  - PMT表通过PID 0x1001传输（默认值，可在PAT表中指定）
			 *  - PMT表包含PCR PID，用于时钟同步
			 *  
			 *  @note PMT表写入到PID 0x1001（默认值）
			 *  @note PMT表固定使用table_id 0x02
			 *  @note PMT表通过WritePmt()方法写入
			 */
			 PmtWriter   pmt_writer_;
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 音频编码器（Audio Encoder）
			 *  
			 *  该成员变量用于将音频包编码为TS格式。音频编码器负责
			 *  PES包封装和TS包分割。
			 *  
			 *  音频编码器功能：
			 *  - 将音频包编码为PES包
			 *  - 添加PES包头部（PTS/DTS时间戳等）
			 *  - 将PES包分割为TS包
			 *  - 写入TS包到StreamWriter
			 *  
			 *  @note 音频编码器支持多种音频编码格式（AAC、MP3等）
			 *  @note 音频编码器使用audio_pid_作为TS包的PID
			 */
			 AudioEncoder audio_encoder_;
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 视频编码器（Video Encoder）
			 *  
			 *  该成员变量用于将视频包编码为TS格式。视频编码器负责
			 *  PES包封装和TS包分割。
			 *  
			 *  视频编码器功能：
			 *  - 将视频包编码为PES包
			 *  - 添加PES包头部（PTS/DTS时间戳等）
			 *  - 将PES包分割为TS包
			 *  - 写入TS包到StreamWriter
			 *  
			 *  @note 视频编码器支持多种视频编码格式（H264、H265等）
			 *  @note 视频编码器使用video_pid_作为TS包的PID
			 */
			 VideoEncoder video_encoder_;
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 音频流类型（Audio Stream Type）
			 *  
			 *  该成员变量用于存储音频流的TS流类型。TS流类型用于标识
			 *  音频编码格式。
			 *  
			 *  音频流类型说明（Audio Stream Type Values）：
			 *  
			 *    0                   1                   2                   3
			 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  kTsStreamAAC: 0x0F |  AAC Audio                               |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  kTsStreamMP3: 0x03 |  MP3 Audio                               |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  kTsStreamAC3: 0x06 |  AC3 Audio                               |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  kTsStreamReserved: 0xFF |  Reserved/Unknown                   |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *  
			 *  @note 音频流类型用于PMT表中的stream_type字段
			 *  @note 默认值为kTsStreamReserved，表示尚未设置
			 *  @note 音频流类型通过SetStreamType()方法设置
			 */
			 TsStreamType	audio_type_{ kTsStreamReserved };
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 视频流类型（Video Stream Type）
			 *  
			 *  该成员变量用于存储视频流的TS流类型。TS流类型用于标识
			 *  视频编码格式。
			 *  
			 *  视频流类型说明（Video Stream Type Values）：
			 *  
			 *    0                   1                   2                   3
			 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  kTsStreamH264: 0x1F |  H264 Video                             |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  kTsStreamH265: 0x24 |  H265 Video                             |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *   |  kTsStreamReserved: 0xFF |  Reserved/Unknown                   |
			 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			 *  
			 *  @note 视频流类型用于PMT表中的stream_type字段
			 *  @note 默认值为kTsStreamReserved，表示尚未设置
			 *  @note 视频流类型通过SetStreamType()方法设置
			 */
			 TsStreamType	video_type_{ kTsStreamReserved };
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 音频PID（Audio Packet Identifier）
			 *  
			 *  该成员变量用于存储音频流的PID。PID用于标识TS包的类型。
			 *  
			 *  音频PID说明：
			 *  - PID是一个13位字段，取值范围0-8191
			 *  - PID用于标识音频TS包的PID
			 *  - PID在PMT表中指定，接收端使用PID过滤音频包
			 *  - 常见的音频PID值：0x0101, 0x0102等
			 *  
			 *  @note 音频PID是13位字段，取值范围0-8191
			 *  @note 默认值为0xE000，表示无效PID
			 *  @note 音频PID通过SetStreamType()方法设置
			 *  @note 音频PID在PMT表中指定，接收端使用PID过滤音频包
			 */
			 uint16_t    audio_pid_{ 0XE000 };
 
			 /**
			 *  @author chensong
			 *  @date 2025-05-02
			 *  @brief 视频PID（Video Packet Identifier）
			 *  
			 *  该成员变量用于存储视频流的PID。PID用于标识TS包的类型。
			 *  
			 *  视频PID说明：
			 *  - PID是一个13位字段，取值范围0-8191
			 *  - PID用于标识视频TS包的PID
			 *  - PID在PMT表中指定，接收端使用PID过滤视频包
			 *  - 常见的视频PID值：0x0100, 0x0101等
			 *  - 视频PID通常作为PCR PID，用于时钟同步
			 *  
			 *  @note 视频PID是13位字段，取值范围0-8191
			 *  @note 默认值为0xE000，表示无效PID
			 *  @note 视频PID通过SetStreamType()方法设置
			 *  @note 视频PID在PMT表中指定，接收端使用PID过滤视频包
			 *  @note 视频PID通常作为PCR PID，用于时钟同步
			 */
			 uint16_t    video_pid_{ 0XE000 };
 
		 };
	 }
 }
 
 #endif // _C_TS_ENCODER_