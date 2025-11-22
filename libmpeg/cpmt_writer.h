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

purpose:		pmt writer


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

#ifndef _C_PMT_WRITER_
#define _C_PMT_WRITER_


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
		*  @brief 节目信息结构体（Program Information Structure）
		*  
		*  ProgramInfo结构体用于存储节目的原始流信息，包括流类型和PID。
		*  每个原始流（音频、视频等）都有一个ProgramInfo条目。
		*  
		*  节目信息数据结构（Program Info Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  stream_type      |  reserved(3) | elementary_PID             |
		*   |  (8 bits)         |  (3 bits)    |  (13 bits)                  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  ES_info_length (12 bits)                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        ES_info (variable)                      :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  流类型说明（Stream Type Description）：
		*  - 0x01: MPEG-1 Video
		*  - 0x02: MPEG-2 Video
		*  - 0x03: MPEG-1 Audio
		*  - 0x04: MPEG-2 Audio
		*  - 0x0F: AAC Audio
		*  - 0x1B: H.264 Video (AVC)
		*  - 0x24: H.265 Video (HEVC)
		*  
		*  @note 流类型标识了原始流的编码格式
		*  @note elementary_PID用于标识原始流的TS包PID
		*/
		struct ProgramInfo
		{
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 流类型（Stream Type）
			*  
			*  该成员变量用于存储原始流的类型。流类型标识了原始流的编码格式，
			*  如H264视频、AAC音频等。
			*  
			*  @note 8位值，范围0x00-0xFF
			*  @note 常用值：0x1B (H.264), 0x0F (AAC), 0x24 (H.265)
			*/
			uint8_t   stream_type; // 8bits

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 原始流PID（Elementary Stream PID）
			*  
			*  该成员变量用于存储原始流的PID。elementary_PID用于标识
			*  该原始流的TS包PID，接收端通过此PID过滤对应的TS包。
			*  
			*  @note 13位值，范围0x0000-0x1FFF
			*  @note 用于标识原始流的TS包PID
			*/
			uint16_t   elementary_pid; // 13bits;
		};

		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief 节目信息指针类型定义（Program Info Pointer Type Definition）
		*  
		该类型定义用于简化ProgramInfo的共享指针使用。
		*/
		using ProgramInfoPtr = std::shared_ptr< ProgramInfo>;

		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief PMT写入器类（PMT Writer）
		*  
		*  PmtWriter类用于写入MPEG-TS流中的PMT（Program Map Table，节目映射表）。
		*  PMT表列出了节目的所有原始流（音频、视频等）信息，包括流类型和PID。
		*  
		*  PMT表说明：
		*  - PMT表固定使用table_id 0x02
		*  - PMT表的PID由PAT表指定（通常在构造函数中设置为0x1001）
		*  - PMT表用于列出节目的所有原始流信息
		*  - 每个原始流（音频、视频等）都有一个ProgramInfo条目
		*  
		*  PMT表Section数据结构（PMT Section Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  table_id=0x02 | section_syntax=1 | reserved=0 | section_length|
		*   |  (8 bits)       |  (1 bit)         |  (1 bit)   |  (12 bits)    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  program_number                                               |
		*   |  (16 bits)                                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  reserved | version | current_next | section_number          |
		*   |  (2 bits) | (5 bits)|  (1 bit)     |  (8 bits)               |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  last_section_number                                           |
		*   |  (8 bits)                                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  reserved(3) | PCR_PID (13 bits)                              |
		*   |  (3 bits=111)|  (13 bits)                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  reserved(4) | program_info_length (12 bits)                  |
		*   |  (4 bits=1111)|  (12 bits)                                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        Program Info Descriptors (variable)    :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        Elementary Stream List                 :
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | stream_type (8 bits)                                       | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | reserved(3) | elementary_PID (13 bits)                     | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  | reserved(4) | ES_info_length (12 bits)                     | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  :  ES_info descriptors (variable)                            : |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   :  ... more elementary streams ...                              :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        CRC32 (32 bits)                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  原始流条目格式（Elementary Stream Entry Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  stream_type      |  reserved(3) | elementary_PID             |
		*   |  (8 bits)         |  (3 bits=111)|  (13 bits)                  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  reserved(4) | ES_info_length (12 bits)                       |
		*   |  (4 bits=1111)|  (12 bits)                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        ES_info descriptors (variable)         :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note PmtWriter固定使用table_id 0x02，PID由PAT表指定（默认0x1001）
		*  @note PMT表用于列出节目的所有原始流（音频、视频等）信息
		*  @note 每个原始流都有一个ProgramInfo条目，包含流类型和PID
		*  
		*  使用示例：
		*  @code
		*  PmtWriter pmt_writer;
		*  auto video_info = std::make_shared<ProgramInfo>();
		*  video_info->stream_type = 0x1B;  // H.264
		*  video_info->elementary_pid = 0x1011;
		*  pmt_writer.AddProgramInfo(video_info);
		*  pmt_writer.WritePmt(stream_writer);
		*  @endcode
		*/
		class PmtWriter : public PSIWriter
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建PmtWriter实例。会设置PMT表的固定table_id和默认PID。
			*  
			*  初始化说明：
			*  - table_id_: 设置为0x02（PMT表固定table_id）
			*  - pid_: 设置为0x1001（默认PMT表PID，可由PAT表指定）
			*  
			*  @note PMT表固定使用table_id 0x02
			*  @note PMT表的PID由PAT表指定，默认值为0x1001
			*/
			PmtWriter()
				: PSIWriter()
			{
				table_id_ = 0X02;
				pid_ = 0X1001;
			}

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理PmtWriter实例。使用默认析构函数，自动释放成员变量。
			*  
			*  @note 使用默认析构函数，智能指针自动管理ProgramInfo对象
			*/
			~PmtWriter() = default;
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 写入PMT表（Write PMT Table）
			*  
			*  该方法用于写入PMT（Program Map Table，节目映射表）。PMT表列出了
			*  节目的所有原始流（音频、视频等）信息，包括流类型和PID。
			*  
			*  PMT表数据格式（PMT Table Data Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  program_number                                               |
			*   |  (16 bits)                                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved(3) | PCR_PID (13 bits)                              |
			*   |  (3 bits=111)|  (13 bits)                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved(4) | program_info_length (12 bits)                  |
			*   |  (4 bits=1111)|  (12 bits)                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                        Program Info Descriptors (variable)    :
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                        Elementary Stream List                 :
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  | stream_type (8 bits)                                       | |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  | reserved(3) | elementary_PID (13 bits)                     | |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  | reserved(4) | ES_info_length (12 bits)                     | |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  :  ES_info descriptors (variable)                            : |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   :  ... more elementary streams ...                              :
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  写入流程：
			*  1. 构建PMT表数据，包括program_number、PCR_PID
			*  2. 遍历programs_列表，为每个原始流添加条目
			*  3. 每个条目包含stream_type和elementary_PID
			*  4. 调用基类WriteSection方法封装为PSI Section
			*  5. 分片到TS包并发送
			*  
			*  @param w StreamWriter指针，用于写入TS包数据，不能为空
			*  @note 该方法会将PMT表数据封装为PSI Section，并调用基类WriteSection方法
			*        写入到TS流中。PMT表固定使用table_id 0x02。
			*  @note PMT表包含节目的所有原始流信息（音频、视频等）
			*  
			*  使用示例：
			*  @code
			*  PmtWriter pmt_writer;
			*  // ... 添加原始流信息 ...
			*  pmt_writer.WritePmt(stream_writer);
			*  @endcode
			*/
			void WritePmt(StreamWriter *w);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 添加原始流信息（Add Program Info）
			*  
			*  该方法用于向PMT表添加一个原始流的信息。每个原始流（音频、视频等）
			*  都需要一个ProgramInfo条目，包含流类型和PID。
			*  
			*  原始流信息格式（Elementary Stream Info Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  stream_type      |  reserved(3) | elementary_PID             |
			*   |  (8 bits)         |  (3 bits=111)|  (13 bits)                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved(4) | ES_info_length (12 bits)                       |
			*   |  (4 bits=1111)|  (12 bits)                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                        ES_info descriptors (variable)         :
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param program 指向ProgramInfo对象的共享指针，包含原始流的流类型和PID
			*  @note 每个原始流（音频、视频等）都需要添加一个ProgramInfo条目
			*  @note ProgramInfo包含stream_type（流类型）和elementary_pid（原始流PID）
			*  
			*  使用示例：
			*  @code
			*  auto video_info = std::make_shared<ProgramInfo>();
			*  video_info->stream_type = 0x1B;  // H.264
			*  video_info->elementary_pid = 0x1011;
			*  pmt_writer.AddProgramInfo(video_info);
			*  @endcode
			*/
			void AddProgramInfo(ProgramInfoPtr & program);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置PCR PID（Set PCR PID）
			*  
			*  该方法用于设置PCR（Program Clock Reference）PID。PCR用于同步
			*  接收端的时钟，通常与视频流的PID相同。
			*  
			*  PCR PID格式（PCR PID Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved(3) | PCR_PID (13 bits)                              |
			*   |  (3 bits=111)|  (13 bits)                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  格式: 0xE000 | pcr_id_                                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  PCR说明：
			*  - PCR（Program Clock Reference）用于同步接收端的时钟
			*  - PCR通常与视频流的PID相同
			*  - PCR在TS包的adaptation_field中传输
			*  
			*  @param pid PCR PID值，范围0x0000-0x1FFF（13位）
			*  @note PCR PID用于标识包含PCR字段的TS包
			*  @note PCR通常与视频流的PID相同
			*  
			*  使用示例：
			*  @code
			*  pmt_writer.SetPcrPid(0x1011);  // 设置PCR PID为视频流PID
			*  @endcode
			*/
			void SetPcrPid(int32_t pid);
		private:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief PCR PID（PCR Packet Identifier）
			*  
			*  该成员变量用于存储PCR（Program Clock Reference）PID。PCR用于同步
			*  接收端的时钟，通常与视频流的PID相同。
			*  
			*  PCR PID格式（PCR PID Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved(3) | PCR_PID (13 bits)                              |
			*   |  (3 bits=111)|  (13 bits)                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  写入格式: 0xE000 | pcr_id_                                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0xE000（保留值，实际PID的高3位）
			*  @note PCR PID用于标识包含PCR字段的TS包
			*  @note PCR通常与视频流的PID相同
			*/
			uint16_t    pcr_id_{ 0XE000 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 原始流信息列表（Elementary Stream Info List）
			*  
			*  该成员变量用于存储节目的所有原始流信息列表。每个原始流（音频、视频等）
			*  都有一个ProgramInfo条目，包含流类型和PID。
			*  
			*  原始流列表结构（Elementary Stream List Structure）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  programs_[0]  |  Stream 0 (stream_type, elementary_pid)     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  programs_[1]  |  Stream 1 (stream_type, elementary_pid)     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :  ...            |  ...                                         :
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  programs_[N]  |  Stream N (stream_type, elementary_pid)     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 列表包含节目的所有原始流信息（音频、视频等）
			*  @note 每个ProgramInfo包含stream_type（流类型）和elementary_pid（原始流PID）
			*  @note 列表通过AddProgramInfo()方法添加原始流信息
			*/
			std::vector< ProgramInfoPtr> programs_;  //多少路原始流

		};
	}
}

#endif // 