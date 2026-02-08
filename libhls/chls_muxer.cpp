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

purpose:		HLS Muxer - HLS复用器实现
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
#include "libmedia_transfer_protocol/libhls//chls_muxer.h"  
#include "rtc_base/string_encode.h"
namespace libmedia_transfer_protocol {
	namespace libhls
	{
		/**
		*  @brief HLSMuxer构造函数实现
		*  
		*  初始化HLS复用器，设置流名称。
		*  
		*  初始化流程：
		*  1. 设置stream_name_为指定的会话名称
		*  2. 如果会话名称包含路径分隔符（/），提取最后一部分作为流名称
		*  3. 初始化fragment_window_（默认窗口大小为5）
		*  4. 初始化encoder_（MPEG-TS编码器）
		*  5. 设置current_fragment_为空指针
		*  6. 初始化fragment_seq_no_为0
		*  
		*  流名称处理：
		*  - 如果session_name包含3个路径部分（例如："app/stream/name"），提取最后一部分
		*  - 否则使用完整的session_name作为流名称
		*  
		*  @param session_name 会话名称字符串，用于生成切片文件名
		*  @note 会话名称通常用于生成切片文件的基础文件名
		*/
		HLSMuxer::HLSMuxer(const std::string &session_name)
			:stream_name_(session_name)
		{
			std::vector<std::string> list;
			rtc::split(session_name, '/', &list);

			// 如果会话名称包含3个路径部分，提取最后一部分作为流名称
			if (list.size() == 3)
			{
				stream_name_ = list[2];
			}
		}
	 
		/**
		*  @brief 获取播放列表实现
		*  
		*  该方法返回M3U8格式的播放列表字符串。
		*  
		*  @return 返回M3U8格式的播放列表字符串
		*  @note 播放列表由FragmentWindow生成和管理
		*/
		std::string  HLSMuxer::PlayList()
		{
			return fragment_window_.GetPlayList();
		}
		
		/**
		*  @brief 检查是否为编解码器头实现
		*  
		*  该方法检查媒体包是否为编解码器头信息。编解码器头信息的第二个字节为0。
		*  
		*  检查流程：
		*  1. 检查包大小是否大于1字节
		*  2. 读取第二个字节（packet->Data() + 1）
		*  3. 如果第二个字节为0，表示这是编解码器头信息
		*  
		*  编解码器头类型：
		*  - H264 SPS/PPS：第二个字节为0
		*  - AAC ASC：第二个字节为0
		*  
		*  @param packet 指向媒体包的共享指针，需要检查的包
		*  @return 如果包是编解码器头信息，返回true；否则返回false
		*  @note 该方法通过检查第二个字节是否为0来判断
		*/
		bool HLSMuxer::IsCodecHeader(const std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet>& packet)
		{
			if (packet->PacketSize() > 1)
			{
				const char *b = packet->Data() + 1;

				// 第二个字节不是0就是一个序列头
				if (*b == 0)
				{
					return true;
				}
			}

			return false;
		}
		
