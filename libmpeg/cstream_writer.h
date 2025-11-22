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

#ifndef _C_STREAM_WRITER_
#define _C_STREAM_WRITER_


#include <cstdint>
#include <memory>

#include <string>
#include <unordered_map>
#include <memory>
#include <sstream>
 
 
#include <unordered_map>
 
#include <functional>
#include <memory>

namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{

		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief 段最大大小常量（Section Maximum Size Constant）
		*  
		*  该常量定义了PSI Section的最大大小。PSI Section的最大长度为1021字节
		*  （不包括4字节的CRC32），加上头部和CRC32后，总大小约为1200字节。
		*  
		*  段大小说明：
		*  - PSI Section数据部分最大：1021字节
		*  - PSI Section总长度（包含头部和CRC32）：约1200字节
		*  - 该常量用于缓冲区分配和大小检查
		*  
		*  @note PSI Section的大小限制由MPEG-TS标准规定
		*  @note 该常量用于确保PSI Section不超过最大大小
		*/
		const int32_t kSectionMaxSize = 1200;

		/**
		*  @author chensong
		*  @date 2025-05-02
		*  @brief 流写入器抽象基类（Stream Writer Abstract Base Class）
		*  
		*  StreamWriter是流写入器的抽象基类，定义了写入媒体流数据的接口。
		*  它支持写入数据、添加时间戳、管理SPS/PPS等编解码器参数。
		*  
		*  流写入器说明：
		*  - StreamWriter是抽象基类，定义了写入流数据的标准接口
		*  - 子类实现具体的写入逻辑（如TS包写入、文件写入等）
		*  - 支持SPS（Sequence Parameter Set）和PPS（Picture Parameter Set）管理
		*  - 支持时间戳管理
		*  
		*  流数据格式（Stream Data Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Stream Header (variable)                                      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  SPS/PPS (optional, if needed)                                  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        Media Data                              :
		*   |                        (variable length)                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  SPS/PPS数据格式（SPS/PPS Data Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  SPS NALU (variable length)                                     |
		*   |  - NAL Header: 0x67 (SPS)                                       |
		*   |  - SPS RBSP Data                                                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  PPS NALU (variable length)                                     |
		*   |  - NAL Header: 0x68 (PPS)                                       |
		*   |  - PPS RBSP Data                                                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note StreamWriter是抽象基类，不能直接实例化
		*  @note 子类需要实现所有纯虚函数（AppendTimestamp、Write、Data、Size）
		*  @note 支持SPS/PPS管理，用于H264视频编解码器参数
		*  
		*  使用示例：
		*  @code
		*  // StreamWriter是抽象基类，不能直接实例化，通过子类使用
		*  class MyStreamWriter : public StreamWriter {
		*      // 实现纯虚函数
		*  };
		*  @endcode
		*/
		class StreamWriter
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建StreamWriter实例。初始化流写入器的成员变量。
			*  
			*  初始化说明：
			*  - sps_: 初始化为空字符串
			*  - pps_: 初始化为空字符串
			*  - sps_pps_appended_: 初始化为false，表示尚未附加SPS/PPS
			*  
			*  @note 使用默认构造函数，所有成员变量使用默认初始化值
			*/
			StreamWriter();

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理StreamWriter实例。使用虚析构函数，
			*  确保派生类对象正确释放。
			*  
			*  @note 使用虚析构函数，确保派生类对象正确释放
			*/
			virtual ~StreamWriter(){}

		public:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 添加时间戳（Append Timestamp）
			*  
			*  该纯虚方法用于向流中添加时间戳。时间戳用于标识媒体数据的播放时间。
			*  不同的流写入器实现可能有不同的时间戳处理方式。
			*  
			*  时间戳格式（Timestamp Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |                        pts                                    |
			*   |                        (64 bits)                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PTS: Presentation Time Stamp, 90KHz clock or milliseconds    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param pts 表示时间戳（Presentation Time Stamp），单位为毫秒或90KHz时钟
			*  @note 该方法是纯虚函数，必须由子类实现
			*  @note 时间戳用于同步音视频播放
			*  
			*  使用示例：
			*  @code
			*  // 在子类中实现
			*  void MyStreamWriter::AppendTimestamp(int64_t pts) {
			*      // 实现时间戳处理逻辑
			*  }
			*  @endcode
			*/
			virtual void AppendTimestamp(int64_t pts) = 0;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 写入数据（Write Data）
			*  
			*  该纯虚方法用于向流中写入数据。数据可以是媒体数据（音频/视频）
			*  或其他流数据。不同的流写入器实现可能有不同的写入方式。
			*  
			*  数据写入格式（Data Writing Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |                        Data Buffer                            |
			*   |                        (size bytes)                            |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Write data to stream buffer                                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param buf 指向要写入数据的缓冲区指针，不能为空
			*  @param size 要写入的数据大小，单位为字节
			*  @return 返回实际写入的数据大小，单位为字节。如果发生错误，可能小于size
			*  @note 该方法是纯虚函数，必须由子类实现
			*  @note 不同的流写入器实现可能有不同的写入方式和错误处理
			*  
			*  使用示例：
			*  @code
			*  // 在子类中实现
			*  int32_t MyStreamWriter::Write(void* buf, size_t size) {
			*      // 实现数据写入逻辑
			*      return written_size;
			*  }
			*  @endcode
			*/
			virtual int32_t Write(void * buf, size_t size) = 0;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取数据指针（Get Data Pointer）
			*  
			*  该纯虚方法用于获取流数据的指针。返回的指针指向流数据的缓冲区，
			*  可以用于读取或传输数据。
			*  
			*  数据指针格式（Data Pointer Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Data Buffer <-- Data() returns pointer here                  |
			*   |  (Size() bytes)                                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @return 返回指向流数据缓冲区的字符指针。如果数据为空，可能返回nullptr
			*  @note 该方法是纯虚函数，必须由子类实现
			*  @note 返回的指针指向流内部的数据缓冲区
			*  @note 数据大小可以通过Size()方法获取
			*  
			*  使用示例：
			*  @code
			*  // 在子类中实现
			*  char* MyStreamWriter::Data() {
			*      return buffer_ptr;
			*  }
			*  @endcode
			*/
			virtual char * Data() = 0;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取数据大小（Get Data Size）
			*  
			*  该纯虚方法用于获取流中实际存储的数据大小。返回的大小表示
			*  当前流中已写入的数据量。
			*  
			*  @return 返回流中实际存储的数据大小，单位为字节
			*  @note 该方法是纯虚函数，必须由子类实现
			*  @note 返回的大小是已写入的数据大小，而不是缓冲区容量
			*  
			*  使用示例：
			*  @code
			*  // 在子类中实现
			*  int32_t MyStreamWriter::Size() {
			*      return current_size;
			*  }
			*  @endcode
			*/
			virtual int32_t Size() = 0;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置SPS（Set Sequence Parameter Set）
			*  
			*  该方法用于设置SPS（Sequence Parameter Set）。SPS是H264视频编码中的
			*  序列参数集，包含图像的宽度、高度、帧率等编码参数。
			*  
			*  SPS NALU格式（SPS NALU Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NAL Header    |  SPS RBSP Data (variable)                    |
			*   |  0x67 (SPS)    |                                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  SPS contains: profile_idc, level_idc, width, height, ...     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param sps SPS数据的字符串，包含完整的SPS NALU（包括NAL Header 0x67）
			*  @note SPS包含视频编码的基本参数，如宽度、高度、帧率等
			*  @note SPS通常需要在关键帧（IDR）前写入流中
			*  
			*  使用示例：
			*  @code
			*  std::string sps_data = ...;  // SPS NALU数据
			*  writer->SetSPS(sps_data);
			*  @endcode
			*/
			void SetSPS(const std::string& sps)
			{
				sps_ = sps;
			}

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置PPS（Set Picture Parameter Set）
			*  
			*  该方法用于设置PPS（Picture Parameter Set）。PPS是H264视频编码中的
			*  图像参数集，包含熵编码模式、量化参数等编码参数。
			*  
			*  PPS NALU格式（PPS NALU Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NAL Header    |  PPS RBSP Data (variable)                    |
			*   |  0x68 (PPS)    |                                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  PPS contains: pic_parameter_set_id, entropy_coding_mode, ... |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param pps PPS数据的字符串，包含完整的PPS NALU（包括NAL Header 0x68）
			*  @note PPS包含图像编码的特定参数，如熵编码模式、量化参数等
			*  @note PPS通常需要在关键帧（IDR）前写入流中，紧跟SPS之后
			*  
			*  使用示例：
			*  @code
			*  std::string pps_data = ...;  // PPS NALU数据
			*  writer->SetPPS(pps_data);
			*  @endcode
			*/
			void SetPPS(const std::string & pps)
			{
				pps_ = pps;
			}

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取SPS（Get Sequence Parameter Set）
			*  
			*  该方法用于获取当前设置的SPS（Sequence Parameter Set）。
			*  
			*  @return 返回SPS数据的常量引用。如果未设置，返回空字符串
			*  @note SPS包含H264视频编码的基本参数
			*  
			*  使用示例：
			*  @code
			*  const std::string& sps = writer->GetSPS();
			*  if (!sps.empty()) {
			*      // 使用SPS数据
			*  }
			*  @endcode
			*/
			const std::string & GetSPS() const
			{
				return sps_;
			}

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取PPS（Get Picture Parameter Set）
			*  
			*  该方法用于获取当前设置的PPS（Picture Parameter Set）。
			*  
			*  @return 返回PPS数据的常量引用。如果未设置，返回空字符串
			*  @note PPS包含H264视频编码的图像参数
			*  
			*  使用示例：
			*  @code
			*  const std::string& pps = writer->GetPPS();
			*  if (!pps.empty()) {
			*      // 使用PPS数据
			*  }
			*  @endcode
			*/
			const std::string & GetPPS() const
			{
				return pps_;
			}

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 设置SPS/PPS附加标志（Set SPS/PPS Appended Flag）
			*  
			*  该方法用于设置SPS/PPS是否已附加到流中的标志。当SPS/PPS已经
			*  写入流中后，应设置此标志为true，避免重复写入。
			*  
			*  SPS/PPS附加流程（SPS/PPS Appending Process）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Check if sps_pps_appended_ == false                       |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. If false, append SPS/PPS to stream                        |
			*   |     - Write SPS NALU (0x67 + SPS RBSP)                        |
			*   |     - Write PPS NALU (0x68 + PPS RBSP)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Set sps_pps_appended_ = true                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param b 布尔值，true表示SPS/PPS已附加，false表示尚未附加
			*  @note 此标志用于避免重复写入SPS/PPS
			*  @note SPS/PPS通常在关键帧（IDR）前写入流中
			*  
			*  使用示例：
			*  @code
			*  writer->SetSpsPpsAppended(true);  // 标记SPS/PPS已附加
			*  @endcode
			*/
			void SetSpsPpsAppended(bool b)
			{
				sps_pps_appended_ = b;
			}

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief 获取SPS/PPS附加标志（Get SPS/PPS Appended Flag）
			*  
			*  该方法用于获取SPS/PPS是否已附加到流中的标志。
			*  
			*  @return 如果SPS/PPS已附加，返回true；否则返回false
			*  @note 此标志用于判断是否需要写入SPS/PPS
			*  
			*  使用示例：
			*  @code
			*  if (!writer->GetSpsPpsAppended()) {
			*      // 需要写入SPS/PPS
			*  }
			*  @endcode
			*/
			bool GetSpsPpsAppended() const
			{
				return sps_pps_appended_;
			}
		protected:
			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief SPS数据（Sequence Parameter Set Data）
			*  
			*  该成员变量用于存储SPS（Sequence Parameter Set）数据。SPS是H264视频
			*  编码中的序列参数集，包含图像的宽度、高度、帧率等编码参数。
			*  
			*  SPS数据格式（SPS Data Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NAL Header    |  SPS RBSP Data (variable)                    |
			*   |  0x67 (SPS)    |                                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Contains: profile_idc, level_idc, width, height, fps, ...    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note SPS包含视频编码的基本参数，如宽度、高度、帧率等
			*  @note SPS通常需要在关键帧（IDR）前写入流中
			*  @note SPS数据包含完整的NALU（包括NAL Header 0x67）
			*/
			std::string sps_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief PPS数据（Picture Parameter Set Data）
			*  
			*  该成员变量用于存储PPS（Picture Parameter Set）数据。PPS是H264视频
			*  编码中的图像参数集，包含熵编码模式、量化参数等编码参数。
			*  
			*  PPS数据格式（PPS Data Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NAL Header    |  PPS RBSP Data (variable)                    |
			*   |  0x68 (PPS)    |                                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Contains: pic_parameter_set_id, entropy_coding_mode, ...     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note PPS包含图像编码的特定参数，如熵编码模式、量化参数等
			*  @note PPS通常需要在关键帧（IDR）前写入流中，紧跟SPS之后
			*  @note PPS数据包含完整的NALU（包括NAL Header 0x68）
			*/
			std::string pps_;

			/**
			*  @author chensong
			*  @date 2025-05-02
			*  @brief SPS/PPS附加标志（SPS/PPS Appended Flag）
			*  
			*  该成员变量用于标识SPS/PPS是否已附加到流中。当SPS/PPS已经写入流中后，
			*  此标志设置为true，避免重复写入。
			*  
			*  标志状态（Flag States）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  sps_pps_appended_ = false: SPS/PPS not yet appended          |
			*   |  sps_pps_appended_ = true: SPS/PPS already appended           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为false，表示尚未附加SPS/PPS
			*  @note 此标志用于避免重复写入SPS/PPS
			*  @note SPS/PPS通常在关键帧（IDR）前写入流中
			*/
			bool sps_pps_appended_{ false };
		};
	}
}

#endif // _C_STREAM_WRITER_