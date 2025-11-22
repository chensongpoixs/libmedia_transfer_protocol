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

purpose:		  _C_FRAGMENT_


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

#ifndef _C_FRAGMENT_
#define _C_FRAGMENT_


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
		class Packet;
	}
	namespace libhls
	{
		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief HLS 切片步长大小（HLS Fragment Step Size）
		*  
		*  该常量定义了HLS切片的默认步长大小，用于控制切片的增长步进。
		*  当切片缓冲区需要扩容时，会以此大小为增量进行扩展。
		*  
		*  @note 默认值为 128KB，这是一个经验值，平衡了内存使用和性能
		*/
		const int32_t kFragmentStepSize = 128 * 1024;

		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief HLS 切片类（HLS Fragment）
		*  
		*  Fragment类表示HLS协议中的一个媒体切片（segment），通常对应一个TS文件。
		*  它继承自StreamWriter，负责管理切片的写入、存储和元数据。
		*  
		*  HLS切片说明：
		*  - HLS（HTTP Live Streaming）是Apple开发的流媒体传输协议
		*  - 媒体流被分割成多个小切片文件（通常是TS格式）
		*  - 每个切片包含一段连续的媒体数据，具有一定的时长
		*  - 客户端通过播放列表（M3U8）获取切片列表并顺序播放
		*  
		*  切片数据结构（Fragment Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                    Fragment Header                              |
		*   |                    (Sequence Number, Duration)                  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                    TS Packet Data                               |
		*   |                    (188 bytes per packet)                       |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                    TS Packet Data                               |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                                                               :
		*   |                    ... more TS packets ...                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  切片元数据（Fragment Metadata）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |        Sequence No (32 bits)      |      Duration (64 bits)    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                         Start DTS (64 bits)                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                         File Name (variable)                    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                         Data Size (32 bits)                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note Fragment负责管理单个切片的生命周期，包括数据写入、时长统计、文件命名等
		*  @note 切片数据以TS包（Transport Stream Packet）形式存储，每个TS包188字节
		*  @note 切片大小动态增长，初始缓冲区大小为512KB，可以按步长扩展
		*  
		*  使用示例：
		*  @code
		*  auto fragment = std::make_shared<Fragment>();
		*  fragment->SetSequenceNo(1);
		*  fragment->Write(data, size);
		*  int64_t duration = fragment->Duration();
		*  @endcode
		*/
		class Fragment :public libmedia_transfer_protocol::libmpeg::StreamWriter
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建Fragment实例。使用默认构造函数，所有成员变量使用默认值。
			*  
			*  初始化说明：
			*  - duration_: 初始化为0，表示切片时长为0
			*  - filename_: 初始化为空字符串
			*  - start_dts_: 初始化为-1，表示尚未设置起始时间戳
			*  - data_: 初始化为空指针，表示尚未分配数据缓冲区
			*  - buf_size_: 初始化为512KB，表示初始缓冲区大小
			*  - data_size_: 初始化为0，表示当前数据大小为0
			*  - sequence_no_: 初始化为0，表示序列号为0
			*  
			*  @note 使用默认构造函数，所有成员变量使用默认初始化值
			*/
			Fragment() = default;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理Fragment实例。使用默认析构函数，自动释放成员变量。
			*  
			*  @note 使用默认析构函数，智能指针自动管理内存
			*/
			~Fragment() = default;

		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 添加时间戳（Append Timestamp）
			*  
			*  该方法用于向切片添加时间戳信息。当写入新的媒体数据时，
			*  会记录该数据的时间戳，用于计算切片的总时长。
			*  
			*  时间戳处理流程：
			*  1. 如果是第一个时间戳（start_dts_为-1），设置start_dts_为pts
			*  2. 计算切片当前时长：duration_ = pts - start_dts_
			*  3. 更新duration_以反映切片的总时长
			*  
			*  时间戳格式：
			*  - pts（Presentation Time Stamp）：表示时间戳，单位为毫秒或90KHz时钟
			*  - start_dts_：切片起始时间戳，用于计算相对时长
			*  - duration_：切片总时长，等于最后一个pts减去start_dts_
			*  
			*  @param pts 表示时间戳（Presentation Time Stamp），单位为毫秒或90KHz时钟
			*  @note 该方法在写入媒体数据时自动调用
			*  @note 时间戳用于计算切片的时长，影响M3U8播放列表中的DURATION字段
			*  
			*  使用示例：
			*  @code
			*  fragment->AppendTimestamp(90000);  // 1秒（90KHz时钟）
			*  @endcode
			*/
			void AppendTimestamp(int64_t pts)override;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 写入数据（Write Data）
			*  
			*  该方法用于向切片缓冲区写入数据。数据通常是TS包数据，
			*  每个TS包188字节。
			*  
			*  写入流程：
			*  1. 检查缓冲区是否有足够空间
			*  2. 如果空间不足，扩展缓冲区（按kFragmentStepSize步长）
			*  3. 将数据复制到缓冲区
			*  4. 更新data_size_以反映当前数据大小
			*  
			*  数据格式（TS Packet Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Sync Byte | Transport Error | Payload Unit Start | Priority  |
			*   |  (0x47)    |   Indicator     |   Indicator         |          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PID (13 bits)         | Transport Scrambling | Adaptation   |
			*   |                        |   Control            |   Field Ctrl |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Continuity Counter    |       Adaptation Field (optional)    |
			*   |  (4 bits)              |                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |                        Payload Data (variable)                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param buf 指向要写入数据的缓冲区指针，不能为空
			*  @param size 要写入的数据大小，单位为字节
			*  @return 返回实际写入的数据大小，单位为字节。如果发生错误，可能小于size
			*  @note 该方法会自动扩展缓冲区，确保有足够空间写入数据
			*  @note 数据以TS包形式存储，通常每个TS包188字节
			*  
			*  使用示例：
			*  @code
			*  uint8_t ts_packet[188] = {...};
			*  int32_t written = fragment->Write(ts_packet, 188);
			*  @endcode
			*/
			int32_t Write(void * buf, size_t size)  override;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取数据指针（Get Data Pointer）
			*  
			*  该方法用于获取切片数据的指针。返回的数据指针指向切片的缓冲区，
			*  可以直接用于读取或传输数据。
			*  
			*  @return 返回指向切片数据缓冲区的字符指针。如果数据为空，可能返回nullptr
			*  @note 返回的指针指向切片内部的缓冲区，不应修改数据
			*  @note 数据大小可以通过Size()方法获取
			*  
			*  使用示例：
			*  @code
			*  char* data = fragment->Data();
			*  int32_t size = fragment->Size();
			*  // 使用data和size进行传输或处理
			*  @endcode
			*/
			char * Data()  override;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取数据大小（Get Data Size）
			*  
			*  该方法用于获取切片数据的实际大小。返回的大小表示当前已写入的数据量，
			*  不一定等于缓冲区的大小。
			*  
			*  @return 返回切片数据的实际大小，单位为字节
			*  @note 返回的大小是已写入的数据大小，而不是缓冲区大小
			*  @note 数据大小随着Write()方法的调用而增长
			*  
			*  使用示例：
			*  @code
			*  int32_t size = fragment->Size();
			*  // 使用size进行数据传输或处理
			*  @endcode
			*/
			int32_t Size() override;
		public:

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取切片时长（Get Duration）
			*  
			*  该方法用于获取切片的时长。时长是通过最后一个时间戳减去起始时间戳计算的，
			*  表示切片包含的媒体数据的播放时长。
			*  
			*  时长计算：
			*  - duration_ = last_pts - start_dts_
			*  - 如果start_dts_为-1，表示尚未设置起始时间戳，返回0
			*  - 时长单位与时间戳单位相同（通常是毫秒或90KHz时钟）
			*  
			*  @return 返回切片的时长，单位为毫秒或90KHz时钟。如果尚未设置时间戳，返回0
			*  @note 时长用于M3U8播放列表中的DURATION字段
			*  @note 时长随着AppendTimestamp()的调用而更新
			*  
			*  使用示例：
			*  @code
			*  int64_t duration = fragment->Duration();
			*  // duration 表示切片的播放时长
			*  @endcode
			*/
			int64_t  Duration() const;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取文件名（Get File Name）
			*  
			*  该方法用于获取切片的文件名。文件名通常由基础文件名和序列号组成，
			*  格式为 "stream_0001.ts" 或类似的格式。
			*  
			*  文件名格式：
			*  - 基础文件名 + "_" + 序列号（格式化） + ".ts"
			*  - 例如：如果基础文件名为 "stream"，序列号为 1，则文件名为 "stream_0001.ts"
			*  
			*  @return 返回切片的文件名常量引用。如果尚未设置基础文件名，可能返回空字符串
			*  @note 文件名用于M3U8播放列表中的URI字段
			*  @note 文件名通常包含序列号，便于客户端顺序请求
			*  
			*  使用示例：
			*  @code
			*  const std::string& filename = fragment->FileName();
			*  // filename 例如 "stream_0001.ts"
			*  @endcode
			*/
			const std::string &FileName() const;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置基础文件名（Set Base File Name）
			*  
			*  该方法用于设置切片的基础文件名。基础文件名是切片文件名的前缀部分，
			*  实际文件名会在此基础上添加序列号和扩展名。
			*  
			*  文件名生成：
			*  1. 使用基础文件名作为前缀
			*  2. 添加序列号（通常格式化为4位数字）
			*  3. 添加扩展名 ".ts"
			*  4. 最终文件名格式："{base_name}_{sequence_no:04d}.ts"
			*  
			*  @param v 基础文件名字符串，例如 "stream" 或 "live"
			*  @note 基础文件名通常与会话名称或流名称相关
			*  @note 设置基础文件名后，会自动生成完整的文件名
			*  
			*  使用示例：
			*  @code
			*  fragment->SetBaseFileName("stream");
			*  fragment->SetSequenceNo(1);
			*  // 生成的文件名为 "stream_0001.ts"
			*  @endcode
			*/
			void SetBaseFileName(const std::string& v);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取序列号（Get Sequence Number）
			*  
			*  该方法用于获取切片的序列号。序列号用于标识切片在播放列表中的顺序，
			*  客户端通过序列号来确保按顺序播放切片。
			*  
			*  序列号说明：
			*  - 序列号从0或1开始，按顺序递增
			*  - 序列号用于生成文件名和M3U8播放列表
			*  - 序列号在切片窗口中应该是唯一的
			*  
			*  @return 返回切片的序列号。如果尚未设置，返回0
			*  @note 序列号用于M3U8播放列表中的序列号字段
			*  @note 序列号通常与文件名中的序号一致
			*  
			*  使用示例：
			*  @code
			*  int32_t seq_no = fragment->SequenceNo();
			*  // seq_no 例如 1, 2, 3, ...
			*  @endcode
			*/
			int32_t SequenceNo() const;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置序列号（Set Sequence Number）
			*  
			*  该方法用于设置切片的序列号。序列号用于标识切片在播放列表中的顺序，
			*  设置序列号后会自动更新文件名。
			*  
			*  @param no 序列号整数，通常从0或1开始，按顺序递增
			*  @note 设置序列号后，文件名会自动更新以包含新的序列号
			*  @note 序列号应该是唯一的，避免重复
			*  
			*  使用示例：
			*  @code
			*  fragment->SetSequenceNo(1);
			*  // 切片序列号设置为1
			*  @endcode
			*/
			void SetSequenceNo(int32_t no);

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 重置切片（Reset Fragment）
			*  
			*  该方法用于重置切片的所有状态，使其可以重新使用。
			*  重置后，切片的所有数据都会被清空，恢复到初始状态。
			*  
			*  重置操作：
			*  1. 清空数据缓冲区（data_）
			*  2. 重置duration_为0
			*  3. 重置start_dts_为-1
			*  4. 重置data_size_为0
			*  5. 清空文件名（filename_）
			*  6. 序列号保持不变（或根据需求重置）
			*  
			*  @note 重置后，切片可以用于存储新的媒体数据
			*  @note 通常用于切片窗口中的空闲切片复用
			*  
			*  使用示例：
			*  @code
			*  fragment->Reset();
			*  // 切片已重置，可以重新使用
			*  @endcode
			*/
			void  Reset();

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取切片数据（Get Fragment Data）
			*  
			*  该方法用于获取切片的底层数据包对象。返回的Packet对象包含切片的完整数据，
			*  可以用于进一步处理或传输。
			*  
			*  @return 返回指向Packet对象的共享指针引用。如果数据为空，可能返回空指针
			*  @note 返回的是内部数据对象的引用，修改会影响切片数据
			*  @note Packet对象包含完整的TS包数据
			*  
			*  使用示例：
			*  @code
			*  auto& packet = fragment->FragmentData();
			*  // 使用packet进行进一步处理
			*  @endcode
			*/
			std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet> & FragmentData();
		private:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 切片时长（Fragment Duration）
			*  
			*  该成员变量用于存储切片的播放时长。时长通过最后一个时间戳减去起始时间戳计算得到，
			*  表示切片包含的媒体数据的播放时长。
			*  
			*  时长单位：
			*  - 通常使用毫秒或90KHz时钟单位
			*  - 90KHz时钟单位：1秒 = 90000个单位
			*  - 毫秒单位：1秒 = 1000毫秒
			*  
			*  @note 时长为0表示尚未添加时间戳或切片为空
			*  @note 时长用于M3U8播放列表中的DURATION字段
			*/
			int64_t     duration_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 文件名（File Name）
			*  
			*  该成员变量用于存储切片的完整文件名。文件名通常由基础文件名、序列号和扩展名组成，
			*  格式为 "stream_0001.ts" 或类似的格式。
			*  
			*  文件名格式：
			*  - "{base_name}_{sequence_no:04d}.ts"
			*  - 例如："stream_0001.ts"、"live_0042.ts" 等
			*  
			*  @note 文件名用于M3U8播放列表中的URI字段
			*  @note 文件名通常包含序列号，便于客户端顺序请求
			*/
			std::string  filename_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 起始时间戳（Start DTS）
			*  
			*  该成员变量用于存储切片的起始时间戳（Decode Time Stamp或Presentation Time Stamp）。
			*  起始时间戳是切片中第一个媒体数据的时间戳，用于计算切片的时长。
			*  
			*  时间戳说明：
			*  - 初始值为-1，表示尚未设置起始时间戳
			*  - 在第一次调用AppendTimestamp()时设置为该时间戳值
			*  - 用于计算duration_ = last_pts - start_dts_
			*  
			*  @note start_dts_为-1表示切片尚未开始或已被重置
			*  @note 时间戳单位通常是毫秒或90KHz时钟
			*/
			int64_t       start_dts_{ -1 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 切片数据（Fragment Data）
			*  
			*  该成员变量用于存储切片的媒体数据。数据以Packet对象形式存储，
			*  包含完整的TS包数据。
			*  
			*  数据结构（Packet Structure）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |                    Packet Header                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |                    TS Packet Array                              |
			*   |                    (188 bytes per packet)                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :                                                               :
			*   |                    ... more TS packets ...                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 数据通过智能指针管理，自动释放内存
			*  @note 数据以TS包形式存储，每个TS包188字节
			*/
			std::shared_ptr<libmedia_transfer_protocol::libmpeg::Packet>   data_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 缓冲区大小（Buffer Size）
			*  
			*  该成员变量用于存储切片缓冲区的初始大小。当数据写入时，
			*  如果缓冲区空间不足，会按kFragmentStepSize步长进行扩展。
			*  
			*  缓冲区管理：
			*  - 初始大小：512KB
			*  - 扩展步长：kFragmentStepSize（128KB）
			*  - 当data_size_接近buf_size_时，自动扩展缓冲区
			*  
			*  @note buf_size_表示缓冲区的容量，data_size_表示实际数据大小
			*  @note 缓冲区大小可以根据实际需求动态扩展
			*/
			int32_t    buf_size_{ 512 * 1024 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 数据大小（Data Size）
			*  
			*  该成员变量用于存储切片中当前已写入的数据大小。这个大小表示
			*  实际的数据量，不一定等于缓冲区的大小。
			*  
			*  @note data_size_ <= buf_size_，表示已使用的大小不超过缓冲区容量
			*  @note 数据大小随着Write()方法的调用而增长
			*/
			int32_t   data_size_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 序列号（Sequence Number）
			*  
			*  该成员变量用于存储切片的序列号。序列号用于标识切片在播放列表中的顺序，
			*  客户端通过序列号来确保按顺序播放切片。
			*  
			*  序列号说明：
			*  - 序列号从0或1开始，按顺序递增
			*  - 序列号用于生成文件名和M3U8播放列表
			*  - 序列号在切片窗口中应该是唯一的
			*  
			*  @note 序列号用于M3U8播放列表中的#EXT-X-MEDIA-SEQUENCE字段
			*  @note 序列号通常与文件名中的序号一致
			*/
			int32_t   sequence_no_{ 0 };
		};
	}
}

#endif //