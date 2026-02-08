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
 created: 		2025-04-29

 author:			chensong

 purpose:		FLV Encoder - FLV格式编码器
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

#ifndef _C_FLV_ENCODER______
#define _C_FLV_ENCODER______


#include <cstdint>
#include <memory> 
#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>
 
#include <functional>
#include <memory>
#include "libmedia_transfer_protocol/libnetwork/connection.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/system/arch.h"
namespace libmedia_transfer_protocol
{
	namespace libflv
	{
		/**
		*  @brief FLV消息类型枚举（FLV Message Type）
		*  
		*  定义FLV文件中Tag的类型，用于标识数据包的内容类型。
		*  
		*  类型说明：
		*  - kFlvMsgTypeAudio (8)：音频数据Tag
		*  - kFlvMsgTypeVideo (9)：视频数据Tag
		*  - kFlvMsgTypeAMFMeta (18)：脚本数据Tag（元数据），使用AMF0格式
		*  
		*  @note FLV文件由一系列Tag组成，每个Tag都有类型标识
		*  @note 元数据Tag通常包含视频分辨率、帧率、编码器等信息
		*/
		enum FlvMsgType
		{
			kFlvMsgTypeAudio = 8,      // 音频Tag
			kFlvMsgTypeVideo = 9,      // 视频Tag
			kFlvMsgTypeAMFMeta = 18,   // 脚本数据Tag（元数据）
		};









#pragma pack(push, 1)

		/**
		*  @author chensong
		*  @date 2025-04-29
		*  @brief FLV文件头结构（FLV File Header）
		*  
		*  FLV文件头是FLV文件的第一个数据结构，固定为9字节，描述文件的基本信息。
		*  
		*  结构说明：
		*  - 总大小：9字节
		*  - 字节序：大端序（Big-Endian）
		*  - 位置：文件开头
		*  
		*  字段详解：
		*  - flv[3]：文件签名，固定为"FLV"（0x46 0x4C 0x56）
		*  - version：FLV版本号，当前固定为1
		*  - have_audio：音频标志位，1表示文件包含音频
		*  - have_video：视频标志位，1表示文件包含视频
		*  - length：文件头长度，固定为9
		*  - previous_tag_size0：第一个Tag之前的Tag大小，固定为0
		*  
		*  音视频组合：
		*  - 只有音频：have_audio=1, have_video=0
		*  - 只有视频：have_audio=0, have_video=1
		*  - 音视频都有：have_audio=1, have_video=1
		*  
		*  @note FLV文件头总是作为第一个数据先发送
		*  @note 使用#pragma pack(1)确保结构体紧凑排列，无填充字节
		*/
		struct FLVHeader {
		public:
			/**
			*  @brief FLV版本号常量
			*/
			static constexpr uint8_t kFlvVersion = 1;

			/**
			*  @brief FLV文件头长度常量（字节）
			*/
			static constexpr uint8_t kFlvHeaderLength = 9;

			// 文件签名，固定为"FLV"
			char flv[3];

			// 文件版本号，固定为1
			uint8_t version;

#if defined( WEBRTC_ARCH_LITTLE_ENDIAN   )
			// 小端序架构的位域定义
			// 保留位，置0
			uint8_t : 5;
			// 是否有音频标志位
			uint8_t have_audio : 1;
			// 保留位，置0
			uint8_t : 1;
			// 是否有视频标志位
			uint8_t have_video : 1;
#elif  defined(  WEBRTC_ARCH_BIG_ENDIAN)
			// 大端序架构的位域定义
			// 是否有视频标志位
			uint8_t have_video : 1;
			// 保留位，置0
			uint8_t : 1;
			// 是否有音频标志位
			uint8_t have_audio : 1;
			// 保留位，置0
			uint8_t : 5;
#endif
			// 文件头长度，固定为9字节
			uint32_t length;

			// 第一个Tag之前的Tag大小，固定为0
			uint32_t previous_tag_size0;
		};


