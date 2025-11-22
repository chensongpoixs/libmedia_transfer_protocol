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
 
 *	created: 		2025-10-09
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

#ifndef _C_LIBMPEG_PS_DECODER_H_
#define _C_LIBMPEG_PS_DECODER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "libmedia_codec/encoded_frame.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_codec/encoded_image.h"
// //////////////////
//#include "libcross_platform_collection_render/video_render/cvideo_render_factory.h"
//#include "libcross_platform_collection_render/video_render/cvideo_render.h"
//#include "libcross_platform_collection_render/track_capture/ctrack_capture.h"
//#include "libmedia_transfer_protocol/rtp_packet_sink_interface.h"
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"
//#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"
//#include "libmedia_codec/video_codec_type.h"
//#include "libmedia_codec/video_codecs/h264_decoder.h"
//#include "libmedia_codec/video_codecs/nal_parse_factory.h"
//#include "libmedia_transfer_protocol/rtp_stream_receiver_controller.h"
//#include "libmedia_transfer_protocol/librtsp/rtsp_session.h"
//#include "libmedia_transfer_protocol/video_receive_stream.h"
//#include "libp2p_peerconnection/connection_context.h"
//#include "libmedia_transfer_protocol/libgb28181/gb28181_session.h"
namespace  libmedia_transfer_protocol {
	class VideoReceiveStream;

	namespace libmpeg
	{
		/**
		*  @author chensong
		*  @date 2025-10-09
		*  @brief MPEG解码器类（MPEG Decoder）
		*  
		*  MpegDecoder类用于解析MPEG-TS流并提取音视频数据。它接收TS包数据，
		*  解析PES包，提取音视频帧，并通过信号槽机制发送给接收端。
		*  
		*  MPEG-TS流解析说明：
		*  - MPEG-TS（Transport Stream）是MPEG-2标准定义的传输流格式
		*  - TS流由多个188字节的TS包组成
		*  - TS包包含PES（Packetized Elementary Stream）数据
		*  - PES包包含音视频编码数据（如H264视频、AAC音频）
		*  
		*  TS包数据结构（TS Packet Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  sync_byte    | transport_error | payload_unit_start | priority|
		*   |  (0x47)       |  indicator      |  indicator        |          |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PID (13 bits)              | transport_scrambling |adapt_field|
		*   |                             |  control             |  control  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  continuity_counter         |  adaptation_field (optional)    |
		*   |  (4 bits)                   |                                  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        Payload Data                             |
		*   |                        (variable, up to 184 bytes)              |
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
		*   |  stream_id     |  PES_packet_length                            |
		*   |  (8 bits)      |  (16 bits)                                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  marker | scrambling | priority | data_alignment | copyright   |
		*   |  bits   |  control   |          |  indicator     |             |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  original_or_copy | PTS_DTS_flags | ESCR_flag | ES_rate_flag  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  DSM_trick_mode_flag | additional_copy_info | CRC_flag        |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PES_header_data_length                                        |
		*   |  (8 bits)                                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PTS (33 bits, optional)                                       |
		*   |  DTS (33 bits, optional)                                       |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        PES Packet Data                         :
		*   |                        (Video: H264 NALU, Audio: AAC/MP3)       |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  解析流程（Parsing Flow）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  1. Receive TS packet data (188 bytes)                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  2. Parse TS packet header (sync_byte, PID, etc.)              |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  3. Extract PES packet from TS payload                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  4. Parse PES header (PTS, DTS, stream_id, etc.)              |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  5. Extract encoded frame (H264 NALU or AAC frame)             |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  6. Signal video frame via SignalRecvVideoFrame                |
		*   |     or audio frame via SignalRecvAudioFrame                    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note MpegDecoder使用信号槽机制发送解析后的音视频帧
		*  @note 解析后的视频帧通过SignalRecvVideoFrame信号发送
		*  @note 解析后的音频帧通过SignalRecvAudioFrame信号发送
		*  
		*  使用示例：
		*  @code
		*  MpegDecoder decoder;
		*  decoder.SignalRecvVideoFrame.connect(&receiver, &Receiver::OnVideoFrame);
		*  decoder.parse(ts_packet_data, 188);
		*  @endcode
		*/
		class MpegDecoder : public   sigslot::has_slots<>
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建MpegDecoder实例。初始化解码器的各个成员变量。
			*  
			*  初始化说明：
			*  - byte_stream_: 初始化为空指针
			*  - stream_len_: 初始化为0
			*  - read_byte_: 初始化为0
			*  - video_pts_: 初始化为0
			*  - audio_pts_: 初始化为0
			*  
			*  @note 构造函数初始化解码器状态，准备接收TS包数据
			*/
			MpegDecoder();

			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理MpegDecoder实例。释放所有相关资源。
			*  
			*  清理流程：
			*  1. 释放byte_stream_缓冲区（如果已分配）
			*  2. 清理所有成员变量
			*  
			*  @note 析构函数会自动释放已分配的缓冲区
			*/
			~MpegDecoder();

