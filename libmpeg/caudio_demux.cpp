/***********************************************************************************************
created: 		2025-04-29

author:			chensong

purpose:		http_parser
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
#include "libmedia_transfer_protocol/libmpeg/caudio_demux.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
#include "libmedia_transfer_protocol/libmpeg/NalBitStream.h"
namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{

		int32_t  AudioDemux::OnDemux(const char * data, size_t size, std::list<SampleBuf> & list)
		{
			if (size < 1)
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << "param error . size < 1 failed !!!";
			}

			/*    
			0     1     2    3    4     5    6    7     8    9   10   11  12
			+-------------------------------------------------------+
			|        format       |   rate   |size| type|           |
			+-------------------------------------------------------+
			          4bit            2bit    1bit  1bit

			*/
			sound_format_ = (AudioCodecID)((*data>>4)&0X0F);
			sound_rate_ = (SoundRate)((*data&0XC0)>> 2);
			sound_size_ = (SoundSize)((*data &0X02)>>1);
			sound_type_ = (SoundChannel)(*data &0X01);
			/*DEMUX_DEBUG << "audio format:" << sound_format_
				<< ", rate: " << sound_rate_
				<< ", size:" << sound_size_
				<< ", type:" << sound_type_;*/

			if (sound_format_ == kAudioCodecIDMP3)
			{
				return DemuxMP3(data, size, list);
			}
			else if (sound_format_ == kAudioCodecIDAAC)
			{
				return DemuxAAC(data, size, list);
			}
			else
			{
				LIBMUXER_LOG_T_F(LS_ERROR) << " not support code id: " << sound_format_<<", failed !!!";
			}

			return -1;
		}
		int32_t  AudioDemux::DemuxAAC(const char *data, size_t size, std::list<SampleBuf>& list)
		{
			// 
			AACPacketType type = (AACPacketType)data[1];
			if (type == kAACPacketTypeAACSequenceHeader)
			{
				if (size - 2 > 0)
				{
					return DemuxAACSequenceHeader(data + 2, size - 2);
				}
			}
			else if (type == kAACPacketTypeAACRaw)//原始数据
			{
				//没有序列头就丢了无用的数据
				if (!aac_ok_)
				{
					return -1;
				}
				list.emplace_back(SampleBuf(data + 2, size - 2));
			}
			return 0;
		}
		int32_t  AudioDemux::DemuxMP3(const char * data, size_t size, std::list<SampleBuf>& list)
		{


			list.emplace_back(SampleBuf(data+1, size-1));
			return 0;
		}
		int32_t  AudioDemux::DemuxAACSequenceHeader(const char * data, size_t size)
		{
			if (size < 2)
			{
				LIBMUXER_LOG_T_F(LS_ERROR) << "demux aac seq header failed . size < 2 failed !!!";
				return -1;
			}
			//解析AAC序列头

			NalBitStream  stream(data, size);

			aac_object_ = (AACObjectType)stream.GetWord(4);
			aac_sample_rate_ = stream.GetWord(5);
			aac_channel_ = stream.GetWord(4);
			aac_ok_ = true;

			return 0;
		}
	}
}