		/**
		*  @author chensong
		*  @date 2025-04-29
		*  @brief FLV Tag头结构（FLV Tag Header）
		*  
		*  FLV Tag头描述每个Tag的基本信息，固定为11字节。
		*  FLV文件由一系列Tag组成，每个Tag包含Tag头和Tag数据。
		*  
		*  结构说明：
		*  - 总大小：11字节
		*  - 字节序：大端序（Big-Endian）
		*  
		*  字段详解：
		*  - type：Tag类型（8=音频，9=视频，18=脚本数据）
		*  - data_size[3]：Tag数据大小（24位，不包含Tag头）
		*  - timestamp[3]：时间戳低24位，单位毫秒
		*  - timestamp_ex：时间戳高8位，扩展时间戳
		*  - streamid[3]：流ID，总是为0
		*  
		*  时间戳说明：
		*  - 完整时间戳 = (timestamp_ex << 24) | timestamp
		*  - 范围：0 到 0xFFFFFFFF（约49.7天）
		*  - 单位：毫秒
		*  
		*  @note Tag数据紧跟在Tag头之后
		*  @note 每个Tag后面跟一个4字节的PreviousTagSize字段
		*/
		struct FlvTagHeader {
			// Tag类型（8=音频，9=视频，18=脚本数据）
			uint8_t type = 0;
			// Tag数据大小（24位，大端序）
			uint8_t data_size[3] = { 0 };
			// 时间戳低24位（大端序）
			uint8_t timestamp[3] = { 0 };
			// 时间戳高8位（扩展时间戳）
			uint8_t timestamp_ex = 0;
			// 流ID，总是为0
			uint8_t streamid[3] = { 0 };
		};

		/**
		*  @author chensong
		*  @date 2025-04-29
		*  @brief FLV增强视频头结构（FLV Enhanced Video Header）
		*  
		*  增强视频头用于支持新的视频编码格式（如H.265、AV1等）。
		*  这是FLV扩展规范的一部分。
		*  
		*  字段说明：
		*  - enhanced：增强标志位，1表示使用增强视频格式
		*  - frame_type：帧类型（1=关键帧，2=非关键帧）
		*  - pkt_type：包类型（0=序列头，1=NALU，2=序列结束）
		*  - fourcc：编码格式标识（FourCC代码）
		*  
		*  @note 增强视频头支持更多的编码格式
		*  @note 位域顺序根据架构字节序调整
		*/
		struct FlvVideoHeaderEnhanced {
#if defined( WEBRTC_ARCH_LITTLE_ENDIAN   )
			uint8_t enhanced : 1;      // 增强标志位
			uint8_t frame_type : 3;    // 帧类型
			uint8_t pkt_type : 4;      // 包类型
			uint32_t fourcc;           // 编码格式标识
#elif defined( WEBRTC_ARCH_BIG_ENDIAN   )
			uint8_t pkt_type : 4;      // 包类型
			uint8_t frame_type : 3;    // 帧类型
			uint8_t enhanced : 1;      // 增强标志位
			uint32_t fourcc;           // 编码格式标识
#endif
		};

		/**
		*  @author chensong
		*  @date 2025-04-29
		*  @brief FLV经典视频头结构（FLV Classic Video Header）
		*  
		*  经典视频头用于传统的视频编码格式（如H.264）。
		*  这是FLV标准规范的一部分。
		*  
		*  字段说明：
		*  - frame_type：帧类型
		*    1 = 关键帧（可搜索帧）
		*    2 = 非关键帧（不可搜索帧）
		*    3 = 可丢弃的非关键帧（H.263专用）
		*    4 = 生成的关键帧（服务器保留）
		*    5 = 视频信息/命令帧
		*  
		*  - codec_id：编码格式ID
		*    2 = H.263
		*    3 = Screen video
		*    4 = VP6
		*    5 = VP6 with alpha
		*    6 = Screen video v2
		*    7 = H.264/AVC
		*    8 = Real H.263
		*    9 = MPEG-4
		*  
		*  - h264_pkt_type：H.264包类型（仅当codec_id=7时有效）
		*    0 = AVC序列头（包含SPS/PPS）
		*    1 = AVC NALU（视频数据）
		*    2 = AVC序列结束
		*  
		*  @note 位域顺序根据架构字节序调整
		*  @note H.264是最常用的编码格式
		*/
		struct FlvVideoHeaderClassic {
#if defined( WEBRTC_ARCH_LITTLE_ENDIAN   )
			uint8_t frame_type : 4;    // 帧类型
			uint8_t codec_id : 4;      // 编码格式ID
			uint8_t h264_pkt_type;     // H.264包类型
#elif defined( WEBRTC_ARCH_BIG_ENDIAN   )
			uint8_t codec_id : 4;      // 编码格式ID
			uint8_t frame_type : 4;    // 帧类型
			uint8_t h264_pkt_type;     // H.264包类型
#endif
		};

#pragma pack(pop)