		public:

			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 解析TS包数据（Parse TS Packet Data）
			*  
			*  该方法用于解析输入的TS包数据，提取PES包，并从中提取音视频帧。
			*  解析后的音视频帧通过信号槽机制发送给接收端。
			*  
			*  解析流程（Parsing Process）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Store TS packet data to byte_stream_                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Parse TS packet header                                    |
			*   |     - Check sync_byte (0x47)                                  |
			*   |     - Extract PID                                             |
			*   |     - Extract continuity_counter                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Check payload_unit_start indicator                        |
			*   |     - If set, new PES packet starts                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. Extract PES packet from TS payload                        |
			*   |     - Reassemble PES packet across multiple TS packets        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  5. Parse PES packet header                                   |
			*   |     - Extract PTS/DTS                                         |
			*   |     - Extract stream_id                                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  6. Extract encoded frame data                                |
			*   |     - Video: H264 NALU units                                  |
			*   |     - Audio: AAC/MP3 frames                                   |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  7. Signal video frame via SignalRecvVideoFrame               |
			*   |     or audio frame via SignalRecvAudioFrame                   |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  TS包解析格式（TS Packet Parsing Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  sync_byte=0x47 | TEI|PUSI|TP | PID (13 bits)                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  TSC | AFC | CC (4 bits)      |  Adaptation Field (optional) |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |                        Payload Data (184 bytes max)            |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PUSI=1: PES packet starts in this TS packet                  |
			*   |  PUSI=0: PES packet continues from previous TS packet         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向TS包数据的指针，包含188字节的TS包数据，不能为空
			*  @param len TS包数据的大小，单位为字节，通常为188字节
			*  @return 返回0表示成功，其他值表示解析失败
			*  @note 该方法会自动处理跨多个TS包的PES包重组
			*  @note 解析后的音视频帧通过信号槽机制发送
			*  @note 需要持续调用此方法接收完整的PES包
			*  
			*  使用示例：
			*  @code
			*  uint8_t ts_packet[188] = {...};
			*  decoder.parse(ts_packet, 188);
			*  @endcode
			*/
			int parse(const uint8_t *data, int32_t len);


		public:
			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 接收视频帧信号（Receive Video Frame Signal）
			*  
			*  该信号用于在解析出视频帧时通知接收端。接收端可以连接到此信号
			*  来处理解析后的视频帧数据。
			*  
			*  视频帧数据结构（Video Frame Data Structure）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  EncodedImage fields:                                         |
			*   |  - _encodedWidth, _encodedHeight                              |
			*   |  - _frameType (I/P/B frame)                                   |
			*   |  - _timeStamp (PTS)                                           |
			*   |  - data() (H264 NALU units)                                   |
			*   |  - size()                                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 该信号在解析出视频帧时触发
			*  @note 视频帧数据以EncodedImage对象形式传递
			*  @note 接收端需要连接到此信号来处理视频帧
			*  
			*  使用示例：
			*  @code
			*  decoder.SignalRecvVideoFrame.connect(&receiver, &Receiver::OnVideoFrame);
			*  @endcode
			*/
			sigslot::signal1<libmedia_codec::EncodedImage> SignalRecvVideoFrame;

			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 接收音频帧信号（Receive Audio Frame Signal）
			*  
			*  该信号用于在解析出音频帧时通知接收端。接收端可以连接到此信号
			*  来处理解析后的音频帧数据。
			*  
			*  音频帧数据结构（Audio Frame Data Structure）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Audio Frame:                                                 |
			*   |  - Data: CopyOnWriteBuffer (AAC/MP3 frame data)               |
			*   |  - PTS: int64_t (presentation timestamp)                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 该信号在解析出音频帧时触发
			*  @note 音频帧数据以CopyOnWriteBuffer和PTS形式传递
			*  @note 接收端需要连接到此信号来处理音频帧
			*  
			*  使用示例：
			*  @code
			*  decoder.SignalRecvAudioFrame.connect(&receiver, &Receiver::OnAudioFrame);
			*  @endcode
			*/
			sigslot::signal2<rtc::CopyOnWriteBuffer, int64_t > SignalRecvAudioFrame;

