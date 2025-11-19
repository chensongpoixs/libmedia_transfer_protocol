

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

purpose:		video encoder
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
#include "libmedia_transfer_protocol/libmpeg/cvideo_encoder.h"
#include "libmedia_transfer_protocol/libmpeg/cpsi_writer.h"
 
#include <list>
#include <cstring>
#include <string>
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"


namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{
		namespace
		{
			uint8_t * get_start_payload(uint8_t * pkt)
			{
				if (pkt[3] & 0X20)
				{
					return pkt + 5 +pkt[4];
				}
				return pkt + 4;
			}

			int WritePcr(uint8_t *buf, int64_t pcr)
			{
#if 0
				int64_t pcr_low = pcr % 300, pcr_high = pcr / 300;

				*buf++ = pcr_high >> 25;
				*buf++ = pcr_high >> 17;
				*buf++ = pcr_high >> 9;
				*buf++ = pcr_high >> 1;
				*buf++ = pcr_high << 7 | pcr_low >> 8 | 0x7e;
				*buf++ = pcr_low;
				return 6;
#endif // 
				int64_t pcrv = (0) & 0x1ff;
				pcrv |= (0x3f << 9) & 0x7E00;
				pcrv |= ((pcr) << 15) & 0xFFFFFFFF8000LL;

				char *pp = (char*)&pcrv;
				*buf++ = pp[5];
				*buf++ = pp[4];
				*buf++ = pp[3];
				*buf++ = pp[2];
				*buf++ = pp[1];
				*buf++ = pp[0];

				return 6;
			}

			void  WritePts(uint8_t *q, int fourbits, int64_t pts)
			{
				int val;

				val = fourbits << 4 | (((pts >> 30) & 0x07) << 1) | 1;
				*q++ = val;
				val = (((pts >> 15) & 0x7fff) << 1) | 1;
				*q++ = val >> 8;
				*q++ = val;
				val = (((pts) & 0x7fff) << 1) | 1;
				*q++ = val >> 8;
				*q++ = val;
			}
		}


		//VideoEncoder
		int32_t   VideoEncoder::EncodeVideo(StreamWriter *writer, bool key, std::shared_ptr<Packet>& data, int64_t dts)
		{
			std::list<SampleBuf> list;
			//int ret;
			auto ret = demux_.OnDemux(data->Data(), data->PacketSize (), list);
			if (ret == -1)
			{
				LIBMPEG_LOG_T_F(LS_ERROR) << "video demux  error. ";
				return -1;
			}
 
			// 检查nalu 
			//if (TsTool::IsCodecHeader(data))
			//{
			//	return 0;
			//}

 
			writer->AppendTimestamp(dts);


			dts *= 90;

			if (demux_.GetCodecID() == kVideoCodecIDAVC)
			{
				return EncodeAvc(writer, list, key, dts);
			} 
			return 0;
		}

		void VideoEncoder::SetPid(uint16_t pid)
		{
			pid_ = pid;
		}
		void VideoEncoder::SetStreamType(TsStreamType type)
		{
			type_ = type;
		}

	 
		int32_t   VideoEncoder::EncodeAvc(StreamWriter*writer, std::list<SampleBuf>& sample_list, bool key, int64_t dts)
		{
			int32_t total_size = 0;
			std::list<SampleBuf> result;
			bool startcode_inserted = true;
			if (!demux_.HasAud())
			{
				static uint8_t   default_aud_nalu[] = { 0X09, 0XF0 };
				static SampleBuf  default_aud_buf((const char *)&default_aud_nalu[0], 2);
				total_size +=AvcInsertStartCode(result, startcode_inserted);
				result.push_back(default_aud_buf);
				total_size += 2;
			}
			for (auto const & l : sample_list)
			{
				if (l.size <= 0/* || l.size > 0X1FFF*/)
				{
					LIBMPEG_LOG_T_F(LS_ERROR) << "invalid AVC frame length. ";
					continue;
				}

				auto bytes = l.addr;
				NaluType type = (NaluType)(bytes[0] & 0X1f);
				if (type == kNaluTypeIDR && !demux_.HasSpsPps() && 
					(writer->GetSPS() != demux_.GetSPS() ||
						writer->GetPPS() != demux_.GetPPS()||
						!writer->GetSpsPpsAppended() ))
				{
					auto const &sps = demux_.GetSPS();
					if (!sps.empty())
					{
						total_size += AvcInsertStartCode(result, startcode_inserted);
						result.push_back({ (const char *)sps.data(), sps.size() });
						total_size += sps.size();
						writer->SetSPS({ (const char *)sps.data(), sps.size() });
					}
					else
					{
						LIBMPEG_LOG_T_F(LS_ERROR)  << "no sps !!!";
					} 
					auto const &pps = demux_.GetPPS();
					if (!pps.empty())
					{
						total_size += AvcInsertStartCode(result, startcode_inserted);
						result.push_back({ (const char *)pps.data(), pps.size() });
						total_size += pps.size();
						writer->SetSPS({ (const char *)pps.data(), pps.size() });
					}
					else
					{
						LIBMPEG_LOG_T_F(LS_ERROR) << "no pps !!!";
					}

					sps_pps_appended_ = true;
					writer->SetSpsPpsAppended(true);
				}
				total_size += AvcInsertStartCode(result, startcode_inserted);
				result.push_back({(const char *) l.addr, l.size });
				total_size += l.size;
				
			}
			int64_t  pts = dts;
			if (demux_.GetCST() > 0)
			{
				pts = dts + demux_.GetCST()*90;
			}
			return WriteVideoPes(writer, result, total_size, pts, dts, key);
		}
		int32_t   VideoEncoder::AvcInsertStartCode(std::list<SampleBuf> & sample_list, bool &startcode_inserted)
		{
			if (startcode_inserted)
			{
				static uint8_t   default_start_nalu[] = { 0X00, 0X00, 0X01 };
				static SampleBuf  default_start_buf((const char *)&default_start_nalu[0], 3);
				//total_size += AvcInsertStartCode(result);
				sample_list.emplace_back(default_start_buf);
				return 3;

			} 


			{
				static uint8_t   default_start_nalu[] = { 0X00, 0X00,0X00, 0X01 };
				static SampleBuf  default_start_buf((const char *)&default_start_nalu[0], 4);
			//	total_size += AvcInsertStartCode(result);
				sample_list.emplace_back(default_start_buf);
				startcode_inserted_ = true;
				startcode_inserted = true;
			}
			return 4;
			 
		}
		int32_t   VideoEncoder::WriteVideoPes(StreamWriter * writer, std::list<SampleBuf> & result, int32_t payload_size,
			int64_t pts, int64_t  dts, int32_t key)
		{
			uint8_t   buf[188], *q;
			int32_t   val = 0;


			bool is_start = true;

			while (payload_size > 0 && !result.empty())
			{
				memset(buf, 0X00, 188);
				q = buf;

				*q++ = 0X47;
				val = pid_ >> 8;
				if (is_start)
				{
					val |= 0X40;
				}
				*q++ = val;
				*q++ = pid_;
				cc_ = (cc_ + 1) & 0XF;
				*q++ = 0X10 | cc_;
				if (is_start)
				{
					if (key)
					{
						//
						buf[3] |= 0X20;
						buf[4] = 1;
						buf[5] = 0X10;
						q = get_start_payload(buf);

						auto size = WritePcr(q, pts);

						buf[4] += size;
						q = get_start_payload(buf);
					}


					*q++ = 0X00;
					*q++ = 0X00;
					*q++ = 0X01;

					// 视频 id 是 0XE0
					*q++ = 0XE0;
					auto header_len = 5;
					uint8_t flags = 0X02;
					if (pts != dts)
					{
						header_len += 5;
						flags = 0X03;
					}
					int32_t   len = payload_size + header_len + 3;
					if (len > 0XFFFF)
					{
						len = 0;
					}
					*q++ = len >> 8;
					*q++ = len;
					*q++ = 0X80;
					*q++ = flags << 6;
					*q++ = header_len;

					if (flags == 0X02)
					{
						WritePts(q, 0X02, pts);
						q += 5;
					}
					else if (flags == 0X03)
					{
						 WritePts(q, 0X03, pts);
						q += 5;
						 WritePts(q, 0X01, dts);
						q += 5;
					}
					


					is_start = false;
				}


				int32_t   header_len = q - buf;
				int32_t    len = 188 - header_len;
				if (len > payload_size)
				{
					len = payload_size;
				}

				int32_t  stuffing = 188 - header_len - len;
				if (stuffing > 0)
				{
					if (buf[3] & 0X20)
					{
						int32_t   af_len = buf[4] + 1;
						memmove(buf + 4 + af_len + stuffing, buf + 4 + af_len, header_len - (4 + af_len));
						buf[4] += stuffing;
						memset(buf + 4 + af_len, 0XFF, stuffing);
					}
					else
					{
						memmove(buf + 4 + stuffing, buf + 4, header_len - (4));
						buf[3] |= 0X20;
						buf[4] = stuffing - 1;
						if (stuffing > 2)
						{
							buf[5] = 0X00;
							memset(buf + 6, 0XFF, stuffing - 2);
						}
					}
				}

				auto slen = len;
				while (slen > 0 && !result.empty())
				{
					auto   &sbuf = result.front();
					if (sbuf.size <= slen)
					{
						memcpy(buf + 188 - slen, sbuf.addr, sbuf.size);
						slen -= sbuf.size;
						result.pop_front();
					}
					else
					{
						memcpy(buf + 188 - slen, sbuf.addr, slen);
						sbuf.addr += slen;;
						sbuf.size -= slen;
						slen = 0;
						break;
						//result.pop_front();
					}
				}

				payload_size -= len;
				writer->Write(buf, 188);

			}


			return 0;
		}
	}
}