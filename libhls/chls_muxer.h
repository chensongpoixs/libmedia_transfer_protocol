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

purpose:		  ts 的 切片管理  释放，


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

#ifndef _C_HLS_MUXER___
#define _C_HLS_MUXER___


#include <cstdint>
#include <memory>

#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>


#include <unordered_map>

#include <functional>
#include <memory>
//#include "cstream_writer.h"
//#include "cts_encoder.h"
#include "cfragment.h"
#include <mutex>
#include <vector>
#include<algorithm>
//#include"libmedia_transfer_protocol/libhls/cfragment.h"

#include "libmedia_transfer_protocol/libhls/cfragment_window.h"
#include "libmedia_transfer_protocol/libmpeg/mpegts_encoder.h"
#include "libmedia_transfer_protocol/libmpeg/packet.h"
namespace libmedia_transfer_protocol
{
	namespace libhls
	{

		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief HLS 复用器类（HLS Muxer）
		*  
		*  HLSMuxer类用于将媒体数据打包成HLS格式。它负责接收媒体包（Packet），
		*  编码成TS格式，并将TS数据写入切片（Fragment），最终生成M3U8播放列表。
		*  
		*  HLS复用器说明：
		*  - HLS（HTTP Live Streaming）是Apple开发的流媒体传输协议
		*  - 媒体流被分割成多个小切片文件（通常是TS格式）
		*  - 每个切片包含一段连续的媒体数据，具有一定的时长和大小
		*  - 客户端通过播放列表（M3U8）获取切片列表并顺序播放
		*  
		*  HLS复用器架构（HLS Muxer Architecture）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        Media Packet                             |
		*   |                        (Audio/Video)                             |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        MPEG-TS Encoder                           |
		*   |                        (Packet -> TS)                            |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        Current Fragment                          |
		*   |                        (TS Data Buffer)                          |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        Fragment Window                           |
		*   |                        (Fragment List)                           |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        M3U8 Playlist                             |
		*   |                        (Playlist String)                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  数据流程图（Data Flow Diagram）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  1. OnPacket() receives media packet                            |
		*   |     - Audio packet or Video packet                               |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  2. Check if packet is codec header                              |
		*   |     - Parse codec header if needed                               |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  3. MpegTsEncoder encodes packet to TS                           |
		*   |     - Audio -> TS audio packet                                    |
		*   |     - Video -> TS video packet                                    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  4. Write TS data to current fragment                            |
		*   |     - Check fragment size                                         |
		*   |     - Create new fragment if needed                               |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  5. Append fragment to FragmentWindow                            |
		*   |     - Window manages fragment lifecycle                           |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  6. Generate/Update M3U8 playlist                                |
		*   |     - FragmentWindow generates playlist                           |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  切片创建流程（Fragment Creation Flow）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Condition: current_fragment_ == nullptr                         |
		*   |  Action: GetIdleFragment() -> create new fragment                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Condition: fragment->Size() >= min_fragment_size_               |
		*   |  Action: Create new fragment (old fragment becomes ready)        |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Condition: fragment->Size() >= max_fragment_size_               |
		*   |  Action: Force create new fragment (enforce max size)            |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note HLSMuxer负责整个HLS流的生成过程，包括编码、切片、窗口管理
		*  @note 切片大小通过min_fragment_size_和max_fragment_size_控制
		*  @note 编码器将媒体包转换为TS格式，然后写入切片
		*  
		*  使用示例：
		*  @code
		*  HLSMuxer muxer("stream_name");
		*  auto packet = std::make_shared<Packet>();
		*  muxer.OnPacket(packet);
		*  std::string playlist = muxer.PlayList();
		*  @endcode
		*/
		class HLSMuxer
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建HLSMuxer实例。初始化复用器的各个组件，
			*  包括切片窗口和编码器。
			*  
			*  初始化流程：
			*  1. 设置stream_name_为指定的会话名称
			*  2. 初始化fragment_window_（默认窗口大小为5）
			*  3. 初始化encoder_（MPEG-TS编码器）
			*  4. 设置current_fragment_为空指针
			*  5. 初始化fragment_seq_no_为0
			*  6. 设置min_fragment_size_为3000（TS包数量，约564KB）
			*  7. 设置max_fragment_size_为12000（TS包数量，约2.2MB）
			*  
			*  切片大小说明：
			*  - min_fragment_size_: 最小切片大小（TS包数量），默认3000包
			*  - max_fragment_size_: 最大切片大小（TS包数量），默认12000包
			*  - 每个TS包188字节，3000包约564KB，12000包约2.2MB
			*  
			*  @param session_name 会话名称字符串，用于生成切片文件名
			*  @note 会话名称通常用于生成切片文件的基础文件名
			*  @note 编码器在构造函数中自动初始化
			*  
			*  使用示例：
			*  @code
			*  HLSMuxer muxer("live_stream");
			*  // 复用器已初始化，可以接收媒体包
			*  @endcode
			*/
			HLSMuxer(const std::string &session_name) ;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理HLSMuxer实例。使用默认析构函数，
			*  所有成员变量自动释放。
			*  
			*  @note 使用智能指针管理资源，自动释放内存
			*/
			~HLSMuxer() = default;
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取播放列表（Get Play List）
			*  
			*  该方法用于获取M3U8格式的播放列表。播放列表包含切片窗口中
			*  所有切片的元信息，客户端通过播放列表获取切片列表。
			*  
			*  @return 返回M3U8格式的播放列表字符串
			*  @note 播放列表由FragmentWindow生成和管理
			*  @note 播放列表格式符合HLS协议规范
			*  
			*  使用示例：
			*  @code
			*  std::string playlist = muxer.PlayList();
			*  // playlist 包含完整的M3U8播放列表
			*  @endcode
			*/
			std::string  PlayList();

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 处理媒体包（On Packet）
			*  
			*  该方法用于处理输入的媒体包（音频或视频包）。该方法负责
			*  将媒体包编码为TS格式，并写入当前切片。
			*  
			*  处理流程（Processing Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Check if packet is codec header                            |
			*   |     - If yes, parse codec header                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Get or create current fragment                              |
			*   |     - If current_fragment_ == nullptr, get idle fragment        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Encode packet to TS format                                  |
			*   |     - MpegTsEncoder encodes packet                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. Write TS data to current fragment                           |
			*   |     - fragment->Write(ts_data, ts_size)                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  5. Update fragment timestamp                                   |
			*   |     - fragment->AppendTimestamp(packet->pts)                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  6. Check fragment size                                         |
			*   |     - If size >= min_fragment_size_, create new fragment        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  7. Append fragment to window if ready                          |
			*   |     - fragment_window_.AppendFragment(std::move(fragment))      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param packet 指向媒体包的共享指针，包含音频或视频数据
			*  @note 媒体包可以是音频包或视频包
			*  @note 如果包是编解码器头信息（SPS/PPS等），会进行特殊处理
			*  @note 切片大小通过min_fragment_size_和max_fragment_size_控制
			*  
			*  使用示例：
			*  @code
			*  auto packet = std::make_shared<Packet>();
			*  // ... 填充packet数据 ...
			*  muxer.OnPacket(packet);
			*  @endcode
			*/
			void OnPacket(std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet> & packet);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 根据文件名获取切片（Get Fragment By Name）
			*  
			*  该方法用于根据文件名在切片窗口中查找对应的切片。通常用于
			*  客户端请求特定切片时定位切片数据。
			*  
			*  @param name 切片的文件名，例如 "stream_0001.ts"
			*  @return 返回指向匹配切片的共享指针。如果未找到，返回空指针
			*  @note 该方法委托给FragmentWindow的GetFragmentByName方法
			*  
			*  使用示例：
			*  @code
			*  auto fragment = muxer.GetFragment("stream_0001.ts");
			*  if (fragment) {
			*      // 使用fragment数据
			*  }
			*  @endcode
			*/
			std::shared_ptr<Fragment> GetFragment(const std::string & name);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 解析编解码器信息（Parse Codec）
			*  
			*  该方法用于解析媒体包中的编解码器头信息（如H264的SPS/PPS）。
			*  编解码器头信息需要特殊处理，通常需要写入切片的开头。
			*  
			*  编解码器头格式（Codec Header Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Codec Type    |  Header Data (variable)                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  For H264:                                                    |
			*   |  - SPS (Sequence Parameter Set)                                |
			*   |  - PPS (Picture Parameter Set)                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  For AAC:                                                     |
			*   |  - Audio Specific Config (ASC)                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  解析流程：
			*  1. 检查包是否为编解码器头信息（SPS/PPS/ASC等）
			*  2. 如果是指定的编解码器头，提取头信息
			*  3. 将头信息写入切片的开头（如果有新切片创建）
			*  4. 更新编码器的编解码器配置
			*  
			*  @param fragment 指向切片的共享指针，用于写入编解码器头信息
			*  @param packet 指向媒体包的共享指针，包含编解码器头数据
			*  @note 编解码器头信息通常需要写入每个切片的开头
			*  @note 该方法主要处理H264的SPS/PPS和AAC的ASC
			*  
			*  使用示例：
			*  @code
			*  auto fragment = muxer.GetIdleFragment();
			*  auto packet = std::make_shared<Packet>();
			*  // ... packet包含SPS/PPS数据 ...
			*  muxer.ParseCodec(fragment, packet);
			*  @endcode
			*/
			void ParseCodec(std::shared_ptr<Fragment> & fragment, std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet>&packet);
		private:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 检查是否为编解码器头（Is Codec Header）
			*  
			*  该方法用于检查媒体包是否为编解码器头信息。编解码器头信息包括
			*  H264的SPS/PPS和AAC的ASC等，需要特殊处理。
			*  
			*  编解码器头判断（Codec Header Detection）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Check Packet Type:                                            |
			*   |  - H264 SPS: NALU type = 7                                     |
			*   |  - H264 PPS: NALU type = 8                                     |
			*   |  - AAC ASC: ADTS header or specific pattern                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param packet 指向媒体包的共享指针，需要检查的包
			*  @return 如果包是编解码器头信息，返回true；否则返回false
			*  @note 该方法通过检查包的NALU类型或特定模式来判断
			*  @note 编解码器头信息需要写入切片的开头
			*  
			*  使用示例：
			*  @code
			*  if (muxer.IsCodecHeader(packet)) {
			*      // 处理编解码器头信息
			*  }
			*  @endcode
			*/
			bool   IsCodecHeader(const std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet>& packet);
		private:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 切片窗口（Fragment Window）
			*  
			*  该成员变量用于管理HLS切片窗口。切片窗口是一个滑动窗口，
			*  用于维护当前可用的切片列表，并自动释放过期的切片。
			*  
			*  窗口功能：
			*  - 管理活跃切片列表（fragments_）
			*  - 管理空闲切片列表（free_fragments_）
			*  - 生成M3U8播放列表
			*  - 自动释放过期切片
			*  
			*  @note 窗口大小默认值为5，可以通过构造函数配置
			*  @note 窗口是线程安全的，所有操作都有互斥锁保护
			*/
			FragmentWindow fragment_window_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief MPEG-TS编码器（MPEG-TS Encoder）
			*  
			*  该成员变量用于将媒体包（音频/视频）编码为MPEG-TS格式。
			*  TS格式是HLS协议使用的标准传输流格式。
			*  
			*  编码器功能：
			*  - 将音频包编码为TS音频包
			*  - 将视频包编码为TS视频包
			*  - 生成TS包的PID、时间戳等信息
			*  - 处理PAT、PMT等PSI表
			*  
			*  @note 编码器在构造函数中自动初始化
			*  @note 编码器将媒体包转换为188字节的TS包
			*/
			libmedia_transfer_protocol::libmpeg::MpegTsEncoder         encoder_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 当前切片（Current Fragment）
			*  
			*  该成员变量用于存储当前正在写入的切片。当切片大小达到阈值时，
			*  会创建一个新的切片，当前切片会被添加到切片窗口中。
			*  
			*  切片生命周期（Fragment Lifecycle）：
			*  1. 创建：current_fragment_ = GetIdleFragment()
			*  2. 写入：写入TS数据到current_fragment_
			*  3. 完成：当大小达到阈值时，添加到窗口
			*  4. 释放：current_fragment_ = nullptr，准备创建新切片
			*  
			*  @note 当前切片为空时（nullptr），表示需要创建新切片
			*  @note 切片大小通过min_fragment_size_和max_fragment_size_控制
			*/
			std::shared_ptr<Fragment>	 current_fragment_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 流名称（Stream Name）
			*  
			*  该成员变量用于存储流的名称。流名称通常用于生成切片文件的基础文件名。
			*  
			*  文件名生成：
			*  - 基础文件名：stream_name_
			*  - 完整文件名："{stream_name_}_{sequence_no:04d}.ts"
			*  - 例如：如果stream_name_ = "live"，序列号 = 1，则文件名为 "live_0001.ts"
			*  
			*  @note 流名称在构造函数中设置，通常不会更改
			*  @note 流名称用于生成切片文件名和M3U8播放列表中的URI
			*/
			std::string  stream_name_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 切片序列号（Fragment Sequence Number）
			*  
			*  该成员变量用于存储当前切片的序列号。序列号用于标识切片在播放列表中的顺序，
			*  客户端通过序列号来确保按顺序播放切片。
			*  
			*  序列号管理：
			*  - 序列号从0或1开始，按顺序递增
			*  - 每个新切片都会分配一个新的序列号
			*  - 序列号用于生成文件名和M3U8播放列表
			*  
			*  @note 序列号用于M3U8播放列表中的#EXT-X-MEDIA-SEQUENCE字段
			*  @note 序列号通常与文件名中的序号一致
			*/
			int32_t     fragment_seq_no_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 最小切片大小（Minimum Fragment Size）
			*  
			*  该成员变量用于存储切片的最小大小（以TS包数量为单位）。
			*  当切片大小达到最小大小时，可以创建新的切片。
			*  
			*  切片大小说明：
			*  - 单位：TS包数量（每个TS包188字节）
			*  - 默认值：3000包（约564KB）
			*  - 用途：控制切片的最小大小，避免切片过小
			*  
			*  切片大小计算（Fragment Size Calculation）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  fragment->Size() / 188 = TS packet count                     |
			*   |  If packet_count >= min_fragment_size_:                        |
			*   |    - Can create new fragment (if other conditions met)         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 切片大小以TS包数量为单位，每个TS包188字节
			*  @note 最小大小用于控制切片的质量和网络效率
			*/
			int32_t   min_fragment_size_{ 3000 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 最大切片大小（Maximum Fragment Size）
			*  
			*  该成员变量用于存储切片的最大大小（以TS包数量为单位）。
			*  当切片大小达到最大大小时，必须创建新的切片。
			*  
			*  切片大小说明：
			*  - 单位：TS包数量（每个TS包188字节）
			*  - 默认值：12000包（约2.2MB）
			*  - 用途：强制限制切片的最大大小，避免切片过大
			*  
			*  切片大小控制（Fragment Size Control）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  fragment->Size() / 188 = TS packet count                     |
			*   |  If packet_count >= max_fragment_size_:                        |
			*   |    - Force create new fragment (enforce max size)              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 切片大小以TS包数量为单位，每个TS包188字节
			*  @note 最大大小用于强制限制切片大小，避免切片过大导致内存问题
			*  @note 当切片大小达到最大值时，会立即创建新切片
			*/
			int32_t   max_fragment_size_{ 12000 };

		};
	}
}

#endif // 