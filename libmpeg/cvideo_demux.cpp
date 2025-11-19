
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
#include "libmedia_transfer_protocol/libmpeg/cvideo_demux.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
#include "libmedia_transfer_protocol/rtp_rtcp/byte_io.h"
namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{
		int32_t    VideoDemux::OnDemux(const char * data, size_t size, std::list<SampleBuf> & outs)
		{
			VideoCodecID id = (VideoCodecID)(*data & 0X0F);
			if (id != kVideoCodecIDAVC)
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << "not support  video type : " << id;
				return -1;
			}
			codec_id_ = id;
			return DemuxAVC(data, size, outs);
		}
		int32_t    VideoDemux::DemuxAVC(const char *data, size_t size, std::list<SampleBuf>& outs)
		{
			uint8_t  ftype = (*data & 0XF0) >> 4;
			if (ftype == kFrameTypeVideoInfoFrame)
			{
				// 不处理
				LIBMUXER_LOG_T_F(LS_INFO) << "igore info frame ..";
				return 0;
			}
			uint8_t avc_packet_type = data[1];
			int32_t  cst = libmedia_transfer_protocol::ByteReader<uint32_t, 3>::ReadBigEndian((const uint8_t * )(data + 2));
			//int32_t cst = BytesReader::ReadUint24T(data + 2);
			composition_time_ = cst;

			if (avc_packet_type == 0)
			{
				return DecodeAVCSeqHeader(data + 5, size - 5, outs);
			}
			else if (avc_packet_type == 1)
			{
				return DecodeAvcNalu(data + 5, size - 5, outs);
			}
			return 0;

		}
		int32_t  VideoDemux::DecodeAVCSeqHeader(const char * data, size_t size, std::list<SampleBuf>& outs)
		{
			if (size < 5)
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << " seq header size error size : " << size;
				return -1;
			}


			config_version_ = data[0];
			profile_ = data[1];
			profile_cmpa_ = data[2];
			level_ = data[3];

			nalu_unit_length_ = data[4]&0X03;

			LIBMUXER_LOG_T_F(LS_INFO) << "nalu_unit length : " << nalu_unit_length_;
			data += 5;
			size -= 5;

			// jx sps pps
			if (size < 3)
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << " seq header size error , no found sps.. failed !!!";
				return -1;
			}
			// 5 bit
			int8_t sps_num = data[0] &0X1F;

			if (sps_num != 1)
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << " more than 1  sps ... failed !!!";
				return -1;
			}
			int16_t  sps_length = libmedia_transfer_protocol::ByteReader<uint16_t >::ReadBigEndian((const uint8_t *)(data + 1));
			if (sps_length > 0 && sps_length < (size -3))
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << " found sps bytes: " << sps_length;
				sps_.assign(data + 3, sps_length);

			}
			else
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << "sps length  error sps_length : " << sps_length;
			}
			data += 3;
			size -= 3;
			data += sps_length;
			size -= sps_length;


			// 5 bit
			int8_t pps_num = data[0] & 0X1F;

			if (pps_num != 1)
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << " more than 1   pps ... failed !!!";
				return -1;
			}
			int16_t  pps_length = libmedia_transfer_protocol::ByteReader<uint16_t >::ReadBigEndian((const uint8_t *)(data + 1)); //BytesReader::ReadUint16T(data + 1);
			if (pps_length > 0 && pps_length <= (size - 3))
			{
				LIBMUXER_LOG_T_F(LS_INFO) << " found  pps bytes: " << pps_length;
				pps_.assign(data + 3, pps_length);

			}
			else
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << "pps length  error pps_length : " << pps_length;
				return -1;
			}
			return 0;
		}
		int32_t  VideoDemux::DecodeAvcNalu(const char * data, size_t size, std::list<SampleBuf> & outs)
		{
			if (payload_format_ == kPayloadFormatNukonwed)
			{
				if (!DecodeAVCNalulAvcc(data, size, outs))
				{
					payload_format_ = kPayloadFormatAvcc;
				}
				else
				{
					if (!DecodeAVCNaluAnnexb(data, size, outs))
					{
						payload_format_ = kPayloadFormatAnnexB;
					}
					else
					{
						LIBMUXER_LOG_T_F(LS_WARNING) << " payload format error , no found format : ";
							return -1;
					}
				}
			}
			else if (payload_format_ == kPayloadFormatAvcc)
			{
				return DecodeAVCNalulAvcc(data, size, outs);
			}
			else if (payload_format_ == kPayloadFormatAnnexB)
			{
				return DecodeAVCNaluAnnexb(data, size, outs);
			}
			return 0;
		}
		int32_t   VideoDemux::DecodeAVCNalulAvcc(const char *data, size_t size, std::list<SampleBuf>& outs)
		{
			// AVCC 结构

			// length + nalu

			while (size >1)
			{
				uint32_t nalu_size = 0;
				if (nalu_unit_length_ == 3)
				{
					nalu_size  = libmedia_transfer_protocol::ByteReader<uint32_t >::ReadBigEndian((const uint8_t *)(data  )); //BytesReader::ReadUint32T(data);
				}
				else if (nalu_unit_length_ == 1)
				{
					nalu_size  = libmedia_transfer_protocol::ByteReader<uint16_t >::ReadBigEndian((const uint8_t *)(data  )); //BytesReader::ReadUint16T(data);
				}
				else
				{
					nalu_size = data[0];
				}

				data += nalu_unit_length_ + 1;
				size -= (nalu_unit_length_ + 1);

				if (nalu_size > size || size <= 0)
				{
					// 长度不够了
					LIBMUXER_LOG_T_F(LS_WARNING) << " error avcc nalu bytes:" << size << "nalu size:" << nalu_size;
					return -1;
				}
				outs.emplace_back(SampleBuf(data, nalu_size));
				CheckNaluType(data);
				data += nalu_size;
				size -= nalu_size;
			}
			return 0;
		}
		int32_t   VideoDemux::DecodeAVCNaluAnnexb(const char * data, size_t size, std::list<SampleBuf>& outs)
		{
			if (size < 3)
			{
				LIBMUXER_LOG_T_F(LS_WARNING) << " error annexb bytes: " << size;
				return -1;
			}

			const char * data_end = data + size;
			const char * nalu_start = data;// FindAnnexbNalu(data, data_end);


			while (nalu_start < data_end)
			{
				//找到下一个开始完整  annexb
				const char * nalu_next = FindAnnexbNalu(nalu_start + 1, data_end);
				int32_t nalu_size = nalu_next - nalu_start;

				if (nalu_size > size || size <= 0)
				{
					LIBMUXER_LOG_T_F(LS_WARNING) << "error avcc nalu bytes : " << size << ", nalu size : " << nalu_size;
					return -1;
				}

				outs.emplace_back(SampleBuf(data, nalu_size));
				CheckNaluType(nalu_start);
				data += nalu_size;
				size -= nalu_size;
				nalu_start = nalu_next;
			}
			return 0;
		}
		

		
		
		const char * VideoDemux::FindAnnexbNalu(const char * p, const char * end)
		{
			for (p += 2; p + 1 < end; p++)
			{
				if (*p == 0X01 && *(p - 1) == 0X00 && *(p - 2) == 0X00)
				{
					return p + 1;
				}
			}
			return end;
		}
	 
		void VideoDemux::CheckNaluType(const char * data)
		{
			NaluType type = (NaluType)(data[0] & 0x1F);
			if (type == kNaluTypeIDR)
			{
				has_idr_ = true;
			}
			else if (type == kNaluTypeAccessUnitDelimiter)
			{
				has_aud_ = true;
			}
			else if (type == kNaluTypeSPS || type == kNaluTypePPS)
			{
				has_sps_pps_ = true;
			}

		}
		bool VideoDemux::HasIdr() const
		{
			return has_idr_;
		}
		bool VideoDemux::HasAud() const
		{
			return has_aud_;
		}
		bool VideoDemux::HasSpsPps() const
		{
			return has_sps_pps_;
		}
	}
}