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

#ifndef _C_FRAGMENT_WINDOW_
#define _C_FRAGMENT_WINDOW_


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
#include "libmedia_transfer_protocol/libhls/cfragment.h"
#include <mutex>
#include <vector>
#include<algorithm>
namespace libmedia_transfer_protocol
{
	namespace libhls
	{

		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief HLS 切片窗口类（HLS Fragment Window）
		*  
		*  FragmentWindow类用于管理HLS协议中的切片窗口。切片窗口是一个滑动窗口，
		*  用于维护当前可用的切片列表，并自动释放过期的切片。
		*  
		*  HLS切片窗口说明：
		*  - HLS协议使用切片窗口来管理媒体流的切片
		*  - 切片窗口是一个固定大小的容器，通常包含3-10个切片
		*  - 当窗口满时，最老的切片会被移除，新的切片会被添加
		*  - 切片窗口用于生成M3U8播放列表，客户端通过播放列表获取切片列表
		*  
		*  切片窗口数据结构（Fragment Window Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                  Window Size (32 bits)                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                  Fragment List (variable)                      |
		*   |                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |                  | Fragment 0 (Sequence No = N)              | |
		*   |                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |                  | Fragment 1 (Sequence No = N+1)            | |
		*   |                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |                  | ...                                        | |
		*   |                  | Fragment K (Sequence No = N+K)            | |
		*   |                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                  Free Fragment List (variable)                 |
		*   |                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |                  | Free Fragment 0                            | |
		*   |                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |                  | ...                                        | |
		*   |                  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                  Playlist (M3U8 format, variable)              |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  M3U8播放列表格式（M3U8 Playlist Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  #EXTM3U                                                       |
		*   |  #EXT-X-VERSION:3                                              |
		*   |  #EXT-X-TARGETDURATION:10                                      |
		*   |  #EXT-X-MEDIA-SEQUENCE:0                                       |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  #EXTINF:10.000,                                               |
		*   |  stream_0000.ts                                                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  #EXTINF:10.000,                                               |
		*   |  stream_0001.ts                                                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                                                               :
		*   |  ... more fragments ...                                        |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  窗口操作流程（Window Operation Flow）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  1. AppendFragment()                                           |
		*   |     - Add new fragment to fragments_                           |
		*   |     - If window is full, move oldest to free_fragments_        |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  2. Shrink()                                                   |
		*   |     - Remove oldest fragments from fragments_                  |
		*   |     - Move to free_fragments_ for reuse                        |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  3. GetIdleFragment()                                          |
		*   |     - Get from free_fragments_ if available                    |
		*   |     - Create new fragment if free list is empty                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  4. UpdatePlayList()                                           |
		*   |     - Generate M3U8 playlist from fragments_                   |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note FragmentWindow是线程安全的，使用互斥锁保护所有操作
		*  @note 切片窗口大小通常为3-10，默认值为5
		*  @note 空闲切片列表用于切片复用，减少内存分配和释放
		*  
		*  使用示例：
		*  @code
		*  FragmentWindow window(5);  // 创建大小为5的窗口
		*  auto fragment = window.GetIdleFragment();
		*  window.AppendFragment(std::move(fragment));
		*  std::string playlist = window.GetPlayList();
		*  @endcode
		*/
		class FragmentWindow
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建FragmentWindow实例。切片窗口大小决定了窗口中
			*  可以同时保留的切片数量。
			*  
			*  初始化流程：
			*  1. 设置window_size_为指定大小（默认5）
			*  2. 初始化fragments_为空向量
			*  3. 初始化free_fragments_为空向量
			*  4. 初始化playlist_为空字符串
			*  5. 初始化互斥锁lock_
			*  
			*  @param size 切片窗口大小，默认值为5。窗口大小决定了可以同时保留的切片数量
			*  @note 窗口大小通常为3-10，过小可能导致客户端无法获取足够的切片
			*  @note 窗口大小过大会占用更多内存
			*  
			*  使用示例：
			*  @code
			*  FragmentWindow window(5);  // 创建大小为5的窗口
			*  @endcode
			*/
			FragmentWindow(int32_t size = 5) ;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理FragmentWindow实例。所有切片和资源会自动释放。
			*  
			*  清理流程：
			*  1. 清空fragments_列表，释放所有活跃切片
			*  2. 清空free_fragments_列表，释放所有空闲切片
			*  3. 清空playlist_字符串
			*  4. 销毁互斥锁
			*  
			*  @note 使用智能指针管理切片，自动释放内存
			*/
			~FragmentWindow()  ;

