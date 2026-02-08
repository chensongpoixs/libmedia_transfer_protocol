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

purpose:		Fragment - HLS切片数据管理
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


#include "libmedia_transfer_protocol/libhls/cfragment.h"
#include "libmedia_transfer_protocol/libmpeg/cmpeg_type.h"
#include "libmedia_transfer_protocol/libmpeg/mpegts_encoder.h"
#include <fcntl.h>
#include "rtc_base/time_utils.h"
#include <errno.h> 

namespace libmedia_transfer_protocol
{
	namespace libhls
	{
		/**
		*  @brief 添加时间戳实现
		*  
		*  该方法向切片添加时间戳信息，用于计算切片的总时长。
		*  
		*  时间戳处理流程：
		*  1. 如果是第一个时间戳（start_dts_为-1），设置start_dts_为pts
		*  2. 更新start_dts_为最小时间戳（处理时间戳回绕）
		*  3. 计算切片当前时长：duration_ = pts - start_dts_
		*  
		*  @param pts 表示时间戳（Presentation Time Stamp），单位为毫秒或90KHz时钟
		*  @note 该方法在写入媒体数据时自动调用
		*/
		void Fragment::AppendTimestamp(int64_t pts)
		{
			if (start_dts_ == -1)
			{
				// 第一个时间戳，设置起始时间戳
				start_dts_ = pts;
			}

			// 更新起始时间戳为最小值（处理时间戳回绕）
			start_dts_ = std::min(start_dts_, pts);
			
			// 计算切片时长
			duration_ = pts - start_dts_;
		}
		
		/**
		*  @brief 写入数据实现
		*  
		*  该方法向切片缓冲区写入数据。如果缓冲区空间不足，会自动扩展。
		*  
		*  写入流程：
		*  1. 检查data_是否已分配，如果未分配则创建新的Packet对象
		*  2. 检查缓冲区剩余空间是否足够
		*  3. 如果空间不足，按kFragmentStepSize步长扩展缓冲区
		*  4. 创建新的Packet对象，复制旧数据到新缓冲区
		*  5. 将新数据复制到缓冲区末尾
		*  6. 更新data_size_以反映当前数据大小
		*  
		*  缓冲区扩展策略：
		*  - 初始大小：buf_size_（默认512KB）
		*  - 扩展步长：kFragmentStepSize（128KB）
		*  - 扩展条件：data_->Space() < size
		*  - 扩展方式：buf_size_ += kFragmentStepSize，直到满足需求
		*  
		*  @param buf 指向要写入数据的缓冲区指针，不能为空
		*  @param size 要写入的数据大小，单位为字节
		*  @return 返回实际写入的数据大小，单位为字节
		*  @note 该方法会自动扩展缓冲区，确保有足够空间写入数据
		*/
		int32_t Fragment::Write(void * buf, size_t size)
		{
			// 检查data_是否已分配
			if (!data_)
			{
				data_ = libmedia_transfer_protocol::libmpeg::Packet::NewPacket(buf_size_);
			}

			// 检查缓冲区剩余空间是否足够
			if (data_->Space() < size)
			{
				// 空间不足，扩展缓冲区
				buf_size_ += kFragmentStepSize;
				while (data_size_ + size > buf_size_)
				{
					buf_size_ += kFragmentStepSize;
				}
				
				// 创建新的Packet对象
				std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet> new_pkt = 
					libmedia_transfer_protocol::libmpeg::Packet::NewPacket(buf_size_);
				
				// 复制旧数据到新缓冲区
				memcpy(new_pkt->Data(), data_->Data(), data_->PacketSize());
				new_pkt->SetPacketSize(data_->PacketSize());
				
				// 更新data_指针（C++11 shared_ptr 自动释放旧内存）
				data_ = new_pkt;
			}

			// 将新数据复制到缓冲区末尾
			memcpy(data_->Data() + data_->PacketSize(), buf, size);
			data_->UpdatePacketSize(size);
			data_size_ += size;
			
			return size;
		}
		
		/**
		*  @brief 获取数据指针实现
		*  
		*  该方法返回指向切片数据缓冲区当前写入位置的指针。
		*  
		*  @return 返回指向切片数据缓冲区当前写入位置的字符指针
		*  @note 返回的指针指向缓冲区的末尾，可以用于继续写入数据
		*/
		char * Fragment::Data()
		{
			return data_->Data() + data_->PacketSize();
		}
		
