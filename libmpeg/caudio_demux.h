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

purpose:		audio_demux




			0     1     2    3    4     5    6    7     8    9   10   11  12
			+-------------------------------------------------------+
			|        format       |   rate   |size| type|           |
			+-------------------------------------------------------+
					  4bit            2bit    1bit  1bit




 .音频编码格式为 sound_format为2 
1. 第二个字节开始， 就是MP3的RAW数据
 
 音频编码格式为 sound_format为10


1. 第二个字节为AACPacketType， 值为0， 表示当前包为AAC sequence header， 否则为AAC raw原始数据
2. 第三个字节开始为AAC原始音频数据
 



 AAC sSequence Header


 1. audioObjectType 4 位
 2. samplingFrequencyIndexx  5位
 3. channelConfiguration： 4位








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

#ifndef _C_AUDIO_DEMUX_
#define _C_AUDIO_DEMUX_


#include <cstdint>
#include <memory> 
#include <string>
#include <unordered_map>
#include <memory>
#include <sstream> 
 #include <list>
#include <unordered_map> 
#include <functional>
#include <memory> 
#include "libmedia_transfer_protocol/libmpeg/packet.h"
#include "libmedia_transfer_protocol/libmpeg/cmpeg_type.h"

namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{

		/**
		*  @author chensong
		*  @date 2025-04-29
		*  @brief 音频解复用器类（Audio Demuxer）
		*  
		*  AudioDemux类用于解复用音频数据，支持MP3和AAC格式的音频流。
		*  它从FLV或RTMP格式的音频Tag中提取原始音频数据，并解析音频格式信息。
		*  
		*  音频解复用说明：
		*  - 音频Tag Header包含音频格式、采样率、采样深度、通道数等信息
		*  - 支持MP3格式（sound_format=2）：直接从Tag Data开始为MP3 RAW数据
		*  - 支持AAC格式（sound_format=10）：需要解析AAC Sequence Header和RAW数据
		*  - AAC Sequence Header包含音频对象类型、采样率索引、通道配置等信息
		*  
		*  音频Tag Header数据结构（Audio Tag Header Data Structure）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  sound_format | sound_rate | sound_size | sound_type |        |
		*   |  (4 bits)     |  (2 bits)  |  (1 bit)   |  (1 bit)   |        |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Byte 0: [sound_format(4)][sound_rate(2)][sound_size(1)][sound_type(1)] |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Byte 1: AACPacketType (for AAC) or MP3 RAW Data (for MP3)    |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   :                        Audio Data (variable)                  :
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  音频Tag Header字节格式（Audio Tag Header Byte Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Byte 0 (8 bits)                                               |
		*   |  Bit 7-4: sound_format (4 bits)                                |
		*   |  Bit 3-2: sound_rate (2 bits)                                  |
		*   |  Bit 1:   sound_size (1 bit)                                   |
		*   |  Bit 0:   sound_type (1 bit)                                   |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  sound_format: 2=MP3, 10=AAC                                   |
		*   |  sound_rate: 0=5.5K, 1=11K, 2=22K, 3=44K                      |
		*   |  sound_size: 0=8bit, 1=16bit                                   |
		*   |  sound_type: 0=mono, 1=stereo                                  |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  AAC Sequence Header格式（AAC Sequence Header Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Byte 0: AACPacketType (0=Sequence Header, 1=RAW Data)        |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  Byte 1: Audio Specific Config (ASC)                          |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  |  audioObjectType(5) | samplingFrequencyIndex(4) |          | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  |  channelConfiguration(4) | ...                             | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  AAC Audio Specific Config (ASC) 格式：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  audioObjectType    | samplingFrequencyIndex |channelConfig  | |
		*   |  (5 bits)           |  (4 bits)              |  (4 bits)     | |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  audioObjectType: 2=AAC-LC, 5=HE-AAC, 29=HE-AAC v2           |
		*   |  samplingFrequencyIndex: 索引到采样率表                       |
		*   |  channelConfiguration: 1=mono, 2=stereo, 3=2.1, ...         |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  MP3 RAW数据格式（MP3 RAW Data Format）：
		*  
		*    0                   1                   2                   3
		*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  MP3 Frame Header (4 bytes)                                    |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   |  |  Sync Word(11) | Version(2) | Layer(2) | ...              | |
		*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*   |  MP3 Frame Data (variable)                                     |
		*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*  
		*  @note AudioDemux支持MP3和AAC两种音频格式
		*  @note MP3格式：从Tag Data开始就是MP3 RAW数据
		*  @note AAC格式：需要解析Sequence Header，然后提取RAW数据
		*  
		*  使用示例：
		*  @code
		*  AudioDemux demux;
		*  std::list<SampleBuf> samples;
		*  demux.OnDemux(audio_data, data_size, samples);
		*  int32_t codec_id = demux.GetCodecId();  // 2=MP3, 10=AAC
		*  @endcode
		*/
		class AudioDemux
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建AudioDemux实例。使用默认构造函数，所有成员变量使用默认值。
			*  
			*  初始化说明：
			*  - sound_format_: 初始化为0，表示尚未识别音频格式
			*  - sound_rate_: 初始化为0，表示采样率未设置
			*  - sound_size_: 初始化为0，表示采样深度未设置
			*  - sound_type_: 初始化为0，表示通道类型未设置
			*  - aac_object_: 初始化为默认值
			*  - aac_sample_rate_: 初始化为0
			*  - aac_channel_: 初始化为0
			*  - aac_ok_: 初始化为false，表示尚未收到AAC Sequence Header
			*  
			*  @note 使用默认构造函数，所有成员变量使用默认初始化值
			*/
			AudioDemux() = default;

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理AudioDemux实例。使用默认析构函数，自动释放成员变量。
			*  
			*  @note 使用默认析构函数，自动释放资源
			*/
			~AudioDemux() = default;


		public:
			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 解复用音频数据（Demux Audio Data）
			*  
			*  该方法用于解复用音频Tag数据。它会解析音频Tag Header，提取音频格式信息，
			*  并根据音频格式（MP3或AAC）调用相应的解复用方法。
			*  
			*  解复用流程（Demux Flow）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  1. Parse Audio Tag Header (Byte 0)                          |
			*   |     - Extract sound_format (4 bits)                           |
			*   |     - Extract sound_rate (2 bits)                             |
			*   |     - Extract sound_size (1 bit)                              |
			*   |     - Extract sound_type (1 bit)                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  2. Check audio format                                        |
			*   |     - If sound_format == 2: Call DemuxMP3()                   |
			*   |     - If sound_format == 10: Call DemuxAAC()                  |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  3. Return demuxed audio samples                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  音频数据格式识别（Audio Format Identification）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Byte 0: [sound_format(4)][sound_rate(2)][sound_size(1)][sound_type(1)] |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  sound_format values:                                         |
			*   |  2 = MP3 format -> DemuxMP3()                                 |
			*   |  10 = AAC format -> DemuxAAC()                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向音频Tag数据的指针，包含完整的音频Tag（Header+Data）
			*  @param size 音频Tag数据的大小，单位为字节
			*  @param list 输出参数，用于存储解复用后的音频样本缓冲区列表
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法会自动识别音频格式（MP3或AAC）并调用相应的解复用方法
			*  @note 解复用后的音频样本存储在SampleBuf列表中
			*  
			*  使用示例：
			*  @code
			*  AudioDemux demux;
			*  std::list<SampleBuf> samples;
			*  int32_t ret = demux.OnDemux(audio_tag_data, tag_size, samples);
			*  @endcode
			*/
			int32_t   OnDemux(const char * data, size_t size, std::list<SampleBuf> & list);


			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 获取编解码器ID（Get Codec ID）
			*  
			*  该方法用于获取音频的编解码器ID。编解码器ID标识了音频的编码格式。
			*  
			*  编解码器ID说明：
			*  - 2: MP3格式
			*  - 10: AAC格式
			*  
			*  @return 返回音频编解码器ID。2表示MP3，10表示AAC，0表示未知
			*  @note 编解码器ID从Audio Tag Header的sound_format字段提取
			*  
			*  使用示例：
			*  @code
			*  int32_t codec_id = demux.GetCodecId();
			*  if (codec_id == 2) {
			*      // MP3格式
			*  } else if (codec_id == 10) {
			*      // AAC格式
			*  }
			*  @endcode
			*/
			int32_t   GetCodecId() const {
				return sound_format_;
			}

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 获取AAC对象类型（Get AAC Object Type）
			*  
			*  该方法用于获取AAC音频的对象类型。AAC对象类型标识了AAC编码的配置，
			*  如AAC-LC、HE-AAC等。
			*  
			*  AAC对象类型说明：
			*  - 2: AAC-LC（Low Complexity）
			*  - 5: HE-AAC（High Efficiency AAC）
			*  - 29: HE-AAC v2
			*  
			*  @return 返回AAC对象类型。如果不是AAC格式或未解析Sequence Header，返回默认值
			*  @note AAC对象类型从AAC Sequence Header的Audio Specific Config中提取
			*  
			*  使用示例：
			*  @code
			*  AACObjectType object_type = demux.GetObjectType();
			*  // object_type 例如 AAC-LC, HE-AAC等
			*  @endcode
			*/
			AACObjectType  GetObjectType() const
			{
				return aac_object_;
			}

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 获取采样率索引（Get Sample Rate Index）
			*  
			*  该方法用于获取AAC音频的采样率索引。采样率索引用于查找实际的采样率值。
			*  
			*  采样率索引说明：
			*  - 索引值对应采样率表，如0=96000Hz, 1=88200Hz, 4=48000Hz等
			*  - 采样率索引从AAC Sequence Header的Audio Specific Config中提取
			*  
			*  @return 返回采样率索引。如果不是AAC格式或未解析Sequence Header，返回0
			*  @note 采样率索引需要映射到实际的采样率值
			*  
			*  使用示例：
			*  @code
			*  int32_t sample_rate_index = demux.GetSampleRateIndex();
			*  // sample_rate_index 例如 4 (48000Hz)
			*  @endcode
			*/
			int32_t  GetSampleRateIndex() const
			{
				return aac_sample_rate_;
			}

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 获取通道数（Get Channel Count）
			*  
			*  该方法用于获取AAC音频的通道数。通道数标识了音频的声道配置，
			*  如单声道、立体声等。
			*  
			*  通道数说明：
			*  - 1: 单声道（Mono）
			*  - 2: 立体声（Stereo）
			*  - 3: 2.1声道
			*  - 其他: 多声道配置
			*  
			*  @return 返回通道数。如果不是AAC格式或未解析Sequence Header，返回0
			*  @note 通道数从AAC Sequence Header的Audio Specific Config中提取
			*  
			*  使用示例：
			*  @code
			*  int32_t channels = demux.GetChannel();
			*  // channels 例如 2 (立体声)
			*  @endcode
			*/
			int32_t  GetChannel() const
			{
				return aac_channel_;
			}
		private:

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 解复用AAC音频（Demux AAC Audio）
			*  
			*  该方法用于解复用AAC格式的音频数据。AAC格式需要区分Sequence Header
			*  和RAW数据，Sequence Header包含编解码器配置信息，RAW数据包含音频帧。
			*  
			*  AAC数据格式（AAC Data Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Byte 0: AACPacketType                                        |
			*   |  (8 bits)                                                      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  AACPacketType values:                                        |
			*   |  0 = AAC Sequence Header -> DemuxAACSequenceHeader()          |
			*   |  1 = AAC RAW Data -> Extract RAW data                         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   :  AAC Sequence Header (if AACPacketType == 0)                 :
			*   |  or AAC RAW Data (if AACPacketType == 1)                     :
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向AAC音频数据的指针，从Byte 1开始（Byte 0是AACPacketType）
			*  @param size AAC音频数据的大小，单位为字节（不包括Byte 0）
			*  @param list 输出参数，用于存储解复用后的音频样本缓冲区列表
			*  @return 返回0表示成功，其他值表示失败
			*  @note 如果AACPacketType为0，会调用DemuxAACSequenceHeader解析配置
			*  @note 如果AACPacketType为1，直接提取RAW数据作为音频样本
			*/
			int32_t   DemuxAAC(const char *data, size_t size, std::list<SampleBuf>& list);

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 解复用MP3音频（Demux MP3 Audio）
			*  
			*  该方法用于解复用MP3格式的音频数据。MP3格式直接从Tag Data开始就是
			*  MP3 RAW数据，不需要额外的Header解析。
			*  
			*  MP3数据格式（MP3 Data Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  MP3 Frame Header (4 bytes)                                    |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  |  Sync Word(11) | Version(2) | Layer(2) | Protection(1)   | |
			*   |  |  Bitrate(4)    | Freq(2)    | Padding(1)| Private(1)      | |
			*   |  |  Channel(2)    | Mode Ext(2)| Copyright(1)| Original(1)  | |
			*   |  |  Emphasis(2)                                                 |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  MP3 Frame Data (variable length)                              |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向MP3 RAW数据的指针，从Tag Data开始
			*  @param size MP3 RAW数据的大小，单位为字节
			*  @param list 输出参数，用于存储解复用后的音频样本缓冲区列表
			*  @return 返回0表示成功，其他值表示失败
			*  @note MP3格式不需要额外的Header，直接从Tag Data开始就是MP3帧数据
			*  @note MP3帧包含帧头（4字节）和帧数据（可变长度）
			*/
			int32_t    DemuxMP3(const char * data, size_t size, std::list<SampleBuf>& list);

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 解复用AAC Sequence Header（Demux AAC Sequence Header）
			*  
			*  该方法用于解析AAC Sequence Header。AAC Sequence Header包含Audio Specific
			*  Config（ASC），其中包含音频对象类型、采样率索引、通道配置等信息。
			*  
			*  AAC Sequence Header格式（AAC Sequence Header Format）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  Audio Specific Config (ASC)                                  |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   |  |  audioObjectType(5) | samplingFrequencyIndex(4) |          | |
			*   |  |  channelConfiguration(4) | frameLengthFlag(1) | ...      | |
			*   |  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  Audio Specific Config (ASC) 详细格式：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  audioObjectType    | samplingFrequencyIndex |channelConfig  | |
			*   |  (5 bits)           |  (4 bits)              |  (4 bits)     | |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  frameLengthFlag | dependsOnCoreCoder | extensionFlag | ...  |
			*   |  (1 bit)         |  (1 bit)           |  (1 bit)      |      |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  audioObjectType values:                                       |
			*   |  2 = AAC-LC (Low Complexity)                                   |
			*   |  5 = HE-AAC (High Efficiency AAC)                              |
			*   |  29 = HE-AAC v2                                                |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @param data 指向AAC Sequence Header数据的指针，包含Audio Specific Config
			*  @param size AAC Sequence Header数据的大小，通常为2字节
			*  @return 返回0表示成功，其他值表示失败
			*  @note 该方法会解析ASC并更新aac_object_、aac_sample_rate_、aac_channel_等成员变量
			*  @note 解析成功后，会设置aac_ok_为true，表示已收到AAC Sequence Header
			*  
			*  使用示例：
			*  @code
			*  // 在DemuxAAC中自动调用
			*  demux.DemuxAACSequenceHeader(asc_data, 2);
			*  @endcode
			*/
			int32_t    DemuxAACSequenceHeader(const char * data, size_t size);
		private:


			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 音频编码格式（Audio Codec Format）
			*  
			*  该成员变量用于存储音频的编码格式。编码格式标识了音频的编码类型，
			*  如MP3、AAC等。
			*  
			*  编码格式值说明（Codec Format Values）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  sound_format (4 bits)                                        |
			*   |  Values:                                                      |
			*   |  2 = MP3 format                                               |
			*   |  10 = AAC format                                              |
			*   |  0 = Unknown format                                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0，表示尚未识别音频格式
			*  @note 编码格式从Audio Tag Header的sound_format字段提取（4位）
			*  @note 2表示MP3，10表示AAC
			*/
			// 音频编码格式   2:  mp3, 10: aac编码
			int32_t sound_format_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 采样率（Sample Rate）
			*  
			*  该成员变量用于存储音频的采样率。采样率标识了音频的采样频率，
			*  如44.1KHz、48KHz等。
			*  
			*  采样率索引说明（Sample Rate Index）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  sound_rate (2 bits)                                          |
			*   |  Values:                                                      |
			*   |  0 = 5.5KHz                                                   |
			*   |  1 = 11KHz                                                    |
			*   |  2 = 22KHz                                                    |
			*   |  3 = 44KHz                                                    |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0，表示采样率未设置
			*  @note 采样率从Audio Tag Header的sound_rate字段提取（2位）
			*/
			//  采样率  448000
			int32_t sound_rate_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 采样深度（Sample Size）
			*  
			*  该成员变量用于存储音频的采样深度。采样深度标识了每个样本的位数，
			*  如8位、16位等。
			*  
			*  采样深度说明（Sample Size）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  sound_size (1 bit)                                           |
			*   |  Values:                                                      |
			*   |  0 = 8-bit samples                                            |
			*   |  1 = 16-bit samples                                           |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0，表示采样深度未设置
			*  @note 采样深度从Audio Tag Header的sound_size字段提取（1位）
			*/
			// 采样深度  16 
			int32_t  sound_size_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief 通道类型（Channel Type）
			*  
			*  该成员变量用于存储音频的通道类型。通道类型标识了音频的声道配置，
			*  如单声道、立体声等。
			*  
			*  通道类型说明（Channel Type）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  sound_type (1 bit)                                           |
			*   |  Values:                                                      |
			*   |  0 = Mono (单声道)                                            |
			*   |  1 = Stereo (立体声)                                          |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0，表示通道类型未设置
			*  @note 通道类型从Audio Tag Header的sound_type字段提取（1位）
			*/
			// 通道数
			int32_t sound_type_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AAC对象类型（AAC Object Type）
			*  
			*  该成员变量用于存储AAC音频的对象类型。AAC对象类型标识了AAC编码的配置，
			*  如AAC-LC、HE-AAC等。
			*  
			*  AAC对象类型说明（AAC Object Type）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  audioObjectType (5 bits)                                      |
			*   |  Values:                                                      |
			*   |  2 = AAC-LC (Low Complexity)                                  |
			*   |  5 = HE-AAC (High Efficiency AAC)                             |
			*   |  29 = HE-AAC v2                                               |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note AAC对象类型从AAC Sequence Header的Audio Specific Config中提取
			*  @note 仅当AAC格式且已解析Sequence Header时有效
			*/
			// aac 的序列头的消息
			AACObjectType aac_object_;

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AAC采样率索引（AAC Sample Rate Index）
			*  
			*  该成员变量用于存储AAC音频的采样率索引。采样率索引用于查找实际的采样率值。
			*  
			*  采样率索引说明（Sample Rate Index）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  samplingFrequencyIndex (4 bits)                              |
			*   |  Common values:                                                |
			*   |  3 = 48000Hz                                                   |
			*   |  4 = 44100Hz                                                   |
			*   |  5 = 32000Hz                                                   |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0，表示采样率索引未设置
			*  @note 采样率索引从AAC Sequence Header的Audio Specific Config中提取
			*  @note 仅当AAC格式且已解析Sequence Header时有效
			*/
			int32_t aac_sample_rate_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AAC通道数（AAC Channel Count）
			*  
			*  该成员变量用于存储AAC音频的通道数。通道数标识了音频的声道配置。
			*  
			*  通道配置说明（Channel Configuration）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  channelConfiguration (4 bits)                                 |
			*   |  Values:                                                      |
			*   |  1 = Mono (单声道)                                            |
			*   |  2 = Stereo (立体声)                                          |
			*   |  3 = 2.1 channels                                             |
			*   |  4 = 3.1 channels                                             |
			*   |  5 = 5.1 channels                                             |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为0，表示通道数未设置
			*  @note 通道数从AAC Sequence Header的Audio Specific Config中提取
			*  @note 仅当AAC格式且已解析Sequence Header时有效
			*/
			int32_t aac_channel_{ 0 };

			/**
			*  @author chensong
			*  @date 2025-04-29
			*  @brief AAC Sequence Header接收标志（AAC Sequence Header Received Flag）
			*  
			*  该成员变量用于标识是否已收到AAC Sequence Header。AAC Sequence Header
			*  包含编解码器配置信息，在解析AAC RAW数据前必须收到。
			*  
			*  状态说明（Status Description）：
			*  
			*    0                   1                   2                   3
			*    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*   |  aac_ok_ (boolean)                                            |
			*   |  false: 尚未收到AAC Sequence Header                            |
			*   |  true:  已收到AAC Sequence Header，可以解析AAC RAW数据         |
			*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*  
			*  @note 初始值为false，表示尚未收到AAC Sequence Header
			*  @note 当收到AAC Sequence Header（AACPacketType=0）时，设置为true
			*  @note 仅当aac_ok_为true时，才能正确解析AAC RAW数据
			*/
			// 有没有收到aac序列头数据
			bool  aac_ok_{ false };
		};


	}
}


#endif // 