		/**
		*  @author chensong
		*  @date 2025-04-29
		*  @brief FLV编码器类（FLV Encoder）
		*  
		*  FlvEncoder是FLV格式编码器的核心实现类，负责将原始的音视频数据封装为FLV格式
		*  并通过网络连接或文件输出。该类支持H.264视频编码和AAC音频编码。
		*  
		*  核心功能：
		*  - FLV文件头生成和发送
		*  - FLV元数据（onMetaData）生成和发送
		*  - H.264视频帧封装为FLV Video Tag
		*  - AAC音频帧封装为FLV Audio Tag
		*  - SPS/PPS解析和AVC Decoder Configuration Record生成
		*  - 支持网络流和文件输出双模式
		*  
		*  FLV封装流程：
		*  1. 发送HTTP响应头（如果是网络流）
		*  2. 发送FLV Header（9字节）
		*  3. 发送onMetaData脚本数据Tag（包含视频分辨率、帧率等信息）
		*  4. 发送AVC Decoder Configuration Record（包含SPS/PPS）
		*  5. 持续发送视频/音频Tag
		*  
		*  H.264封装说明：
		*  - 使用AVC格式（NALU长度前缀，而非Annex-B的起始码）
		*  - 首个IDR帧前必须发送SPS/PPS配置包
		*  - 每个NALU前添加4字节长度字段（大端序）
		*  - 关键帧标记为FLV_FRAME_KEY，非关键帧标记为FLV_FRAME_INTER
		*  
		*  时间戳处理：
		*  - 记录首个IDR帧的时间戳作为起始时间
		*  - 所有后续帧的时间戳相对于起始时间计算
		*  - 时间戳单位为毫秒
		*  
		*  @note 该类自动处理SPS/PPS的提取和配置包的生成
		*  @note 支持同时输出到网络连接和本地文件
		*  @note 使用循环缓冲区优化内存使用
		*  
		*  使用示例：
		*  @code
		*  // 创建FLV编码器（网络流+文件输出）
		*  FlvEncoder encoder(connection, "output.flv");
		*  
		*  // 发送FLV Header和元数据
		*  encoder.SendFlvHeader(true, true); // 有音频和视频
		*  
		*  // 发送视频帧
		*  rtc::CopyOnWriteBuffer video_frame = ...;
		*  encoder.SendFlvVideoFrame(video_frame, timestamp_ms);
		*  
		*  // 发送音频帧
		*  rtc::CopyOnWriteBuffer audio_frame = ...;
		*  encoder.SendFlvAudioFrame(audio_frame, timestamp_ms);
		*  @endcode
		*/
		class FlvEncoder
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 构造FLV编码器（Constructor）
			*  
			*  该构造函数用于创建FLV编码器实例，初始化网络连接和文件输出。
			*  
			*  初始化流程：
			*  1. 保存网络连接指针
			*  2. 如果指定了文件名，打开文件用于输出
			*  3. 分配输出缓冲区（8MB）和发送缓冲区（8MB）
			*  4. 初始化时间戳和SPS/PPS标志
			*  5. 发送HTTP响应头（如果有网络连接）
			*  
			*  HTTP响应头格式：
			*  - HTTP/1.1 200 OK
			*  - Access-Control-Allow-Origin: *（允许跨域）
			*  - Content-Type: video/x-flv; charset=utf-8
			*  - Connection: Keep-Alive
			*  
			*  @param conn 网络连接对象指针，用于发送FLV数据，可以为nullptr（仅文件输出）
			*  @param out_flv_file_name 输出FLV文件路径，可选参数，为nullptr时不输出文件
			*  @note 缓冲区大小为8MB，适合高码率视频流
			*  @note 如果文件打开失败，会继续运行但不输出文件
			*  
			*  使用示例：
			*  @code
			*  // 仅网络输出
			*  FlvEncoder encoder1(connection, nullptr);
			*  
			*  // 网络+文件输出
			*  FlvEncoder encoder2(connection, "output.flv");
			*  
			*  // 仅文件输出
			*  FlvEncoder encoder3(nullptr, "output.flv");
			*  @endcode
			*/
		  explicit	FlvEncoder(
			  libnetwork::Connection* conn, const char * out_flv_file_name  = nullptr);
 
			
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 析构FLV编码器（Destructor）
			*  
			*  该析构函数用于清理FLV编码器实例，释放所有分配的资源。
			*  
			*  清理流程：
			*  1. 释放输出缓冲区内存
			*  2. 释放发送缓冲区内存
			*  3. 刷新并关闭输出文件
			*  
			*  @note 网络连接的生命周期由外部管理，不在此处关闭
			*  @note 文件会在关闭前自动刷新缓冲区
			*/
		  virtual ~FlvEncoder();