		/**
		*  @brief 获取数据大小实现
		*  
		*  该方法返回切片数据的实际大小。
		*  
		*  @return 返回切片数据的实际大小，单位为字节
		*  @note 返回的大小是已写入的数据大小，而不是缓冲区大小
		*/
		int32_t Fragment::Size()
		{
			return  data_->PacketSize();
		}


		/**
		*  @brief 获取切片时长实现
		*  
		*  该方法返回切片的播放时长。
		*  
		*  @return 返回切片的时长，单位为毫秒或90KHz时钟
		*  @note 时长通过最后一个时间戳减去起始时间戳计算
		*/
		int64_t  Fragment::Duration() const
		{
			return duration_;
		}
		
		/**
		*  @brief 获取文件名实现
		*  
		*  该方法返回切片的文件名。
		*  
		*  @return 返回切片的文件名常量引用
		*  @note 文件名格式："{base_name}_{timestamp}.ts"
		*/
		const std::string &Fragment::FileName() const
		{
			return filename_;
		}
		
		/**
		*  @brief 设置基础文件名实现
		*  
		*  该方法设置切片的基础文件名，并自动生成完整的文件名。
		*  
		*  文件名生成流程：
		*  1. 清空filename_字符串
		*  2. 添加基础文件名（v）
		*  3. 添加下划线分隔符
		*  4. 添加当前系统时间戳（毫秒）
		*  5. 添加扩展名 ".ts"
		*  
		*  文件名格式：
		*  - "{base_name}_{timestamp_ms}.ts"
		*  - 例如："stream_1704067200000.ts"
		*  
		*  @param v 基础文件名字符串，例如 "stream" 或 "live"
		*  @note 使用系统时间戳确保文件名唯一性
		*/
		void Fragment::SetBaseFileName(const std::string& v)
		{
			filename_.clear();
			filename_.append(v);
			filename_.append("_");
			filename_.append(std::to_string(rtc::SystemTimeMillis()));  // 添加时间戳
			filename_.append(".ts");
		}

		/**
		*  @brief 获取序列号实现
		*  
		*  该方法返回切片的序列号。
		*  
		*  @return 返回切片的序列号
		*  @note 序列号用于标识切片在播放列表中的顺序
		*/
		int32_t Fragment::SequenceNo() const
		{
			return sequence_no_;
		}
		
		/**
		*  @brief 设置序列号实现
		*  
		*  该方法设置切片的序列号。
		*  
		*  @param no 序列号整数，通常从0或1开始，按顺序递增
		*  @note 序列号应该是唯一的，避免重复
		*/
		void Fragment::SetSequenceNo(int32_t no)
		{
			sequence_no_ = no;
		}

		/**
		*  @brief 重置切片实现
		*  
		*  该方法重置切片的所有状态，使其可以重新使用。
		*  
		*  重置操作：
		*  1. 重置duration_为0
		*  2. 重置sequence_no_为0
		*  3. 重置data_size_为0
		*  4. 重置start_dts_为-1
		*  5. 重置sps_pps_appended_为false（每个切片都需要SPS/PPS）
		*  6. 如果data_不为空，重置PacketSize为0（保留缓冲区）
		*  
		*  @note 重置后，切片可以用于存储新的媒体数据
		*  @note 缓冲区不会被释放，可以复用
		*/
		void  Fragment::Reset()
		{
			duration_ = 0;
			sequence_no_ = 0;
			data_size_ = 0;
			start_dts_ = -1;
			
			// 每个切片都需要SPS/PPS
			sps_pps_appended_ = false;
			
			// 重置PacketSize，但保留缓冲区
			if (data_)
			{
				data_->SetPacketSize(0);
			}
		}
		
		/**
		*  @brief 获取切片数据实现
		*  
		*  该方法返回切片的底层数据包对象。
		*  
		*  @return 返回指向Packet对象的共享指针引用
		*  @note 返回的是内部数据对象的引用，修改会影响切片数据
		*/
		std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet>& Fragment::FragmentData()
		{
			return data_;
		}
	}
}