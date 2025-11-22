
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
created: 		2025-04-08

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
#include "cpsi_writer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/byte_io.h"
#include "rtc_base/crc32.h"
namespace libmedia_transfer_protocol
{
	namespace libmpeg
	{
		
		

		void PSIWriter::SetVersion(uint8_t v)
		{
			version_ = v;
		}
		int32_t PSIWriter::WriteSection(StreamWriter* w, int32_t id
			, int32_t sec_num, int32_t last_sec_num, uint8_t * buf, int32_t len)
		{
			uint8_t section[1024] = {0}, *p;
			int32_t  total_len = len + 3 + 5 + 4;

			p = section;

			*p++ = table_id_;
			libmedia_transfer_protocol::ByteWriter<uint16_t>::WriteLittleEndian((uint8_t *)p, (5 + 4 + len) | 0Xb000);
			 
			p += 2;
			libmedia_transfer_protocol::ByteWriter<uint16_t>::WriteLittleEndian( p, id);
			p += 2;

			*p++ = 0XC1 | (version_ <<1);
			*p++ = sec_num;
			*p++ = last_sec_num;

			memcpy(p, buf, len); 
			p += len;


			uint32_t crc32 = rtc::ComputeCrc32(section, total_len - 4); 

			libmedia_transfer_protocol::ByteWriter<uint32_t>::WriteLittleEndian( p, crc32 );
			//p += sizeof(uint32_t);
			PushSection(w, section, total_len);
			return 0;
		}

		void PSIWriter::PushSection(StreamWriter*w, uint8_t * buf, size_t len)
		{

			// TS packet
			uint8_t   packet[188], *q;
			uint8_t * p = buf;
			bool first = false;

			while (len > 0)
			{
				q = packet;
				first = (p == buf);;
				*q++ = 0X47;

				auto b = pid_ >> 8;
				if (first)
				{
					b |= 0X40;
				}
				*q++ = b;
				*q ++= pid_;
				cc_ = (cc_ + 1) & 0XF;
				*q++ = 0X10 | cc_;
				if (first)
				{
					*q++ = 0;
				}


				// 负载部分
				auto len1 = 188 - (q-packet);
				if (len1 > len)
				{
					len1 = len;
				}
				memcpy(q, p, len1);
				q += len1;
				auto left = 188 - (q - packet);
				if (left > 0)
				{
					memset(q, 0XFF, left);
				}
				w->Write(packet, 188);
				p += len1;
				len -= len1;
			}

		}


	}
}