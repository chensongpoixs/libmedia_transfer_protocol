
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
created: 		2025-04-29

author:			chensong

purpose:		video_demux


 1. VideoTagHeader第一个字节前4位描述帧类型， 后4位描述视频编码ID
 2. CodecId等于7为AVC视频包


    1. 第二个字节为AVCPacketType， 等于0， 表示AVC Sequence Header;等于1， 表示AVC NALU
	2. 接下来三个字节为Compositiontime
	3. 接下来是AVC Raw原始数据


AVCC和AnnexB

1. AVC码流有两种格式AVCC和AnnexB
2. MP4，FLV等文件默认是使用AVCC
3. MPEGTS使用AnnexB
4. 视频流使用哪一种格式， 需要进行尝试



AVCC格式

1. 长度 +NALU





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

#ifndef _C_VIDEO_DEMUX_
#define _C_VIDEO_DEMUX_


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
#include "libmedia_transfer_protocol/libmpeg/cmpeg_type.h"
 
namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{
		/**
		*  @author chensong
		*  @date 2025-04-29
		*  @brief AVC负载格式枚举（AVC Payload Format Enum）
		*  
		*  AVCPayloadFormat枚举用于标识AVC/H264视频流的负载格式。
		*  AVC码流有两种格式：AVCC和Annex-B。
		*  
		*  AVC格式说明：
		*  - AVCC格式：MP4、FLV等文件格式使用，采用长度前缀 + NALU的方式
		*  - Annex-B格式：MPEG-TS等传输流使用，采用起始码（0x00000001或0x000001）分隔NALU
		*  
		*  @note 需要根据数据格式自动检测或手动指定负载格式
		*/
		enum AVCPayloadFormat
		{
			kPayloadFormatNukonwed = 0,  ///< 未知格式
			kPayloadFormatAvcc,          ///< AVCC格式（长度前缀 + NALU）
			kPayloadFormatAnnexB,        ///< Annex-B格式（起始码分隔NALU）
		};

		/**
		*  @author chensong
		*  @date 2025-04-29
		*  @brief 视频解复用器类（Video Demuxer）
		*  
		*  VideoDemux类用于从FLV或其他容器格式中提取视频数据，并进行解复用。
		*  主要处理AVC/H264视频流，支持AVCC和Annex-B两种格式的解析。
		*  
		*  视频解复用说明：
		*  - 从FLV VideoTag中提取AVC视频数据
		*  - 解析VideoTagHeader，识别帧类型和编码ID
		*  - 处理AVC Sequence Header（SPS/PPS）
		*  - 解析AVC NALU单元，支持AVCC和Annex-B格式
		*  - 提取并缓存SPS、PPS参数集
		*  
		*  VideoTag数据结构（Video Tag Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Frame Type   |  CodecID    |  AVCPacketType |  CompositionTime|
		*   |  (4 bits)     |  (4 bits)   |  (8 bits)      |  (24 bits)      |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        AVC Video Data                          :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  VideoTagHeader详细格式（Video Tag Header Detailed Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Frame Type | CodecID    |  AVCPacketType                      |
		*   |  (4 bits)   |  (4 bits)  |  (8 bits)                           |
		*   |  1=keyframe | 7=AVC/H264 |  0=Sequence Header                  |
		*   |  2=inter    |            |  1=NALU                             |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  CompositionTime                                                |
		*   |  (24 bits, signed)                                              |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        AVC Video Data (variable)               :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  AVC Sequence Header格式（AVC Sequence Header Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  configurationVersion | AVCProfileIndication                  |
		*   |  (8 bits)             |  (8 bits)                              |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  profile_compatibility | AVCLevelIndication                   |
		*   |  (8 bits)              |  (8 bits)                             |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  lengthSizeMinusOne | reserved(6) | numOfSequenceParameterSets|
		*   |  (2 bits)           |  (6 bits=111111) |  (5 bits)            |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  sequenceParameterSetLength (16 bits)                          |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        SPS NALU                                :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  numOfPictureParameterSets (8 bits)                            |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  pictureParameterSetLength (16 bits)                           |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        PPS NALU                                :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  AVCC格式NALU结构（AVCC Format NALU Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  NALU Length (variable: 1/2/4 bytes)                          |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        NALU Data (variable)                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  NALU Length (variable: 1/2/4 bytes)                          |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |                        NALU Data (variable)                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                                                               :
		*   |  ... more NALUs ...                                             |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  Annex-B格式NALU结构（Annex-B Format NALU Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  0x00000001 (start code)      |  NALU Header |  NALU Data     |
		*   |  (32 bits)                    |  (8 bits)    |  (variable)    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  0x000001 (short start code)  |  NALU Header |  NALU Data     |
		*   |  (24 bits)                    |  (8 bits)    |  (variable)    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                                                               :
		*   |  ... more NALUs ...                                             |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  NALU类型说明（NALU Type Description）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  forbidden_zero_bit | NRI  |  NALU Type                        |
		*   |  (1 bit=0)          | (2)  |  (5 bits)                         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  NALU Type:                                                   |
		*   |  1-5: VCL (Video Coding Layer) NALU                           |
		*   |      1=Non-IDR, 2=Partition A, 3=Partition B,                 |
		*   |      4=Partition C, 5=IDR                                      |
		*   |  6-12: Non-VCL NALU                                           |
		*   |      6=SEI, 7=SPS, 8=PPS, 9=AUD                                |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note VideoDemux支持AVCC和Annex-B两种格式的自动检测和解析
		*  @note 会提取并缓存SPS和PPS参数集，用于后续解码
		*  @note 支持AUD（Access Unit Delimiter）检测
		*  
		*  使用示例：
		*  @code
		*  VideoDemux demux;
		*  std::list<SampleBuf> outputs;
		*  demux.OnDemux(flv_video_data, data_size, outputs);
		*  // outputs包含解析后的NALU单元
		*  @endcode
		*/
		class VideoDemux
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建VideoDemux实例。使用默认构造函数，所有成员变量使用默认值。
			*  
			*  初始化说明：
			*  - codec_id_: 初始化为未知编解码器
			*  - composition_time_: 初始化为0
			*  - payload_format_: 初始化为未知格式
			*  - 所有标志位初始化为false
			*  
			*  @note 使用默认构造函数，所有成员变量使用默认初始化值
			*/
			VideoDemux() = default;

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理VideoDemux实例。使用默认析构函数，自动释放成员变量。
			*  
			*  @note 使用默认析构函数，自动释放资源
			*/
			~VideoDemux() = default;
		public:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 视频解复用（On Demux）
			*  
			*  该方法用于解析视频数据并进行解复用。它会识别视频格式（AVC/H264等），
			*  解析VideoTagHeader，提取AVC数据，并解析为NALU单元。
			*  
			*  解复用流程（Demux Process）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Parse VideoTagHeader                                      |
			*   |     - Extract Frame Type, CodecID, AVCPacketType              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Extract CompositionTime (24 bits)                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Check CodecID                                             |
			*   |     - If CodecID == 7 (AVC), call DemuxAVC()                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. DemuxAVC() processes AVC data                             |
			*   |     - If AVCPacketType == 0: DecodeAVCSeqHeader()             |
			*   |     - If AVCPacketType == 1: DecodeAvcNalu()                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  5. Output NALU units to outs list                            |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向视频数据缓冲区的指针，包含完整的VideoTag数据
			*  @param size 视频数据的大小，单位为字节
			*  @param outs 输出列表，用于存储解析后的NALU单元
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法会自动识别视频编解码器类型（目前主要支持AVC/H264）
			*  @note 解析后的NALU单元会添加到outs列表中
			*  @note 支持AVC Sequence Header和AVC NALU两种类型
			*  
			*  使用示例：
			*  @code
			*  VideoDemux demux;
			*  std::list<SampleBuf> outputs;
			*  int32_t ret = demux.OnDemux(flv_video_data, data_size, outputs);
			*  @endcode
			*/
			int32_t    OnDemux(const char * data, size_t size, std::list<SampleBuf> & outs);
		private:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AVC视频解复用（Demux AVC Video）
			*  
			*  该方法用于处理AVC/H264视频数据的解复用。根据AVCPacketType类型，
			*  分别处理AVC Sequence Header和AVC NALU数据。
			*  
			*  AVC数据处理流程（AVC Data Processing Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Read AVCPacketType (1 byte)                               |
			*   |     - 0x00: AVC Sequence Header                                |
			*   |     - 0x01: AVC NALU                                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Read CompositionTime (3 bytes, signed)                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Process AVC data based on AVCPacketType                    |
			*   |     - If AVCPacketType == 0: DecodeAVCSeqHeader()             |
			*   |     - If AVCPacketType == 1: DecodeAvcNalu()                   |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向AVC数据缓冲区的指针，包含AVCPacketType和后续数据
			*  @param size AVC数据的大小，单位为字节
			*  @param outs 输出列表，用于存储解析后的NALU单元
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法会根据AVCPacketType调用相应的解码方法
			*  @note CompositionTime会存储在composition_time_成员变量中
			*/
			int32_t    DemuxAVC(const char *data, size_t size, std::list<SampleBuf>& outs);

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 查找Annex-B格式的NALU起始码（Find Annex-B NALU Start Code）
			*  
			*  该方法用于在Annex-B格式的数据中查找NALU起始码。
			*  Annex-B格式使用起始码（0x00000001或0x000001）分隔NALU单元。
			*  
			*  起始码格式（Start Code Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  0x00000001 (4-byte start code)                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  0x000001 (3-byte start code, preceded by 0x00)                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Start Code Format:                                           |
			*   |  - 4-byte: 0x00 0x00 0x00 0x01                                |
			*   |  - 3-byte: 0x00 0x00 0x01                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param p 指向搜索起始位置的指针
			*  @param end 指向搜索结束位置的指针（不包含）
			*  @return 如果找到起始码，返回指向起始码第一个字节的指针；否则返回nullptr
			*  @note 该方法会查找4字节或3字节的起始码
			*  @note 起始码用于标识Annex-B格式中NALU单元的边界
			*/
			const char * FindAnnexbNalu(const char * p, const char * end);

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 解码Annex-B格式的AVC NALU（Decode Annex-B Format AVC NALU）
			*  
			*  该方法用于解析Annex-B格式的AVC NALU数据。Annex-B格式使用起始码
			*  （0x00000001或0x000001）分隔NALU单元。
			*  
			*  Annex-B格式解析流程（Annex-B Format Parsing Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Find start code (0x00000001 or 0x000001)                   |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Extract NALU (from start code to next start code)          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Parse NALU header                                         |
			*   |     - CheckNaluType() to identify NALU type                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. Extract SPS/PPS if needed                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  5. Add NALU to outs list                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  6. Repeat until all data is processed                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向Annex-B格式数据的指针，包含一个或多个NALU单元
			*  @param size 数据的大小，单位为字节
			*  @param outs 输出列表，用于存储解析后的NALU单元
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法会自动识别4字节或3字节的起始码
			*  @note 解析后的NALU单元会添加到outs列表中
			*  @note 会自动检测NALU类型，提取SPS/PPS参数集
			*/
			int32_t   DecodeAVCNaluAnnexb(const char * data, size_t size, std::list<SampleBuf>& outs);

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 解码AVCC格式的AVC NALU（Decode AVCC Format AVC NALU）
			*  
			*  该方法用于解析AVCC格式的AVC NALU数据。AVCC格式使用长度前缀
			*  （1、2或4字节）标识每个NALU单元的长度。
			*  
			*  AVCC格式解析流程（AVCC Format Parsing Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Read NALU length (nalu_unit_length_ bytes, BE)            |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Extract NALU data (length bytes)                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Parse NALU header                                         |
			*   |     - CheckNaluType() to identify NALU type                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. Extract SPS/PPS if needed                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  5. Add NALU to outs list                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  6. Repeat until all data is processed                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  NALU长度前缀格式（NALU Length Prefix Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  If nalu_unit_length_ == 1:                                    |
			*   |  NALU Length (1 byte, BE)                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  If nalu_unit_length_ == 2:                                    |
			*   |  NALU Length (2 bytes, BE)                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  If nalu_unit_length_ == 4:                                    |
			*   |  NALU Length (4 bytes, BE)                                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |                        NALU Data (variable)                     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向AVCC格式数据的指针，包含一个或多个NALU单元
			*  @param size 数据的大小，单位为字节
			*  @param outs 输出列表，用于存储解析后的NALU单元
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法使用nalu_unit_length_成员变量确定长度前缀的字节数
			*  @note 长度值使用大端序（Big-Endian）格式
			*  @note 解析后的NALU单元会添加到outs列表中
			*/
			int32_t   DecodeAVCNalulAvcc(const char *data, size_t size, std::list<SampleBuf>& outs);

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 解码AVC Sequence Header（Decode AVC Sequence Header）
			*  
			*  该方法用于解析AVC Sequence Header。Sequence Header包含SPS和PPS
			*  参数集，这些参数集用于解码H264视频流。
			*  
			*  AVC Sequence Header解析流程（AVC Sequence Header Parsing Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Read configurationVersion (1 byte)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Read AVCProfileIndication (1 byte)                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Read profile_compatibility (1 byte)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  4. Read AVCLevelIndication (1 byte)                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  5. Read lengthSizeMinusOne (2 bits)                           |
			*   |     - Calculate nalu_unit_length_ = lengthSizeMinusOne + 1     |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  6. Read numOfSequenceParameterSets (5 bits)                   |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  7. For each SPS:                                             |
			*   |     - Read sequenceParameterSetLength (16 bits)                |
			*   |     - Extract SPS NALU data                                    |
			*   |     - Store SPS in sps_ member                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  8. Read numOfPictureParameterSets (8 bits)                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  9. For each PPS:                                             |
			*   |     - Read pictureParameterSetLength (16 bits)                 |
			*   |     - Extract PPS NALU data                                    |
			*   |     - Store PPS in pps_ member                                 |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向AVC Sequence Header数据的指针
			*  @param size Sequence Header数据的大小，单位为字节
			*  @param outs 输出列表，用于存储解析后的NALU单元（通常为空）
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法会提取并存储SPS和PPS参数集到sps_和pps_成员变量中
			*  @note 会设置nalu_unit_length_成员变量，用于后续AVCC格式解析
			*  @note 会设置avc_ok_标志，表示已收到Sequence Header
			*/
			int32_t  DecodeAVCSeqHeader(const char * data, size_t size, std::list<SampleBuf>& outs);

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 解码AVC NALU（Decode AVC NALU）
			*  
			*  该方法用于解析AVC NALU数据。它会根据payload_format_自动选择
			*  相应的解析方法（AVCC或Annex-B格式）。
			*  
			*  AVC NALU解码流程（AVC NALU Decoding Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Check payload_format_                                      |
			*   |     - If AVCC: call DecodeAVCNalulAvcc()                       |
			*   |     - If Annex-B: call DecodeAVCNaluAnnexb()                   |
			*   |     - If Unknown: try to detect format                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Parse NALU units                                           |
			*   |     - CheckNaluType() for each NALU                            |
			*   |     - Extract SPS/PPS if needed                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Add NALU units to outs list                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向AVC NALU数据的指针
			*  @param size NALU数据的大小，单位为字节
			*  @param outs 输出列表，用于存储解析后的NALU单元
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法会根据payload_format_自动选择解析方法
			*  @note 如果格式未知，会尝试自动检测格式
			*  @note 解析后的NALU单元会添加到outs列表中
			*/
			int32_t  DecodeAvcNalu(const char * data, size_t size, std::list<SampleBuf> & outs);

		public:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 检查NALU类型（Check NALU Type）
			*  
			*  该方法用于检查NALU单元的类型，并更新相应的标志位。
			*  根据NALU类型设置has_idr_、has_aud_、has_sps_pps_等标志。
			*  
			*  NALU类型检测（NALU Type Detection）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NALU Header (8 bits)                                         |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  | forbidden_zero_bit | NRI  |  NALU Type                      |
			*   |  |  (1 bit=0)          | (2)  |  (5 bits)                       |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  NALU Type Mapping:                                            |
			*   |  1=Non-IDR, 5=IDR -> has_idr_ = true                           |
			*   |  6=SEI, 7=SPS, 8=PPS -> has_sps_pps_ = true                    |
			*   |  9=AUD -> has_aud_ = true                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向NALU数据的指针，第一个字节包含NALU类型
			*  @note 该方法会检查NALU类型并更新相应的标志位
			*  @note NALU类型在第一个字节的低5位（bits 0-4）
			*  @note 会设置has_idr_、has_aud_、has_sps_pps_等标志
			*/
			void CheckNaluType(const char * data);

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 检查是否有IDR帧（Has IDR Frame）
			*  
			*  该方法用于检查是否已接收到IDR（Instantaneous Decoder Refresh）帧。
			*  IDR帧是关键帧，用于快速恢复视频解码状态。
			*  
			*  @return 如果已接收到IDR帧，返回true；否则返回false
			*  @note IDR帧的NALU类型为5（NALU type = 5）
			*  @note IDR帧用于标识关键帧，用于快速恢复解码状态
			*/
			bool HasIdr() const;

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 检查是否有AUD（Has AUD）
			*  
			*  该方法用于检查是否已接收到AUD（Access Unit Delimiter）单元。
			*  AUD用于标识访问单元（Access Unit）的边界。
			*  
			*  @return 如果已接收到AUD，返回true；否则返回false
			*  @note AUD的NALU类型为9（NALU type = 9）
			*  @note AUD用于标识访问单元的边界
			*/
			bool HasAud() const;

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 检查是否有SPS和PPS（Has SPS/PPS）
			*  
			*  该方法用于检查是否已接收到SPS（Sequence Parameter Set）和PPS
			*  （Picture Parameter Set）参数集。SPS和PPS是H264解码必需的参数。
			*  
			*  @return 如果已接收到SPS和PPS，返回true；否则返回false
			*  @note SPS的NALU类型为7（NALU type = 7）
			*  @note PPS的NALU类型为8（NALU type = 8）
			*  @note SPS和PPS通常从AVC Sequence Header中提取
			*/
			bool HasSpsPps() const;


			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 获取编解码器ID（Get Codec ID）
			*  
			*  该方法用于获取视频流的编解码器ID。编解码器ID标识了视频流的编码格式。
			*  
			*  @return 返回编解码器ID枚举值
			*  @note 目前主要支持AVC/H264编解码器（CodecID = 7）
			*  
			*  使用示例：
			*  @code
			*  VideoCodecID codec_id = demux.GetCodecID();
			*  if (codec_id == kVideoCodecH264) {
			*      // 处理H264视频
			*  }
			*  @endcode
			*/
			VideoCodecID GetCodecID() const
			{
				return codec_id_;
			}

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 获取合成时间（Get Composition Time）
			*  
			*  该方法用于获取视频帧的合成时间（Composition Time）。合成时间用于
			*  标识视频帧的显示时间，相对于解码时间（DTS）的偏移。
			*  
			*  合成时间格式（Composition Time Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  CompositionTime (24 bits, signed)                             |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  24位有符号整数，单位为毫秒或90KHz时钟                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @return 返回合成时间值，单位为毫秒或90KHz时钟
			*  @note 合成时间是一个24位有符号整数
			*  @note 合成时间用于标识视频帧的显示时间偏移
			*  
			*  使用示例：
			*  @code
			*  int32_t cst = demux.GetCST();
			*  // cst 表示合成时间偏移
			*  @endcode
			*/
			int32_t   GetCST() const
			{
				return composition_time_;
			}

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 获取SPS参数集（Get SPS）
			*  
			*  该方法用于获取SPS（Sequence Parameter Set）参数集。SPS包含H264
			*  视频流的序列级参数，是解码必需的参数。
			*  
			*  SPS格式（SPS Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NALU Header |  SPS Data (variable)                            |
			*   |  (8 bits)    |  (variable length)                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NALU Header: 0x67 (NALU type = 7, SPS)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @return 返回SPS参数集的常量引用。如果尚未收到SPS，返回空字符串
			*  @note SPS通常从AVC Sequence Header中提取
			*  @note SPS包含视频流的序列级参数（分辨率、帧率等）
			*  
			*  使用示例：
			*  @code
			*  const std::string& sps = demux.GetSPS();
			*  // sps 包含SPS参数集的二进制数据
			*  @endcode
			*/
			const std::string & GetSPS() const
			{
				return sps_;
			}

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 获取PPS参数集（Get PPS）
			*  
			*  该方法用于获取PPS（Picture Parameter Set）参数集。PPS包含H264
			*  视频流的图像级参数，是解码必需的参数。
			*  
			*  PPS格式（PPS Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NALU Header |  PPS Data (variable)                            |
			*   |  (8 bits)    |  (variable length)                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NALU Header: 0x68 (NALU type = 8, PPS)                        |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @return 返回PPS参数集的常量引用。如果尚未收到PPS，返回空字符串
			*  @note PPS通常从AVC Sequence Header中提取
			*  @note PPS包含视频流的图像级参数（熵编码模式等）
			*  
			*  使用示例：
			*  @code
			*  const std::string& pps = demux.GetPPS();
			*  // pps 包含PPS参数集的二进制数据
			*  @endcode
			*/
			const std::string & GetPPS() const
			{
				return pps_;
			}

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 重置解复用器（Reset Demuxer）
			*  
			*  该方法用于重置解复用器的状态标志位。重置后，所有标志位都会被清空，
			*  可以用于处理新的视频流。
			*  
			*  重置操作：
			*  1. 清空has_aud_标志
			*  2. 清空has_idr_标志
			*  3. 清空has_sps_pps_标志
			*  
			*  @note 重置后，解复用器可以用于处理新的视频流
			*  @note 不会清空SPS/PPS参数集，这些参数集在需要时会重新提取
			*  
			*  使用示例：
			*  @code
			*  demux.Reset();
			*  // 解复用器已重置，可以处理新的视频流
			*  @endcode
			*/
			void Reset()
			{
				has_aud_ = false;
				has_idr_ = false;
				has_sps_pps_ = false;
			}
		private:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 编解码器ID（Codec ID）
			*  
			*  该成员变量用于存储视频流的编解码器ID。编解码器ID标识了视频流的编码格式。
			*  
			*  编解码器ID说明：
			*  - 0x07: AVC/H264视频
			*  - 0x0C: HEVC/H265视频
			*  - 其他值：其他视频编解码器
			*  
			*  @note 目前主要支持AVC/H264编解码器
			*  @note 编解码器ID在解析VideoTagHeader时设置
			*/
			VideoCodecID    codec_id_;

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 合成时间（Composition Time）
			*  
			*  该成员变量用于存储视频帧的合成时间。合成时间用于标识视频帧的显示时间，
			*  相对于解码时间（DTS）的偏移。
			*  
			*  合成时间说明：
			*  - 24位有符号整数
			*  - 单位：毫秒或90KHz时钟
			*  - 用于标识视频帧的显示时间偏移
			*  
			*  @note 初始值为0，表示无时间偏移
			*  @note 合成时间在解析AVC数据时从CompositionTime字段提取
			*/
			int32_t    composition_time_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 配置版本（Configuration Version）
			*  
			*  该成员变量用于存储AVC Sequence Header的配置版本号。
			*  配置版本用于标识AVC配置的版本，固定值为1。
			*  
			*  @note 从AVC Sequence Header的configurationVersion字段提取
			*  @note 配置版本固定为1，表示当前AVC配置版本
			*/
			uint8_t    config_version_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AVC配置级别（AVC Profile）
			*  
			*  该成员变量用于存储AVC配置的级别标识。配置级别用于标识H264编码的配置档次。
			*  
			*  配置级别说明：
			*  - 0x42: Baseline Profile
			*  - 0x4D: Main Profile
			*  - 0x64: High Profile
			*  - 其他值：其他配置级别
			*  
			*  @note 从AVC Sequence Header的AVCProfileIndication字段提取
			*  @note 配置级别用于标识H264编码的配置档次
			*/
			uint8_t     profile_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 配置兼容性（Profile Compatibility）
			*  
			*  该成员变量用于存储AVC配置的兼容性标志。兼容性标志用于标识H264配置的
			*  兼容性信息。
			*  
			*  @note 从AVC Sequence Header的profile_compatibility字段提取
			*  @note 兼容性标志用于标识配置的兼容性
			*/
			uint8_t    profile_cmpa_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AVC级别（AVC Level）
			*  
			*  该成员变量用于存储AVC配置的级别标识。级别用于标识H264编码的级别。
			*  
			*  级别说明：
			*  - 0x1F: Level 3.1
			*  - 0x28: Level 4.0
			*  - 0x29: Level 4.1
			*  - 其他值：其他级别
			*  
			*  @note 从AVC Sequence Header的AVCLevelIndication字段提取
			*  @note 级别用于标识H264编码的级别
			*/
			uint8_t    level_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AVC配置完成标志（AVC Configuration OK）
			*  
			*  该成员变量用于标识是否已收到AVC Sequence Header。只有当收到
			*  Sequence Header后，AVC配置才算完成。
			*  
			*  @note 初始值为false，表示尚未收到Sequence Header
			*  @note 收到Sequence Header后，该标志会设置为true
			*  @note 该标志用于标识AVC配置是否完成
			*/
			bool		avc_ok_{ false };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief NALU单元长度前缀大小（NALU Unit Length Prefix Size）
			*  
			*  该成员变量用于存储AVCC格式中NALU长度前缀的字节数。用于解析AVCC格式
			*  的NALU单元。
			*  
			*  NALU长度前缀大小说明：
			*  - 1: 使用1字节长度前缀
			*  - 2: 使用2字节长度前缀
			*  - 4: 使用4字节长度前缀
			*  
			*  @note 初始值为0，表示尚未设置
			*  @note 从AVC Sequence Header的lengthSizeMinusOne字段计算得到
			*  @note 用于AVCC格式的NALU解析
			*/
			int32_t      nalu_unit_length_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief SPS参数集（Sequence Parameter Set）
			*  
			*  该成员变量用于存储SPS（Sequence Parameter Set）参数集的二进制数据。
			*  SPS包含H264视频流的序列级参数，是解码必需的参数。
			*  
			*  SPS数据结构（SPS Data Structure）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NALU Header |  SPS Data (variable)                            |
			*   |  0x67        |  (variable length)                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note SPS从AVC Sequence Header中提取
			*  @note SPS包含视频流的序列级参数（分辨率、帧率等）
			*  @note SPS用于视频解码器初始化
			*/
			std::string     sps_;

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief PPS参数集（Picture Parameter Set）
			*  
			*  该成员变量用于存储PPS（Picture Parameter Set）参数集的二进制数据。
			*  PPS包含H264视频流的图像级参数，是解码必需的参数。
			*  
			*  PPS数据结构（PPS Data Structure）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  NALU Header |  PPS Data (variable)                            |
			*   |  0x68        |  (variable length)                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note PPS从AVC Sequence Header中提取
			*  @note PPS包含视频流的图像级参数（熵编码模式等）
			*  @note PPS用于视频解码器初始化
			*/
			std::string     pps_;

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AVC负载格式（AVC Payload Format）
			*  
			*  该成员变量用于存储AVC数据的负载格式。AVC码流有两种格式：AVCC和Annex-B。
			*  
			*  负载格式说明：
			*  - kPayloadFormatAvcc: AVCC格式，使用长度前缀分隔NALU
			*  - kPayloadFormatAnnexB: Annex-B格式，使用起始码分隔NALU
			*  - kPayloadFormatNukonwed: 未知格式，需要自动检测
			*  
			*  @note 初始值为未知格式，需要根据数据自动检测
			*  @note 负载格式用于选择相应的NALU解析方法
			*/
			AVCPayloadFormat      payload_format_{ kPayloadFormatNukonwed };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 是否有AUD标志（Has AUD Flag）
			*  
			*  该成员变量用于标识是否已接收到AUD（Access Unit Delimiter）单元。
			*  AUD用于标识访问单元的边界。
			*  
			*  @note 初始值为false，表示尚未接收到AUD
			*  @note AUD的NALU类型为9（NALU type = 9）
			*  @note 该标志在CheckNaluType()方法中设置
			*/
			bool       has_aud_{ false };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 是否有IDR帧标志（Has IDR Frame Flag）
			*  
			*  该成员变量用于标识是否已接收到IDR（Instantaneous Decoder Refresh）帧。
			*  IDR帧是关键帧，用于快速恢复视频解码状态。
			*  
			*  @note 初始值为false，表示尚未接收到IDR帧
			*  @note IDR帧的NALU类型为5（NALU type = 5）
			*  @note 该标志在CheckNaluType()方法中设置
			*/
			bool		has_idr_{ false };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 是否有SPS和PPS标志（Has SPS/PPS Flag）
			*  
			*  该成员变量用于标识是否已接收到SPS（Sequence Parameter Set）和PPS
			*  （Picture Parameter Set）参数集。SPS和PPS是H264解码必需的参数。
			*  
			*  @note 初始值为false，表示尚未接收到SPS和PPS
			*  @note SPS的NALU类型为7（NALU type = 7）
			*  @note PPS的NALU类型为8（NALU type = 8）
			*  @note 该标志在CheckNaluType()或DecodeAVCSeqHeader()方法中设置
			*/
			bool		has_sps_pps_{ false };

		};

	}
}


#endif // libmedia_transfer_protocol