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

purpose:		pat writer


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

#ifndef _C_PAT_WRITER_
#define _C_PAT_WRITER_


#include <cstdint>
#include <memory>

#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>
 
 
#include <unordered_map>
 
#include <functional>
#include <memory>
#include "libmedia_transfer_protocol/libmpeg/cpsi_writer.h"

namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{
		class PatWriter : public PSIWriter
		{
		public:
			PatWriter() 
				: PSIWriter()
			{
				pid_ = 0X0000; //固定 0x0000
				table_id_ = 0X00; // table_id 固定 0x00
			}
			/*virtual*/ ~PatWriter() = default;

		public:

		/**
		*  @author chensong
		*  @date 2025-04-09
		*  @brief 写入PAT表（Program Association Table，节目关联表）
		*  
		*  PAT表是MPEG-TS流中最重要的PSI表之一，它列出了传输流中所有节目的映射关系。
		*  每个节目通过program_number和program_map_PID进行关联，用于定位对应的PMT表。
		*  
		*  PAT表Section数据结构：
		*  - transport_stream_id (16位): 传输流ID，用于标识传输流
		*  - section_length (12位): 段长度
		*  - program_list:
		*      - program_number (16位): 节目号，0x0000表示网络PID，其他值表示节目号
		*      - reserved (3位): 保留位，固定为111
		*      - program_map_PID (13位): PMT表所在TS包的PID，当program_number=0时表示network_PID
		*  
		*  写入的数据格式：
		*  - program_number_ (16位，大端序)
		*  - reserved (3位) + program_map_PID (13位)，共16位，格式为 0xE000 | pmt_pid_（大端序）
		*  
		*  @param w StreamWriter指针，用于写入TS包数据
		*  @note 该方法会将PAT表数据封装为PSI Section，并调用基类WriteSection方法
		*        写入到TS流中。PAT表固定使用PID 0x0000和table_id 0x00。
		*  
		*  使用示例：
		*  @code
		*  PatWriter pat_writer;
		*  pat_writer.WritePat(stream_writer);
		*  @endcode
		*/
		void WritePat(StreamWriter * w);


		private:



			uint16_t  program_number_{0X0001};
			uint16_t  pmt_pid_{ 0X1001 }; // program_map_PID 
			uint16_t  transport_stream_id_{ 0X0001 };
		};
	}
}


#endif // 