		public:

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 添加切片（Append Fragment）
			*  
			*  该方法用于向切片窗口添加一个新的切片。如果窗口已满，最老的切片会被移除
			*  并移动到空闲切片列表中以供复用。
			*  
			*  添加流程：
			*  1. 检查窗口是否已满（fragments_.size() >= window_size_）
			*  2. 如果窗口已满，将最老的切片（fragments_[0]）移动到free_fragments_
			*  3. 重置最老切片的状态（调用Reset()方法）
			*  4. 将新切片添加到fragments_列表的末尾
			*  5. 更新播放列表（调用UpdatePlayList()）
			*  
			*  窗口滑动示意图（Window Sliding Illustration）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Before Append (window_size_ = 5):                             |
			*   |  [Frag0] [Frag1] [Frag2] [Frag3] [Frag4]                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  After Append (Frag5 added):                                   |
			*   |  [Frag1] [Frag2] [Frag3] [Frag4] [Frag5]                       |
			*   |  Frag0 -> moved to free_fragments_                             |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param fragment 要添加的切片，使用右值引用进行移动语义传递
			*  @note 该方法线程安全，使用互斥锁保护
			*  @note 如果窗口已满，最老的切片会被移除并移动到空闲列表
			*  @note 添加后会自动更新播放列表
			*  
			*  使用示例：
			*  @code
			*  auto fragment = window.GetIdleFragment();
			*  // ... 填充fragment数据 ...
			*  window.AppendFragment(std::move(fragment));
			*  @endcode
			*/
			void AppendFragment(std::shared_ptr<Fragment> &&fragment);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 收缩窗口（Shrink Window）
			*  
			*  该方法用于释放窗口中最老的切片。当切片过期或需要释放内存时，
			*  可以调用此方法主动释放切片。
			*  
			*  收缩流程：
			*  1. 检查fragments_列表是否为空
			*  2. 如果非空，将最老的切片（fragments_[0]）移动到free_fragments_
			*  3. 重置最老切片的状态（调用Reset()方法）
			*  4. 从fragments_列表中移除最老的切片
			*  5. 更新播放列表（调用UpdatePlayList()）
			*  
			*  收缩示意图（Shrink Illustration）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Before Shrink:                                              |
			*   |  [Frag0] [Frag1] [Frag2] [Frag3] [Frag4]                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  After Shrink:                                               |
			*   |  [Frag1] [Frag2] [Frag3] [Frag4]                              |
			*   |  Frag0 -> moved to free_fragments_                            |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 该方法线程安全，使用互斥锁保护
			*  @note 收缩后会自动更新播放列表
			*  @note 被移除的切片会被重置并移动到空闲列表，可以复用
			*  
			*  使用示例：
			*  @code
			*  window.Shrink();  // 释放最老的切片
			*  @endcode
			*/
			void Shrink();

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取空闲切片（Get Idle Fragment）
			*  
			*  该方法用于获取一个空闲的切片。如果空闲切片列表中有可用切片，
			*  则返回该切片；否则创建一个新的切片并返回。
			*  
			*  获取流程：
			*  1. 检查free_fragments_列表是否为空
			*  2. 如果非空，从free_fragments_列表中取出最后一个切片并返回
			*  3. 如果为空，创建一个新的Fragment实例并返回
			*  
			*  空闲切片复用示意图（Idle Fragment Reuse Illustration）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  free_fragments_ = [IdleFrag0, IdleFrag1]                     |
			*   |  GetIdleFragment() -> returns IdleFrag1                       |
			*   |  free_fragments_ = [IdleFrag0]                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  If free_fragments_ is empty:                                 |
			*   |  GetIdleFragment() -> creates new Fragment                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @return 返回指向空闲切片的共享指针。如果空闲列表为空，返回新创建的切片
			*  @note 该方法线程安全，使用互斥锁保护
			*  @note 返回的切片已经被重置（Reset()），可以直接使用
			*  @note 空闲切片复用可以减少内存分配和释放的开销
			*  
			*  使用示例：
			*  @code
			*  auto fragment = window.GetIdleFragment();
			*  fragment->SetSequenceNo(seq_no);
			*  fragment->Write(data, size);
			*  @endcode
			*/
			std::shared_ptr<Fragment> GetIdleFragment();

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 根据文件名获取切片（Get Fragment By Name）
			*  
			*  该方法用于根据文件名在切片窗口中查找对应的切片。通常用于客户端
			*  请求特定切片时定位切片数据。
			*  
			*  查找流程：
			*  1. 遍历fragments_列表
			*  2. 比较每个切片的文件名与给定文件名
			*  3. 如果找到匹配的切片，返回该切片的引用
			*  4. 如果未找到，返回空指针或抛出异常
			*  
			*  @param name 切片的文件名，例如 "stream_0001.ts"
			*  @return 返回指向匹配切片的共享指针常量引用。如果未找到，行为取决于实现
			*  @note 该方法线程安全，使用互斥锁保护
			*  @note 文件名应该与切片窗口中的文件名完全匹配
			*  
			*  使用示例：
			*  @code
			*  const auto& fragment = window.GetFragmentByName("stream_0001.ts");
			*  if (fragment) {
			*      // 使用fragment数据
			*  }
			*  @endcode
			*/
			const std::shared_ptr<Fragment> & GetFragmentByName(const std::string & name)  ;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取播放列表（Get Play List）
			*  
			*  该方法用于获取M3U8格式的播放列表。播放列表包含切片窗口中所有
			*  切片的元信息，客户端通过播放列表获取切片列表。
			*  
			*  播放列表格式（M3U8 Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  #EXTM3U                                                       |
			*   |  #EXT-X-VERSION:3                                              |
			*   |  #EXT-X-TARGETDURATION:10                                      |
			*   |  #EXT-X-MEDIA-SEQUENCE:0                                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  #EXTINF:10.000,                                               |
			*   |  stream_0000.ts                                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  #EXTINF:10.000,                                               |
			*   |  stream_0001.ts                                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                                                               :
			*   |  ... more fragments ...                                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @return 返回M3U8格式的播放列表字符串
			*  @note 该方法线程安全，使用互斥锁保护
			*  @note 播放列表会根据窗口中的切片自动更新
			*  @note 播放列表格式符合HLS协议规范
			*  
			*  使用示例：
			*  @code
			*  std::string playlist = window.GetPlayList();
			*  // playlist 包含完整的M3U8播放列表
			*  @endcode
			*/
			std::string GetPlayList()  ;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 更新播放列表（Update Play List）
			*  
			*  该方法用于根据当前切片窗口中的切片重新生成M3U8播放列表。
			*  通常在添加或移除切片后调用，确保播放列表与窗口内容同步。
			*  
			*  更新流程：
			*  1. 清空当前的playlist_字符串
			*  2. 添加M3U8头部标签（#EXTM3U, #EXT-X-VERSION等）
			*  3. 遍历fragments_列表，为每个切片添加播放列表条目
			*  4. 每个条目包含#EXTINF标签（时长）和文件名
			*  5. 添加#EXT-X-MEDIA-SEQUENCE标签（序列号）
			*  
			*  播放列表生成示意图（Playlist Generation Illustration）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  fragments_ = [Frag0, Frag1, Frag2, Frag3, Frag4]            |
			*   |  UpdatePlayList() generates:                                  |
			*   |  #EXTM3U                                                      |
			*   |  #EXT-X-VERSION:3                                             |
			*   |  #EXT-X-TARGETDURATION:10                                     |
			*   |  #EXT-X-MEDIA-SEQUENCE:0                                      |
			*   |  #EXTINF:10.000,                                              |
			*   |  stream_0000.ts                                               |
			*   |  #EXTINF:10.000,                                              |
			*   |  stream_0001.ts                                               |
			*   |  ...                                                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 该方法线程安全，使用互斥锁保护
			*  @note 更新后，GetPlayList()将返回最新的播放列表
			*  @note 播放列表格式符合HLS协议规范
			*  
			*  使用示例：
			*  @code
			*  window.AppendFragment(std::move(fragment));
			*  window.UpdatePlayList();  // 更新播放列表
			*  @endcode
			*/
			void UpdatePlayList();
		private:

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 切片窗口大小（Window Size）
			*  
			*  该成员变量用于存储切片窗口的大小。窗口大小决定了窗口中可以同时
			*  保留的切片数量。当窗口满时，最老的切片会被移除。
			*  
			*  窗口大小说明：
			*  - 默认值为5，表示窗口最多可以保留5个切片
			*  - 窗口大小通常为3-10，过小可能导致客户端无法获取足够的切片
			*  - 窗口大小过大会占用更多内存
			*  
			*  @note 窗口大小在构造函数中设置，通常不会更改
			*  @note 窗口大小影响播放列表的长度
			*/
			int32_t   window_size_{5};

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 切片列表（Fragment List）
			*  
			*  该成员变量用于存储切片窗口中当前活跃的切片列表。列表按添加顺序
			*  排列，最老的切片在列表开头，最新的切片在列表末尾。
			*  
			*  列表结构（List Structure）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  fragments_[0]  |  Fragment 0 (oldest)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  fragments_[1]  |  Fragment 1                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  ...            |  ...                                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  fragments_[N]  |  Fragment N (newest)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 列表大小不超过window_size_
			*  @note 列表中的切片按添加顺序排列
			*  @note 列表用于生成M3U8播放列表
			*/
			std::vector< std::shared_ptr<Fragment>> fragments_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 空闲切片列表（Free Fragment List）
			*  
			*  该成员变量用于存储空闲的切片列表。当切片从窗口中移除时，会被
			*  重置并移动到空闲列表中以供复用，减少内存分配和释放的开销。
			*  
			*  空闲列表管理（Free List Management）：
			*  1. 当切片从fragments_移除时，调用Reset()重置状态
			*  2. 将重置后的切片添加到free_fragments_列表
			*  3. 当需要新切片时，先从free_fragments_获取
			*  4. 如果free_fragments_为空，创建新切片
			*  
			*  @note 空闲切片已被重置（Reset()），可以直接复用
			*  @note 空闲切片复用可以减少内存分配和释放的开销
			*  @note 空闲列表大小不受window_size_限制
			*/
			std::vector< std::shared_ptr<Fragment>>free_fragments_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief M3U8播放列表（M3U8 Playlist）
			*  
			*  该成员变量用于存储M3U8格式的播放列表字符串。播放列表包含切片
			*  窗口中所有切片的元信息，客户端通过播放列表获取切片列表。
			*  
			*  播放列表内容（Playlist Content）：
			*  - #EXTM3U：M3U8文件头部标签
			*  - #EXT-X-VERSION：协议版本号
			*  - #EXT-X-TARGETDURATION：目标切片时长
			*  - #EXT-X-MEDIA-SEQUENCE：媒体序列号
			*  - #EXTINF：切片时长和文件名条目
			*  
			*  @note 播放列表通过UpdatePlayList()方法更新
			*  @note 播放列表格式符合HLS协议规范
			*  @note 播放列表可以缓存在内存中，提高访问效率
			*/
			std::string playlist_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 互斥锁（Mutex Lock）
			*  
			*  该成员变量用于保护切片窗口的并发访问。所有对窗口的操作都需要
			*  先获取互斥锁，确保线程安全。
			*  
			*  线程安全说明：
			*  - 所有公共方法在操作前都会获取互斥锁
			*  - 确保多线程环境下的数据一致性
			*  - 使用std::lock_guard自动管理锁的获取和释放
			*  
			*  @note 所有公共方法都是线程安全的
			*  @note 使用std::mutex进行同步
			*/
			std::mutex	lock_;
		};
	}
}


#endif // 