		/**
		*  @brief 处理媒体包实现
		*  
		*  该方法处理输入的媒体包（音频或视频包），将其编码为TS格式并写入切片。
		*  
		*  处理流程：
		*  1. 检查当前切片是否存在
		*  2. 如果存在，检查切片大小是否达到阈值
		*  3. 如果达到阈值，将当前切片添加到窗口，并创建新切片
		*  4. 如果当前切片不存在，创建新切片
		*  5. 设置切片的基础文件名和序列号
		*  6. 写入PAT和PMT表到切片开头
		*  7. 检查包是否为编解码器头信息
		*  8. 如果是编解码器头，解析编解码器信息
		*  9. 编码媒体包为TS格式
		*  10. 写入TS数据到当前切片
		*  
		*  切片创建条件：
		*  - 当前切片为空（current_fragment_ == nullptr）
		*  - 当前切片是关键帧且时长 >= min_fragment_size_
		*  - 当前切片时长 >= max_fragment_size_（强制创建）
		*  
		*  @param packet 指向媒体包的共享指针，包含音频或视频数据
		*  @note 媒体包可以是音频包或视频包
		*  @note 如果包是编解码器头信息（SPS/PPS等），会进行特殊处理
		*/
		void HLSMuxer::OnPacket(std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet> & packet)
		{
			// 检查当前切片是否存在
			if (current_fragment_)
			{
				bool  is_key = packet->IsKeyFrame();
				
				// 检查是否需要创建新切片
				if ((is_key && current_fragment_->Duration() >= min_fragment_size_)
					|| current_fragment_->Duration() >= max_fragment_size_)
				{
					// 将当前切片添加到窗口
					fragment_window_.AppendFragment(std::move(current_fragment_) );
					current_fragment_.reset();
				}
			}

			// 如果当前切片不存在，创建新切片
			if (!current_fragment_)
			{
				// 从窗口获取空闲切片
				current_fragment_ = fragment_window_.GetIdleFragment();
				if (!current_fragment_)
				{
					// 如果空闲列表为空，创建新切片
					current_fragment_ = std::make_shared<Fragment>();
				}
				
				// 重置切片状态
				current_fragment_->Reset();
				
				// 设置切片的基础文件名
				current_fragment_->SetBaseFileName(stream_name_);
				
				// 设置切片的序列号
				current_fragment_->SetSequenceNo(fragment_seq_no_++);
				
				// 每个切片生成开始位置写入PAT、PMT
				encoder_.WritePatPmt(current_fragment_.get());
			}
			
			// 检查是否为编解码器头信息
			if (IsCodecHeader(packet))
			{
				// 解析编解码器信息
				ParseCodec(current_fragment_, packet);
			}
			
			// 获取时间戳
			int64_t dts = packet->TimeStamp();
			
			// 编码媒体包为TS格式，并写入当前切片
			encoder_.Encode(current_fragment_.get(), packet, dts);
		}
		
		/**
		*  @brief 根据文件名获取切片实现
		*  
		*  该方法在切片窗口中查找指定文件名的切片。
		*  
		*  @param name 切片的文件名，例如 "stream_0001.ts"
		*  @return 返回指向匹配切片的共享指针。如果未找到，返回空指针
		*  @note 该方法委托给FragmentWindow的GetFragmentByName方法
		*/
		std::shared_ptr<Fragment> HLSMuxer::GetFragment(const std::string & name)
		{
			return fragment_window_.GetFragmentByName(name);
		}
		
		/**
		*  @brief 解析编解码器信息实现
		*  
		*  该方法解析媒体包中的编解码器头信息（如H264的SPS/PPS或AAC的ASC）。
		*  
		*  解析流程：
		*  1. 获取包数据指针
		*  2. 检查包类型（音频或视频）
		*  3. 如果是音频包，提取音频编码格式ID（高4位）
		*  4. 如果是视频包，提取视频编码格式ID（低4位）
		*  5. 调用encoder_.SetStreamType()设置编码器的流类型
		*  
		*  编码格式ID提取：
		*  - 音频编码格式ID：(*data & 0xF0) >> 4（高4位）
		*  - 视频编码格式ID：(*data & 0x0F)（低4位）
		*  
		*  @param fragment 指向切片的共享指针，用于写入编解码器头信息
		*  @param packet 指向媒体包的共享指针，包含编解码器头数据
		*  @note 编解码器头信息通常需要写入每个切片的开头
		*/
		void HLSMuxer::ParseCodec(std::shared_ptr<Fragment> & fragment, std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet>&packet)
		{
			char   *data = packet->Data();
			
			// 检查是否为音频包
			if (packet->IsAudio())
			{
				// 提取音频编码格式ID（高4位）
				libmedia_transfer_protocol::libmpeg::AudioCodecID id = 
					(libmedia_transfer_protocol::libmpeg::AudioCodecID)((*data & 0XF0) >> 4);
				
				// 设置编码器的流类型（仅音频）
				encoder_.SetStreamType(fragment.get(), 
					libmedia_transfer_protocol::libmpeg::kVideoCodecIDReserved, id);
			}
			
			// 检查是否为视频包
			if (packet->IsVideo())
			{
				// 提取视频编码格式ID（低4位）
				libmedia_transfer_protocol::libmpeg::VideoCodecID id = 
					(libmedia_transfer_protocol::libmpeg::VideoCodecID)((*data & 0X0F));
				
				// 设置编码器的流类型（仅视频）
				encoder_.SetStreamType(fragment.get(), id, 
					libmedia_transfer_protocol::libmpeg::kAudioCodecIDReserved);
			}
		}
	}
}