		public:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 发送FLV文件头和元数据（Send FLV Header）
			*  
			*  该方法用于发送FLV文件头和onMetaData脚本数据Tag。这是FLV流的第一个数据包，
			*  必须在发送任何音视频帧之前调用。
			*  
			*  发送内容：
			*  1. FLV Header（9字节）：
			*     - 签名："FLV"（3字节）
			*     - 版本号：1（1字节）
			*     - 标志位：音频/视频标志（1字节）
			*     - 头长度：9（4字节，大端序）
			*     - PreviousTagSize0：0（4字节）
			*  
			*  2. onMetaData Tag（脚本数据Tag）：
			*     - 使用AMF0格式编码
			*     - 包含视频信息：编码格式、码率、帧率、分辨率
			*     - 包含音频信息：编码格式、采样率、采样精度、声道数
			*     - 包含编码器信息："libflv_rtc"
			*  
			*  元数据字段：
			*  - duration: 视频时长（秒），初始为0
			*  - videocodecid: 视频编码ID（7=H.264）
			*  - videodatarate: 视频码率（kbps）
			*  - framerate: 视频帧率（fps）
			*  - width: 视频宽度（像素）
			*  - height: 视频高度（像素）
			*  - audiocodecid: 音频编码ID（10=AAC）
			*  - audiodatarate: 音频码率（kbps）
			*  - audiosamplerate: 音频采样率（Hz）
			*  - audiosamplesize: 音频采样精度（位）
			*  - stereo: 是否立体声（布尔值）
			*  - encoder: 编码器名称
			*  
			*  @param has_auido 是否包含音频流，true表示有音频
			*  @param has_video 是否包含视频流，true表示有视频
			*  @note 该方法只能调用一次，通常在编码器初始化后立即调用
			*  @note 元数据中的分辨率和帧率为示例值，实际应用中应传入真实参数
			*  
			*  使用示例：
			*  @code
			*  FlvEncoder encoder(connection, "output.flv");
			*  encoder.SendFlvHeader(true, true); // 音视频都有
			*  @endcode
			*/
			void SendFlvHeader(bool has_auido, bool has_video);
			
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 发送FLV视频帧（Send FLV Video Frame）
			*  
			*  该方法用于将H.264编码的视频帧封装为FLV Video Tag并发送。
			*  该方法会自动解析NALU类型，提取SPS/PPS，并在首个IDR帧前发送配置包。
			*  
			*  处理流程：
			*  1. 解析H.264 NALU（使用起始码分割）
			*  2. 根据NALU类型进行不同处理：
			*     - SPS（7）：保存SPS数据
			*     - PPS（8）：保存PPS数据
			*     - IDR（5）：发送配置包（首次），然后发送IDR帧
			*     - 非IDR（1）：发送P帧
			*     - 其他类型：跳过（SEI、AUD等）
			*  3. 将NALU封装为AVC格式（长度前缀）
			*  4. 构造FLV Video Tag并发送
			*  
			*  FLV Video Tag结构：
			*  - Tag Header（11字节）
			*  - Video Tag Data:
			*    * FrameType + CodecID（1字节）：关键帧=0x17，非关键帧=0x27
			*    * AVCPacketType（1字节）：0=配置包，1=NALU
			*    * CompositionTime（3字节）：相对于DTS的偏移，单位毫秒
			*    * NALU数据：4字节长度 + NALU内容（可能有多个）
			*  - PreviousTagSize（4字节）
			*  
			*  时间戳处理：
			*  - 首个IDR帧的时间戳作为起始时间（start_timestamp_）
			*  - 所有后续帧的时间戳相对于起始时间计算
			*  - CompositionTime = timestamp - start_timestamp_
			*  
			*  @param frame H.264编码帧数据，包含一个或多个NALU（Annex-B格式，带起始码）
			*  @param timestamp 视频帧时间戳，单位毫秒
			*  @return 成功返回true，失败返回false
			*  @note 首次调用时必须包含SPS和PPS，否则无法发送IDR帧
			*  @note 该方法会自动跳过SEI、AUD等辅助NALU
			*  @note 如果未收到SPS/PPS/IDR，非关键帧会被丢弃
			*  
			*  使用示例：
			*  @code
			*  // H.264编码帧（Annex-B格式）
			*  rtc::CopyOnWriteBuffer frame = ...;
			*  uint64_t timestamp_ms = rtc::TimeMillis();
			*  encoder.SendFlvVideoFrame(frame, timestamp_ms);
			*  @endcode
			*/
			bool SendFlvVideoFrame(const rtc::CopyOnWriteBuffer & frame, uint64_t timestamp);
			
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 发送FLV音频帧（Send FLV Audio Frame）
			*  
			*  该方法用于将AAC编码的音频帧封装为FLV Audio Tag并发送。
			*  
			*  处理流程：
			*  1. 构造Audio Tag Header（2字节）：
			*     - SoundFormat（4位）：10（AAC）
			*     - SoundRate（2位）：3（44kHz）
			*     - SoundSize（1位）：1（16-bit）
			*     - SoundType（1位）：1（立体声）
			*  2. 添加AACPacketType（1字节）：1（AAC raw数据）
			*  3. 添加AAC音频数据
			*  4. 构造FLV Audio Tag并发送
			*  
			*  FLV Audio Tag结构：
			*  - Tag Header（11字节）
			*  - Audio Tag Data:
			*    * SoundFormat + SoundRate + SoundSize + SoundType（1字节）
			*    * AACPacketType（1字节）：0=AAC配置，1=AAC数据
			*    * AAC音频数据
			*  - PreviousTagSize（4字节）
			*  
			*  音频格式说明：
			*  - SoundFormat=10：AAC编码
			*  - SoundRate=3：44kHz采样率
			*  - SoundSize=1：16位采样精度
			*  - SoundType=1：立体声
			*  
			*  时间戳处理：
			*  - 音频时间戳相对于起始时间（start_timestamp_）计算
			*  - timestamp - start_timestamp_ = 相对时间戳
			*  
			*  @param frame AAC编码的音频数据（AAC raw格式，不含ADTS头）
			*  @param timestamp 音频帧时间戳，单位毫秒
			*  @return 成功返回true，失败返回false
			*  @note 音频数据必须是AAC raw格式，不能包含ADTS头
			*  @note 首次发送音频帧前应先发送AAC配置包（AudioSpecificConfig）
			*  @note 当前实现假设固定的音频参数（44kHz, 16-bit, 立体声）
			*  
			*  使用示例：
			*  @code
			*  // AAC编码帧（raw格式）
			*  rtc::CopyOnWriteBuffer frame = ...;
			*  uint64_t timestamp_ms = rtc::TimeMillis();
			*  encoder.SendFlvAudioFrame(frame, timestamp_ms);
			*  @endcode
			*/
			bool SendFlvAudioFrame(const rtc::CopyOnWriteBuffer & frame, uint64_t timestamp);
			
			 

