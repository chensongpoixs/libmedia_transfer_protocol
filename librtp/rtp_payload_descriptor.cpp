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
/*****************************************************************************
				  Author: chensong
				  date:  2025-11-13

				TODO: 2025-11-13 chensong   rtcp
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
******************************************************************************/
#include "libmedia_transfer_protocol/librtp/rtp_payload_descriptor.h"

#include <sstream>
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
#include "libmedia_transfer_protocol/librtc/rtc_log.h"

namespace libmedia_transfer_protocol
{
	namespace librtp
	{
		/* Instance methods. */

		std::string H264Payload::H264PayloadDescriptor::Info() const
		{
			 
			std::stringstream cmd;
			cmd << "<PayloadDescriptor>";
			cmd << 
				"  s:" << (int32_t)s << "|e:" <<e<< "|i:" <<i<< "|d:" <<d<< "|b:" << b;
			if (this->hasTid)
			{
				cmd << "  tid        : " <<  this->tid;
			}
			if (this->hasLid)
			{
				cmd << "  lid        : " <<  this->lid;
			}
			if (this->hasTl0picidx)
			{
				cmd << "  tl0picidx  : "  <<  this->tl0picidx;
			}
			cmd << "  isKeyFrame : "<<  (this->isKeyFrame ? "true" : "false");
			cmd << "</PayloadDescriptor>";
			//LIBRTP_LOG(LS_INFO) << cmd.str();
			return cmd.str();
		}



