
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

purpose:		psi writer 

PSI

作用是从一个携带多个节目的TS流中正确找到特定的节目

PSI包含： PAT， PMT， NIT， CAT



PAT：节目关联表

PMT：节目映射表

NIT：网络信息表

CAT：条件存储表

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

#ifndef _C_PSI_WRITER_
#define _C_PSI_WRITER_


#include <cstdint>
#include <memory>
 
#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>
 
 
#include <unordered_map>
 
#include <functional>
#include <memory>
#include "libmedia_transfer_protocol/libmpeg/cstream_writer.h"
namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{
		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief PSI写入器基类（PSI Writer Base Class）
		*  
		*  PSIWriter是MPEG-TS流中PSI（Program Specific Information）表的写入基类。
		*  PSI表用于从携带多个节目的TS流中正确找到特定的节目，包括PAT、PMT、NIT、CAT等。
		*  
		*  PSI表说明：
		*  - PAT（Program Association Table）：节目关联表，列出传输流中所有节目的映射关系
		*  - PMT（Program Map Table）：节目映射表，列出节目的所有原始流（音频、视频等）
		*  - NIT（Network Information Table）：网络信息表，包含网络相关信息
		*  - CAT（Conditional Access Table）：条件访问表，包含条件访问相关信息
		*  
		*  PSI Section 数据结构（PSI Section Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  table_id      |  section_syntax_indicator | reserved       |L|
		*   |  (8 bits)      |  (1 bit)                  |  (1 bit)       |E|
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  section_length                  |  table_id_extension       |
		*   |  (12 bits)                       |  (16 bits)                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  reserved | version | current_next | section_number          |
		*   |  (2 bits) | (5 bits)|  (1 bit)     |  (8 bits)               |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  last_section_number                                           |
		*   |  (8 bits)                                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        Section Data                            :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        CRC32 (32 bits)                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  TS Packet 数据结构（TS Packet Data Structure）：
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
		*  @note PSIWriter是所有PSI表写入器的基类，提供通用的PSI Section封装和TS包分片功能
		*  @note PSI Section会被分片到多个188字节的TS包中发送
		*  @note 第一个TS包的payload_unit_start指示符设置为1，表示包含Section的开始
		*  
		*  使用示例：
		*  @code
		*  // PSIWriter是基类，不能直接实例化，通过子类使用
		*  PatWriter pat_writer;
		*  pat_writer.WritePat(stream_writer);
		*  @endcode
		*/
		class PSIWriter
		{
		public:
			PSIWriter() = default;
			~PSIWriter() = default;

		public:
			 
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 推送PSI Section到TS包（Push PSI Section to TS Packets）
			*  
			*  该方法用于将PSI Section数据分片到多个TS包中并发送出去。
			*  由于TS包固定大小为188字节，而PSI Section可能超过184字节（188-4）的负载大小，
			*  因此需要将PSI Section分片到多个TS包中。
			*  
			*  TS Packet 构建格式（TS Packet Construction Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  0x47 (sync_byte)                                             |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PID[13:8] | TEI|PUSI|TP | PID[7:0]                          |
			*   |             |    |    |   |                                   |
			*   |  TEI=0      | PUSI: 1 for first packet, 0 for others         |
			*   |  PUSI=1/0   | TP=1 (no payload only)                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  TSC | AFC | CC (4 bits)                                       |
			*   |  00  | 01  | continuity_counter                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PSI Section Data (up to 184 bytes)                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Padding (0xFF, if payload < 184 bytes)                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param w StreamWriter指针，用于写入TS包数据，不能为空
			*  @param buf PSI Section数据的指针，包含完整的PSI Section
			*  @param len PSI Section数据的大小，单位为字节
			*  @note 该方法会自动将PSI Section分片到多个188字节的TS包中
			*  @note 第一个TS包的payload_unit_start指示符设置为1
			*  @note 连续性计数器（continuity_counter）会自动递增
			*  @note 如果负载不足184字节，会用0xFF填充
			*/
			void PushSection(StreamWriter*w, uint8_t * buf, size_t len);
			
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 写入PSI Section（Write PSI Section）
			*  
			*  该方法用于组装并写入PSI Section。它会构建完整的PSI Section结构，
			*  包括头部、数据部分和CRC32校验和，然后调用PushSection分片到TS包。
			*  
			*  PSI Section 完整结构（PSI Section Complete Structure）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  table_id          | section_syntax | reserved | section_len  |
			*   |  (8 bits)          | (1 bit)        | (1 bit)  | (12 bits)    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  table_id_extension                                           |
			*   |  (16 bits)                                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved | version | current_next | section_number          |
			*   |  (2 bits) | (5 bits)|  (1 bit)     |  (8 bits)               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  last_section_number                                           |
			*   |  (8 bits)                                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                        Section Data (variable)                 :
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |                        CRC32 (32 bits)                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param w StreamWriter指针，用于写入TS包数据，不能为空
			*  @param id table_id_extension值，用于标识PSI表的特定实例（如传输流ID）
			*  @param sec_num section_number值，当前段的编号（从0开始）
			*  @param last_sec_num last_section_number值，最后一个段的编号
			*  @param buf 指向PSI Section数据部分的指针，包含PSI表的具体数据
			*  @param len PSI Section数据部分的大小，单位为字节
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法会自动计算CRC32校验和并添加到Section末尾
			*  @note PSI Section总长度 = 3 (固定头) + 5 (扩展头) + len (数据) + 4 (CRC32)
			*  @note 完成后会自动调用PushSection将Section分片到TS包
			*/
			int32_t WriteSection(StreamWriter* w, int32_t id, int32_t sec_num, int32_t last_sec_num, uint8_t * buf, int32_t len);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置PSI版本号（Set PSI Version）
			*  
			*  该方法用于设置PSI表的版本号。版本号用于标识PSI表内容的版本，
			*  当PSI表内容发生变化时，版本号会递增，通知接收端更新。
			*  
			*  版本字段格式（Version Field Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved | version | current_next | section_number          |
			*   |  (2 bits) | (5 bits)|  (1 bit)     |  (8 bits)               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |            version_: 5位版本号 (0-31)                          |
			*   |            current_next: 1表示当前表有效                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param v 版本号值，范围0-31（5位）
			*  @note 版本号存储在version_成员变量中，会在WriteSection时使用
			*  @note 当PSI表内容变化时，应该递增版本号
			*/
			void SetVersion(uint8_t v);

		protected:

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 连续性计数器（Continuity Counter）
			*  
			*  该成员变量用于存储TS包的连续性计数器。连续性计数器用于检测TS包丢失，
			*  每个具有相同PID的TS包都会有连续性计数器，按顺序从0到15循环。
			*  
			*  连续性计数器格式（Continuity Counter Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  TSC | AFC | CC (4 bits)                                       |
			*   |  00  | 01  | continuity_counter (0-15)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为-1，表示尚未初始化。第一次使用时会被设置为0
			*  @note 每个TS包发送后，连续性计数器会自动递增
			*/
			int8_t   cc_{ -1 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 包标识符（Packet Identifier）
			*  
			*  该成员变量用于存储TS包的PID（Packet Identifier）。PID用于标识
			*  TS包所属的流或表，每个PSI表都有固定的PID。
			*  
			*  PID格式（PID Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PID[13:8] | TEI|PUSI|TP | PID[7:0]                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |            PID: 13位包标识符 (0x0000-0x1FFF)                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0xe000（保留值），子类需要在构造函数中设置正确的PID
			*  @note PAT表使用PID 0x0000，PMT表的PID由PAT表指定
			*/
			uint16_t  pid_{ 0xe000 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 表标识符（Table Identifier）
			*  
			*  该成员变量用于存储PSI表的table_id。table_id用于标识PSI表的类型，
			*  不同类型的PSI表有不同的table_id值。
			*  
			*  table_id格式（Table ID Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  table_id          | section_syntax | reserved | section_len  |
			*   |  (8 bits)          | (1 bit)        | (1 bit)  | (12 bits)    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |            table_id_: 8位表标识符                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0x00，子类需要在构造函数中设置正确的table_id
			*  @note PAT表使用table_id 0x00，PMT表使用table_id 0x02
			*/
			uint8_t   table_id_{ 0X00 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief PSI版本号（PSI Version）
			*  
			*  该成员变量用于存储PSI表的版本号。版本号用于标识PSI表内容的版本，
			*  当PSI表内容发生变化时，版本号会递增。
			*  
			*  版本字段格式（Version Field Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  reserved | version | current_next | section_number          |
			*   |  (2 bits) | (5 bits)|  (1 bit)     |  (8 bits)               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |            version_: 5位版本号 (0-31)                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0x00，表示版本号为0
			*  @note 版本号可以通过SetVersion()方法设置
			*/
			int8_t     version_{ 0X00 };
		};
	}
}

#endif // _C_PSI_WRITER_