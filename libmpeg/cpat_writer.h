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
		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief PAT写入器类（PAT Writer）
		*  
		*  PatWriter类用于写入MPEG-TS流中的PAT（Program Association Table，节目关联表）。
		*  PAT表是MPEG-TS流中最重要的PSI表之一，它列出了传输流中所有节目的映射关系。
		*  
		*  PAT表说明：
		*  - PAT表固定使用PID 0x0000和table_id 0x00
		*  - PAT表用于列出传输流中所有节目的映射关系
		*  - 每个节目通过program_number和program_map_PID进行关联
		*  - program_map_PID用于定位对应的PMT表
		*  
		*  PAT表Section数据结构（PAT Section Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  table_id=0x00 | section_syntax=1 | reserved=0 | section_length|
		*   |  (8 bits)       |  (1 bit)         |  (1 bit)   |  (12 bits)    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  transport_stream_id                                           |
		*   |  (16 bits)                                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  reserved | version | current_next | section_number          |
		*   |  (2 bits) | (5 bits)|  (1 bit)     |  (8 bits)               |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  last_section_number                                           |
		*   |  (8 bits)                                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        Program List                            :
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | program_number (16 bits)                                   | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | reserved(3) | program_map_PID (13 bits)                    | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   :  ... more programs ...                                        :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        CRC32 (32 bits)                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  节目条目格式（Program Entry Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  program_number                                               |
		*   |  (16 bits)                                                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  reserved(3) | program_map_PID                                |
		*   |  (3 bits)    |  (13 bits)                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note PatWriter固定使用PID 0x0000和table_id 0x00
		*  @note PAT表用于节目映射，是定位PMT表的关键
		*  
		*  使用示例：
		*  @code
		*  PatWriter pat_writer;
		*  pat_writer.WritePat(stream_writer);
		*  @endcode
		*/
		class PatWriter : public PSIWriter
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建PatWriter实例。会设置PAT表的固定PID和table_id。
			*  
			*  初始化说明：
			*  - pid_: 设置为0x0000（PAT表固定PID）
			*  - table_id_: 设置为0x00（PAT表固定table_id）
			*  
			*  @note PAT表固定使用PID 0x0000和table_id 0x00
			*/
			PatWriter() 
				: PSIWriter()
			{
				pid_ = 0X0000; //固定 0x0000
				table_id_ = 0X00; // table_id 固定 0x00
			}

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理PatWriter实例。使用默认析构函数，自动释放成员变量。
			*  
			*  @note 使用默认析构函数，自动释放资源
			*/
			/*virtual*/ ~PatWriter() = default;

		public:

		/**
		*  @author chensong
		*  @date 2025-04-09
		*  @brief 写入PAT表（Write PAT Table）
		*  
		*  PAT表是MPEG-TS流中最重要的PSI表之一，它列出了传输流中所有节目的映射关系。
		*  每个节目通过program_number和program_map_PID进行关联，用于定位对应的PMT表。
		*  
		*  PAT表数据格式（PAT Table Data Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  transport_stream_id                                           |
		*   |  (16 bits)                                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  program_number                                               |
		*   |  (16 bits)                                                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  reserved(3) | program_map_PID                                |
		*   |  (3 bits=111)|  (13 bits)                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  写入的数据格式：
		*  - transport_stream_id_ (16位，大端序)
		*  - program_number_ (16位，大端序)
		*  - reserved (3位=111) + program_map_PID (13位)，共16位，格式为 0xE000 | pmt_pid_（大端序）
		*  
		*  @param w StreamWriter指针，用于写入TS包数据
		*  @note 该方法会将PAT表数据封装为PSI Section，并调用基类WriteSection方法
		*        写入到TS流中。PAT表固定使用PID 0x0000和table_id 0x00。
		*  @note program_number为0x0000时，program_map_PID表示network_PID
		*  
		*  使用示例：
		*  @code
		*  PatWriter pat_writer;
		*  pat_writer.WritePat(stream_writer);
		*  @endcode
		*/
		void WritePat(StreamWriter * w);


		private:

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 节目号（Program Number）
			*  
			*  该成员变量用于存储节目的编号。节目号用于标识传输流中的不同节目。
			*  如果program_number为0x0000，则表示这是一个网络PID条目。
			*  
			*  节目号格式（Program Number Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  program_number                                               |
			*   |  (16 bits)                                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  0x0000: 网络PID条目                                          |
			*   |  其他值: 节目号                                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0x0001，表示节目号为1
			*  @note program_number为0x0000时，表示network_PID条目
			*/
			uint16_t  program_number_{0X0001};

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief PMT表PID（Program Map Table PID）
			*  
			*  该成员变量用于存储PMT表所在TS包的PID。program_map_PID用于定位
			*  对应节目的PMT表，PMT表包含该节目的所有原始流（音频、视频等）信息。
			*  
			*  PMT PID格式（PMT PID Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved(3) | program_map_PID                                |
			*   |  (3 bits=111)|  (13 bits)                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  格式: 0xE000 | pmt_pid_                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0x1001，表示PMT表的PID为0x1001
			*  @note program_map_PID用于定位对应节目的PMT表
			*  @note 如果program_number为0x0000，则program_map_PID表示network_PID
			*/
			uint16_t  pmt_pid_{ 0X1001 }; // program_map_PID 

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 传输流ID（Transport Stream ID）
			*  
			*  该成员变量用于存储传输流的ID。transport_stream_id用于标识传输流，
			*  在一个网络中可以唯一标识一个传输流。
			*  
			*  传输流ID格式（Transport Stream ID Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  transport_stream_id                                           |
			*   |  (16 bits)                                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  用于唯一标识传输流                                            |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0x0001，表示传输流ID为1
			*  @note transport_stream_id在table_id_extension字段中使用
			*/
			uint16_t  transport_stream_id_{ 0X0001 };
		};
	}
}


#endif // 