		private:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 写入FLV Tag（Write FLV Tag）
			*  
			*  该方法是FLV Tag写入的核心实现，负责构造完整的FLV Tag结构并发送。
			*  
			*  FLV Tag完整结构：
			*  1. Tag Header（11字节）：
			*     - TagType（1字节）：8=音频，9=视频，18=脚本数据
			*     - DataSize（3字节）：Tag数据大小，大端序
			*     - Timestamp（3字节）：时间戳低24位，大端序
			*     - TimestampExtended（1字节）：时间戳高8位
			*     - StreamID（3字节）：总是0
			*  2. Tag Data（DataSize字节）：实际的音视频或脚本数据
			*  3. PreviousTagSize（4字节）：当前Tag的总大小（Header + Data），大端序
			*  
			*  时间戳处理：
			*  - FLV时间戳为32位，单位毫秒
			*  - 低24位存储在Timestamp字段
			*  - 高8位存储在TimestampExtended字段
			*  - 完整时间戳 = (TimestampExtended << 24) | Timestamp
			*  
			*  @param type Tag类型（8=音频，9=视频，18=脚本数据）
			*  @param data Tag数据指针，指向实际的音视频或脚本数据
			*  @param size Tag数据大小，单位字节
			*  @param time_stamp 时间戳，单位毫秒，范围0到0xFFFFFFFF
			*  @note 该方法会自动计算PreviousTagSize并追加到Tag末尾
			*  @note 使用Writer方法进行实际的数据写入（支持网络和文件）
			*/
			void WriteFlvTag(uint8_t type, const uint8_t * data, int32_t size, int64_t timestamp);
			
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 写入AVC配置包（Write Config Packet）
			*  
			*  该方法用于生成并发送AVC Decoder Configuration Record（AVCDecoderConfigurationRecord）。
			*  这是H.264视频流的配置信息，包含SPS和PPS，必须在首个IDR帧前发送。
			*  
			*  AVCDecoderConfigurationRecord结构：
			*  1. Video Tag Header（2字节）：
			*     - FrameType + CodecID（1字节）：0x17（关键帧 + AVC）
			*     - AVCPacketType（1字节）：0（AVC序列头）
			*     - CompositionTime（3字节）：0（配置包无偏移）
			*  
			*  2. AVCDecoderConfigurationRecord内容：
			*     - configurationVersion（1字节）：1
			*     - AVCProfileIndication（1字节）：SPS[1]（Profile）
			*     - profile_compatibility（1字节）：SPS[2]（兼容性）
			*     - AVCLevelIndication（1字节）：SPS[3]（Level）
			*     - lengthSizeMinusOne（1字节）：0xFF（NALU长度字段为4字节）
			*     - numOfSequenceParameterSets（1字节）：0xE1（SPS个数=1）
			*     - sequenceParameterSetLength（2字节）：SPS长度，大端序
			*     - sequenceParameterSetNALUnit（N字节）：SPS数据
			*     - numOfPictureParameterSets（1字节）：1（PPS个数）
			*     - pictureParameterSetLength（2字节）：PPS长度，大端序
			*     - pictureParameterSetNALUnit（M字节）：PPS数据
			*  
			*  @note 该方法在首个IDR帧到达时自动调用
			*  @note SPS和PPS必须已经通过SendFlvVideoFrame方法提取
			*  @note 配置包的时间戳为0
			*  @note lengthSizeMinusOne=0xFF表示NALU长度字段为4字节（3+1）
			*/
			void WriteConfigPacket();
			
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 写入数据到输出（Writer）
			*  
			*  该方法是数据输出的底层实现，负责将数据写入文件和网络连接。
			*  使用发送缓冲区进行批量发送，减少网络调用次数。
			*  
			*  写入流程：
			*  1. 如果有文件输出，写入文件并刷新
			*  2. 将数据追加到发送缓冲区
			*  3. 如果fflsh=true，通过网络连接发送缓冲区数据并清空
			*  
			*  批量发送优化：
			*  - 使用send_buffer_缓存多个Tag的数据
			*  - 只有在fflsh=true时才真正发送
			*  - 减少网络调用次数，提高效率
			*  
			*  @param data 要写入的数据指针
			*  @param size 数据大小，单位字节
			*  @param fflsh 是否立即刷新发送缓冲区，默认false
			*  @note 文件输出会立即写入并刷新
			*  @note 网络输出使用批量发送，只有fflsh=true时才发送
			*  @note 发送缓冲区大小为8MB，足够缓存多个Tag
			*/
			void Writer(const uint8_t * data, int32_t size, bool fflsh = false);
			