		public:

			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 字节流缓冲区（Byte Stream Buffer）
			*  
			*  该成员变量用于存储TS包数据的缓冲区。解析器使用此缓冲区来
			*  临时存储接收到的TS包数据，以便进行解析。
			*  
			*  缓冲区管理（Buffer Management）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  byte_stream_ buffer (variable size)                          |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  | TS Packet 0 (188 bytes)                                   | |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  | TS Packet 1 (188 bytes)                                   | |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   :  ...                                                           :
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 缓冲区大小由stream_len_指示
			*  @note 当前读取位置由read_byte_指示
			*  @note 缓冲区在构造函数中初始化为空指针，在需要时动态分配
			*/
			uint8_t*                byte_stream_;

			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 流长度（Stream Length）
			*  
			*  该成员变量用于存储字节流缓冲区的总长度。表示当前缓冲区中
			*  存储的TS包数据的总大小。
			*  
			*  @note 初始化为0，表示缓冲区为空
			*  @note 随着TS包数据的接收，stream_len_会增加
			*  @note stream_len_表示已存储的数据大小
			*/
			int32_t									stream_len_;

			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 已读取字节数（Read Byte Count）
			*  
			*  该成员变量用于存储已从缓冲区读取的字节数。表示当前解析进度，
			*  用于跟踪哪些数据已经被处理。
			*  
			*  @note 初始化为0，表示尚未读取任何数据
			*  @note read_byte_ <= stream_len_，表示已读取的数据不超过已存储的数据
			*  @note 已处理的数据可以通过更新read_byte_来标记
			*/
			int32_t                 read_byte_;

			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 视频时间戳（Video Presentation Time Stamp）
			*  
			*  该成员变量用于存储视频帧的PTS（Presentation Time Stamp）。
			*  PTS用于指示视频帧的显示时间，用于音视频同步。
			*  
			*  时间戳格式（Time Stamp Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  marker(4) | PTS[32:30] | marker | PTS[29:15] | marker        |
			*   |  (4 bits=0011)| (3 bits) | (1 bit) | (15 bits) | (1 bit)      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PTS[14:0] | marker                                            |
			*   |  (15 bits) | (1 bit)                                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PTS: 33位时间戳（90KHz时钟单位）                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始化为0，表示尚未接收到视频帧
			*  @note PTS单位为90KHz时钟单位（1秒=90000个单位）
			*  @note PTS用于视频帧的显示时间同步
			*/
			int64_t                     video_pts_ = 0;

			/**
			*  @author chensong
			*  @date 2025-10-09
			*  @brief 音频时间戳（Audio Presentation Time Stamp）
			*  
			*  该成员变量用于存储音频帧的PTS（Presentation Time Stamp）。
			*  PTS用于指示音频帧的播放时间，用于音视频同步。
			*  
			*  时间戳格式（Time Stamp Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  marker(4) | PTS[32:30] | marker | PTS[29:15] | marker        |
			*   |  (4 bits=0011)| (3 bits) | (1 bit) | (15 bits) | (1 bit)      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PTS[14:0] | marker                                            |
			*   |  (15 bits) | (1 bit)                                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PTS: 33位时间戳（90KHz时钟单位）                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始化为0，表示尚未接收到音频帧
			*  @note PTS单位为90KHz时钟单位（1秒=90000个单位）
			*  @note PTS用于音频帧的播放时间同步
			*/
			int64_t                     audio_pts_ = 0;
		};


	}
}


#endif // _C_LIBMPEG_DECODER_H_