		H264Payload::H264PayloadDescriptor* H264Payload::Parse(
			const uint8_t* data,
			size_t len,
			libmedia_transfer_protocol::librtp::RtpPacket::FrameMarking* frameMarking,
			uint8_t frameMarkingLen)
		{
			if (len < 2)
				return nullptr;

			std::unique_ptr<H264PayloadDescriptor> payloadDescriptor(new H264PayloadDescriptor());

			// Use frame-marking.
			if (frameMarking)
			{
				// Read fields.
				payloadDescriptor->s = frameMarking->start;
				payloadDescriptor->e = frameMarking->end;
				payloadDescriptor->i = frameMarking->independent;
				payloadDescriptor->d = frameMarking->discardable;
				payloadDescriptor->b = frameMarking->base;
				payloadDescriptor->tid = frameMarking->tid;

				payloadDescriptor->hasTid = true;

				if (frameMarkingLen >= 2)
				{
					payloadDescriptor->hasLid = true;
					payloadDescriptor->lid = frameMarking->lid;
				}

				if (frameMarkingLen == 3)
				{
					payloadDescriptor->hasTl0picidx = true;
					payloadDescriptor->tl0picidx = frameMarking->tl0picidx;
				}

				// Detect key frame.
				if (frameMarking->start && frameMarking->independent)
					payloadDescriptor->isKeyFrame = true;
			}

			// NOTE: Unfortunately libwebrtc produces wrong Frame-Marking (without i=1 in
			// keyframes) when it uses H264 hardware encoder (at least in Mac):
			//   https://bugs.chromium.org/p/webrtc/issues/detail?id=10746
			//
			// As a temporal workaround, always do payload parsing to detect keyframes if
			// there is no frame-marking or if there is but keyframe was not detected above.
			if (!frameMarking || !payloadDescriptor->isKeyFrame)
			{
				uint8_t nal = *data & 0x1F;

				switch (nal)
				{
					// Single NAL unit packet.
					// IDR (instantaneous decoding picture).
				case 7:
				{
					payloadDescriptor->isKeyFrame = true;

					break;
				}

				// Aggreation packet.
				// STAP-A.
				case 24:
				{
					size_t offset{ 1 };

					len -= 1;

					// Iterate NAL units.
					while (len >= 3)
					{
						auto naluSize = librtc::Byte::Get2Bytes(data, offset);
						uint8_t subnal = *(data + offset + sizeof(naluSize)) & 0x1F;

						if (subnal == 7)
						{
							payloadDescriptor->isKeyFrame = true;

							break;
						}

						// Check if there is room for the indicated NAL unit size.
						if (len < (naluSize + sizeof(naluSize)))
							break;

						offset += naluSize + sizeof(naluSize);
						len -= naluSize + sizeof(naluSize);
					}

					break;
				}

				// Aggreation packet.
				// FU-A, FU-B.
				case 28:
				case 29:
				{
					uint8_t subnal = *(data + 1) & 0x1F;
					uint8_t startBit = *(data + 1) & 0x80;

					if (subnal == 7 && startBit == 128)
					{
						payloadDescriptor->isKeyFrame = true;
					}

					break;
				}
				}
			}

			return payloadDescriptor.release();

		}
		void H264Payload::ProcessRtpPacket(libmedia_transfer_protocol::librtp::RtpPacket* packet)
		{
			auto* data = packet->GetPayload();
			auto len = packet->GetPayloadLength();
			libmedia_transfer_protocol::librtp::RtpPacket::FrameMarking* frameMarking{ nullptr };
			uint8_t frameMarkingLen{ 0 };

			// Read frame-marking.
			packet->ReadFrameMarking(&frameMarking, frameMarkingLen);

			H264PayloadDescriptor* payloadDescriptor = H264Payload::Parse(data, len, frameMarking, frameMarkingLen);

			if (!payloadDescriptor)
			{
				return;
			}

			auto* payloadDescriptorHandler = new H264PayloadDescriptorHandler(payloadDescriptor);

			packet->SetPayloadDescriptorHandler(payloadDescriptorHandler);
		}
		H264Payload::H264PayloadDescriptorHandler::H264PayloadDescriptorHandler(H264PayloadDescriptor* payloadDescriptor)
		{
			payload_descriptor_.reset(payloadDescriptor);
		}
		bool H264Payload::H264PayloadDescriptorHandler::Process(
			EncodingContext* encodingContext,
			uint8_t* data, bool& marker)
		{
			auto* context = static_cast<EncodingContext*>(encodingContext);

			MS_ASSERT(context->GetTargetTemporalLayer() >= 0, "target temporal layer cannot be -1");

			// Check if the payload should contain temporal layer info.
			if (context->GetTemporalLayers() > 1 && !this->payload_descriptor_->hasTid)
			{
				LIBRTP_LOG_T_F(LS_WARNING) << "stream is supposed to have >1 temporal layers but does not have tid field";

				//MS_WARN_DEV("stream is supposed to have >1 temporal layers but does not have tid field");
			}

			// clang-format off
			if (
				this->payload_descriptor_->hasTid &&
				this->payload_descriptor_->tid > context->GetTargetTemporalLayer()
				)
				// clang-format on
			{
				return false;
			}
			// Upgrade required. Drop current packet if base flag is not set.
			// NOTE: This is possible once this bug in libwebrtc has been fixed:
			//   https://github.com/versatica/mediasoup/issues/306
			//
			// clang-format off
			else if (
				this->payload_descriptor_->hasTid &&
				this->payload_descriptor_->tid > context->GetCurrentTemporalLayer() &&
				!this->payload_descriptor_->b
				)
				// clang-format on
			{
				return false;
			}

			// Update/fix current temporal layer.
			// clang-format off
			if (
				this->payload_descriptor_->hasTid &&
				this->payload_descriptor_->tid > context->GetCurrentTemporalLayer()
				)
				// clang-format on
			{
				context->SetCurrentTemporalLayer(this->payload_descriptor_->tid);
			}
			else if (!this->payload_descriptor_->hasTid)
			{
				context->SetCurrentTemporalLayer(0);
			}

			if (context->GetCurrentTemporalLayer() > context->GetTargetTemporalLayer())
			{
				context->SetCurrentTemporalLayer(context->GetTargetTemporalLayer());
			}

			return true;
		}
		void H264Payload::H264PayloadDescriptorHandler::Restore(uint8_t* data)
		{
		}
}
}