		private: 
			/**
			*  @brief 网络连接对象指针
			*  
			*  用于通过网络发送FLV数据到客户端。如果为nullptr，则只输出到文件。
			*/
			libnetwork::Connection *          connection_;
			
			/**
			*  @brief 输出文件指针
			*  
			*  用于将FLV数据写入本地文件。如果为nullptr，则只输出到网络。
			*/
			FILE *out_file_ptr_;

			/**
			*  @brief 上一个Tag的大小
			*  
			*  记录上一个Tag的总大小（Header + Data），用于写入PreviousTagSize字段。
			*  FLV格式要求每个Tag后面跟一个4字节的PreviousTagSize。
			*/
			uint32_t                  prev_packet_size_;
			
			/**
			*  @brief 输出缓冲区指针
			*  
			*  用于临时存储构造的FLV Tag数据，大小为8MB。
			*  该缓冲区用于Tag的组装，然后通过Writer方法输出。
			*/
			uint8_t *out_buffer_  { nullptr };
			
			/**
			*  @brief 当前写入位置指针
			*  
			*  指向out_buffer_中的当前写入位置，用于追加数据。
			*/
			uint8_t * current_{ nullptr }; 
 
		 	/**
			*  @brief SPS发送标志
			*  
			*  标记是否已经发送过SPS/PPS配置包。
			*  只有在首个IDR帧到达时才设置为true，确保配置包只发送一次。
			*/
			bool                  send_sps_;
			
			/**
			*  @brief SPS数据
			*  
			*  存储从视频帧中提取的SPS（Sequence Parameter Set）数据。
			*  SPS包含视频的基本参数（分辨率、Profile、Level等）。
			*/
			std::string            sps_;
			
			/**
			*  @brief PPS数据
			*  
			*  存储从视频帧中提取的PPS（Picture Parameter Set）数据。
			*  PPS包含图像的编码参数。
			*/
			std::string            pps_;

			/**
			*  @brief 起始时间戳
			*  
			*  记录首个IDR帧的时间戳，作为相对时间戳的基准。
			*  所有后续帧的时间戳都相对于该值计算。
			*/
			uint64_t               start_timestamp_;

			/**
			*  @brief 发送缓冲区指针
			*  
			*  用于批量发送数据到网络，大小为8MB。
			*  多个Tag的数据会先缓存到该缓冲区，然后一次性发送。
			*/
			uint8_t * send_buffer_{nullptr};
			
			/**
			*  @brief 发送缓冲区当前大小
			*  
			*  记录send_buffer_中已缓存的数据大小，单位字节。
			*  当需要刷新时，发送该大小的数据并重置为0。
			*/
			int32_t   send_size_;
  
		};
	}
}

#endif   //FlvEncoder