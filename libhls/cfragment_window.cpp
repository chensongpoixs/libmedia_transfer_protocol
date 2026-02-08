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

purpose:		Fragment Window - HLS切片窗口管理
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
#include "libmedia_transfer_protocol/libhls/cfragment_window.h"
#include <sstream> 
#include <cmath> 


#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"



namespace libmedia_transfer_protocol {
	namespace libhls
	{
		namespace
		{
			/**
			*  @brief 空切片指针（Null Fragment Pointer）
			*  
			*  该静态变量用于表示空切片指针。当查找切片失败时，返回该空指针。
			*  使用静态变量避免每次返回时创建临时对象。
			*/
			static std::shared_ptr<Fragment>    fragment_null;
		}

		/**
		*  @brief FragmentWindow构造函数实现
		*  
		*  初始化切片窗口，设置窗口大小。
		*  
		*  @param size 切片窗口大小，默认值为5
		*/
		FragmentWindow::FragmentWindow(int32_t size )
			:window_size_(size)
		{ 
		}
		
		/**
		*  @brief FragmentWindow析构函数实现
		*  
		*  清理切片窗口资源。所有切片和资源会自动释放。
		*/
		FragmentWindow::~FragmentWindow()
		{}
 
		/**
		*  @brief 添加切片实现
		*  
		*  该方法将新切片添加到切片窗口中。如果窗口已满，会先释放最老的切片。
		*  
		*  添加流程：
		*  1. 获取互斥锁，保护fragments_列表
		*  2. 将新切片添加到fragments_列表末尾
		*  3. 调用Shrink()释放过期切片
		*  4. 调用UpdatePlayList()更新播放列表
		*  
		*  @param fragment 要添加的切片，使用右值引用进行移动语义传递
		*  @note 该方法线程安全，使用互斥锁保护
		*/
		void FragmentWindow::AppendFragment(std::shared_ptr<Fragment> &&fragment)
		{
			{
				std::lock_guard<std::mutex> lk(lock_);
				fragments_.emplace_back(std::move(fragment));
			}

			// 释放过期切片
			Shrink();

			// 更新播放列表
			UpdatePlayList();
		}
		

		/**
		*  @brief 获取空闲切片实现
		*  
		*  该方法从空闲切片列表中获取一个切片。如果空闲列表为空，创建新切片。
		*  
		*  获取流程：
		*  1. 获取互斥锁，保护free_fragments_列表
		*  2. 检查free_fragments_是否为空
		*  3. 如果非空，取出第一个切片并返回
		*  4. 如果为空，创建新的Fragment实例并返回
		*  
		*  @return 返回指向空闲切片的共享指针
		*  @note 该方法线程安全，使用互斥锁保护
		*/
		std::shared_ptr<Fragment> FragmentWindow::GetIdleFragment()
		{
			std::lock_guard<std::mutex> lk(lock_);

			if (free_fragments_.empty())
			{
				// 空闲列表为空，创建新切片
				return  std::make_shared<Fragment>();
			}
			
			// 从空闲列表中取出第一个切片
			auto p =  free_fragments_[0];
			free_fragments_.erase(free_fragments_.begin() );
			return p;
		}

		/**
		*  @brief 根据文件名获取切片实现
		*  
		*  该方法在切片窗口中查找指定文件名的切片。
		*  
		*  查找流程：
		*  1. 获取互斥锁，保护fragments_列表
		*  2. 遍历fragments_列表
		*  3. 比较每个切片的文件名与给定文件名
		*  4. 如果找到匹配的切片，返回该切片的引用
		*  5. 如果未找到，返回空指针（fragment_null）
		*  
		*  @param name 切片的文件名，例如 "stream_0001.ts"
		*  @return 返回指向匹配切片的共享指针常量引用。如果未找到，返回空指针
		*  @note 该方法线程安全，使用互斥锁保护
		*/
		const std::shared_ptr<Fragment> &FragmentWindow::GetFragmentByName(const std::string & name)
		{
			std::lock_guard<std::mutex> lk(lock_);
			for (auto & f : fragments_)
			{
				if (f->FileName() == name)
				{
					return f;
				}
			}
			return fragment_null;
		}
		
		/**
		*  @brief 获取播放列表实现
		*  
		*  该方法返回M3U8格式的播放列表字符串。
		*  
		*  @return 返回M3U8格式的播放列表字符串
		*  @note 该方法线程安全，使用互斥锁保护
		*/
		std::string FragmentWindow::GetPlayList()  
		{
			std::lock_guard<std::mutex> lk(lock_);
			return playlist_;
		}

