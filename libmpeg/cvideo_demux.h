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

		enum AVCPayloadFormat
		{
			kPayloadFormatNukonwed = 0,
			kPayloadFormatAvcc,
			kPayloadFormatAnnexB,
		};
		class VideoDemux
		{
		public:
			VideoDemux() = default;
			~VideoDemux() = default;
		public:
			int32_t    OnDemux(const char * data, size_t size, std::list<SampleBuf> & outs);
		private:
			int32_t    DemuxAVC(const char *data, size_t size, std::list<SampleBuf>& outs);
			const char * FindAnnexbNalu(const char * p, const char * end);
			int32_t   DecodeAVCNaluAnnexb(const char * data, size_t size, std::list<SampleBuf>& outs);
			int32_t   DecodeAVCNalulAvcc(const char *data, size_t size, std::list<SampleBuf>& outs);

			int32_t  DecodeAVCSeqHeader(const char * data, size_t size, std::list<SampleBuf>& outs);
			int32_t  DecodeAvcNalu(const char * data, size_t size, std::list<SampleBuf> & outs);

		public:
			void CheckNaluType(const char * data);
			bool HasIdr() const;
			bool HasAud() const;
			bool HasSpsPps() const;


			VideoCodecID GetCodecID() const
			{
				return codec_id_;
			}
			int32_t   GetCST() const
			{
				return composition_time_;
			}


			const std::string & GetSPS() const
			{
				return sps_;
			}
			const std::string & GetPPS() const
			{
				return pps_;
			}

			void Reset()
			{
				has_aud_ = false;
				has_idr_ = false;
				has_sps_pps_ = false;
			}
		private:
			//编码器id
			VideoCodecID    codec_id_;
			//编码器补偿时间
			int32_t    composition_time_{ 0 };
			// 序列头版本
			uint8_t    config_version_{ 0 };
			uint8_t     profile_{ 0 };
			uint8_t    profile_cmpa_{ 0 };
			uint8_t    level_{ 0 };

			//没有收到序列头
			bool		avc_ok_{ false };

			//nalu 长度
			int32_t      nalu_unit_length_{ 0 };

			//sps=内容
			std::string     sps_;
			// pps 内容
			std::string     pps_;
			// ypt===>  AVCC or  annexB数据类型
			AVCPayloadFormat      payload_format_{ kPayloadFormatNukonwed };

			//有没有收到 这些数据
			bool       has_aud_{ false };
			bool		has_idr_{ false };
			bool		has_sps_pps_{ false };

		};

	}
}


#endif // libmedia_transfer_protocol