		/**
		*  @brief 更新播放列表实现
		*  
		*  该方法根据当前切片窗口中的切片重新生成M3U8播放列表。
		*  
		*  更新流程：
		*  1. 获取互斥锁，保护fragments_列表
		*  2. 检查fragments_大小是否超过window_size_
		*  3. 如果超过，计算需要移除的切片数量
		*  4. 将最老的切片移动到free_fragments_列表
		*  5. 重置被移除切片的状态（调用Reset()）
		*  
		*  @note 该方法线程安全，使用互斥锁保护
		*/
		void FragmentWindow::UpdatePlayList()
		{
			std::lock_guard<std::mutex> lk(lock_);
			int32_t  remove_index = -1;
			
			// 检查窗口是否已满
			if (fragments_.size() <= window_size_)
			{
				return;
			}
			
			// 计算需要移除的切片数量
			remove_index = fragments_.size() - window_size_;

			// 移除最老的切片
			for (int32_t i = 0; i < remove_index && !fragments_.empty(); i++)
			{
				auto p = *fragments_.begin();
				fragments_.erase(fragments_.begin());
				p->Reset();  // 重置切片状态
				free_fragments_.emplace_back(std::move(p));  // 移动到空闲列表
			}
		}


		/**
		*  @brief 收缩窗口实现（生成M3U8播放列表）
		*  
		*  该方法根据当前切片窗口中的切片生成M3U8播放列表。
		*  
		*  生成流程：
		*  1. 获取互斥锁，保护fragments_列表
		*  2. 检查fragments_是否为空或少于3个切片
		*  3. 构造M3U8播放列表头部（#EXTM3U、#EXT-X-VERSION等）
		*  4. 计算目标切片时长（取最后5个切片的最大时长）
		*  5. 遍历fragments_列表，为每个切片添加播放列表条目
		*  6. 每个条目包含#EXTINF标签（时长）和文件名
		*  7. 添加#EXT-X-ENDLIST标签（表示播放列表结束）
		*  8. 更新playlist_字符串
		*  
		*  M3U8播放列表格式：
		*  - #EXTM3U：M3U8文件头部标签
		*  - #EXT-X-VERSION:3：协议版本号
		*  - #EXT-X-TARGETDURATION：目标切片时长（秒）
		*  - #EXT-X-MEDIA-SEQUENCE：媒体序列号（第一个切片的序列号）
		*  - #EXTINF：切片时长（秒）和文件名条目
		*  - #EXT-X-ENDLIST：播放列表结束标记
		*  
		*  @note 该方法线程安全，使用互斥锁保护
		*  @note 目标切片时长取最后5个切片的最大时长，向上取整
		*  @note 切片时长精度为3位小数（毫秒级别）
		*/
		void FragmentWindow::Shrink()
		{
			std::lock_guard<std::mutex> lk(lock_);
			
			// 检查切片列表是否为空或少于3个切片
			if (fragments_.empty() || fragments_.size() < 3)
			{
				return;
			}
	
			/*
			M3U8播放列表格式示例：
			#EXTM3U
			#EXT-X-VERSION:3
			#EXT-X-TARGETDURATION:10
			#EXT-X-MEDIA-SEQUENCE:1057
			#EXT_X_TOTAL_DURATION:1800
			#EXTINF:10.00,
			34020000001320000001_34020000001320000002-20250329003001-1057.ts
			#EXTINF:10.00,
			34020000001320000001_34020000001320000002-20250329003011-1058.ts
			#EXT-X-ENDLIST
			*/
			
			std::ostringstream ss;

			// 写入M3U8文件头部标签
			ss << "#EXTM3U\n";
			
			// 写入协议版本号
			ss << "#EXT-X-VERSION:3 \n";
			
			// 计算目标切片时长（取最后5个切片的最大时长）
			int32_t i = fragments_.size() > 5 ? (fragments_.size() - 5) : 0;
			int32_t j = i;
			int32_t  max_duration = 0;
			for (; j < fragments_.size(); ++j)
			{
				max_duration = std::max(max_duration, (int32_t )fragments_[j]->Duration());
			}
			// 向上取整，转换为秒
			int32_t  target_duration = (int32_t) ceil((max_duration / 1000.0));

			// 写入目标切片时长
			ss << "#EXT-X-TARGETDURATION:" << target_duration << "\n";
			
			// 写入媒体序列号（第一个切片的序列号）
			ss << "#EXT-X-MEDIA-SEQUENCE:" << fragments_[i]->SequenceNo() << "\n";
			
			// 设置浮点数精度为3位小数
			ss.precision(3);
			ss.setf(std::ios::fixed, std::ios::floatfield);
			
			// 遍历切片列表，为每个切片添加播放列表条目
			for (; i < fragments_.size(); ++i)
			{
				// 写入切片时长（转换为秒）
				ss << "#EXTINF:" << fragments_[i]->Duration()/1000.0 <<"\n";
				// 写入切片文件名
				ss << fragments_[i]->FileName() << "\n";
				
#if TEST_TS
				// 测试模式：将切片数据写入文件
				TestStreamWriter test_w(fragments_[i]->FileName());
				test_w.Write(fragments_[i]->FragmentData()->Data(), fragments_[i]->FragmentData()->PacketSize());
#endif
			}
			
			// 写入播放列表结束标记
			ss << "#EXT-X-ENDLIST\n";

			// 更新播放列表字符串
			playlist_.clear();
			playlist_ = std::move(ss.str());

			// 记录播放列表日志
			LIBHLS_LOG(LS_INFO) << "playlist:" << playlist_;
		}
	}
}