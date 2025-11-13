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


#ifndef _LIBRTP_RTP_PAYLOAD_DESCRIPTOR_H_
#define _LIBRTP_RTP_PAYLOAD_DESCRIPTOR_H_

#include <cstdint> 
#include <string>
#include <map>
#include <memory>
#include "libmedia_transfer_protocol/librtc/rtc_utils.h"
 
#include "libmedia_transfer_protocol/librtp/rtp_packet.h"

namespace libmedia_transfer_protocol
{
	namespace librtp {


		// Codec payload descriptor.
		struct PayloadDescriptor
		{
			virtual ~PayloadDescriptor() = default;
			virtual std::string Info() const = 0;
		};
		// Encoding context used by PayloadDescriptorHandler to properly rewrite the
		// PayloadDescriptor.
		class EncodingContext
		{
		public:
			struct Params
			{
				uint8_t spatialLayers{ 1u };
				uint8_t temporalLayers{ 1u };
				bool ksvc{ false };
			};

		public:
			explicit EncodingContext(EncodingContext::Params& params) : params(params)
			{ }
			virtual ~EncodingContext() = default;

		public:
			uint8_t GetSpatialLayers() const
			{
				return this->params.spatialLayers;
			}
			uint8_t GetTemporalLayers() const
			{
				return this->params.temporalLayers;
			}
			bool IsKSvc() const
			{
				return this->params.ksvc;
			}
			int16_t GetTargetSpatialLayer() const
			{
				return this->targetSpatialLayer;
			}
			int16_t GetTargetTemporalLayer() const
			{
				return this->targetTemporalLayer;
			}
			int16_t GetCurrentSpatialLayer() const
			{
				return this->currentSpatialLayer;
			}
			int16_t GetCurrentTemporalLayer() const
			{
				return this->currentTemporalLayer;
			}
			void SetTargetSpatialLayer(int16_t spatialLayer)
			{
				this->targetSpatialLayer = spatialLayer;
			}
			void SetTargetTemporalLayer(int16_t temporalLayer)
			{
				this->targetTemporalLayer = temporalLayer;
			}
			void SetCurrentSpatialLayer(int16_t spatialLayer)
			{
				this->currentSpatialLayer = spatialLayer;
			}
			void SetCurrentTemporalLayer(int16_t temporalLayer)
			{
				this->currentTemporalLayer = temporalLayer;
			}
			virtual void SyncRequired() = 0;

		private:
			Params params;
			int16_t targetSpatialLayer{ -1 };
			int16_t targetTemporalLayer{ -1 };
			int16_t currentSpatialLayer{ -1 };
			int16_t currentTemporalLayer{ -1 };
		};

		class RtpPayloadDescriptorHeader
		{
		public:
			//RtpPayloadDescriptorHeader();
			virtual  ~RtpPayloadDescriptorHeader() = default;;
		public:
			virtual std::string Info() const = 0;
			virtual bool Process(EncodingContext* context, uint8_t* data, bool& marker) = 0;
			virtual void Restore(uint8_t* data) = 0;
			virtual uint8_t GetSpatialLayer() const = 0;
			virtual uint8_t GetTemporalLayer() const = 0;
			virtual bool IsKeyFrame() const = 0;
		private:

		};

		class H264Payload
		{
		public:
			struct H264PayloadDescriptor : public  PayloadDescriptor
			{
				/* Pure virtual methods inherited from RTC::Codecs::PayloadDescriptor. */
				~H264PayloadDescriptor() = default;

				std::string Info() const override;

				// Fields in frame-marking extension.
				uint8_t s : 1;          // Start of Frame.
				uint8_t e : 1;          // End of Frame.
				uint8_t i : 1;          // Independent Frame.
				uint8_t d : 1;          // Discardable Frame.
				uint8_t b : 1;          // Base Layer Sync.
				uint8_t tid{ 0 };       // Temporal layer id.
				uint8_t lid{ 0 };       // Spatial layer id.
				uint8_t tl0picidx{ 0 }; // TL0PICIDX
				// Parsed values.
				bool hasLid{ false };
				bool hasTid{ false };
				bool hasTl0picidx{ false };
				bool isKeyFrame{ false };
			};

		public:
		public:
			static H264Payload::H264PayloadDescriptor* Parse(
				const uint8_t* data,
				size_t len,
				libmedia_transfer_protocol::librtp::RtpPacket::FrameMarking* frameMarking = nullptr,
				uint8_t frameMarkingLen = 0);
			static void ProcessRtpPacket(libmedia_transfer_protocol::librtp::RtpPacket* packet);
		
		public:
			class H264PayloadDescriptorHandler : public  RtpPayloadDescriptorHeader
			{
			public:
				explicit H264PayloadDescriptorHandler(H264PayloadDescriptor* payloadDescriptor);
				virtual ~H264PayloadDescriptorHandler() = default;

			public:
				virtual std::string Info() const override
				{
					return this->payload_descriptor_->Info();
				}
				virtual bool Process(EncodingContext* encodingContext, uint8_t* data, bool& marker) override;
				virtual void Restore(uint8_t* data) override;
				virtual uint8_t GetSpatialLayer() const override
				{
					return 0u;
				}
				virtual uint8_t GetTemporalLayer() const override
				{
					return this->payload_descriptor_->tid;
				}
				virtual bool IsKeyFrame() const override
				{
					return this->payload_descriptor_->isKeyFrame;
				}

			private:
				std::unique_ptr<H264PayloadDescriptor> payload_descriptor_;
			};
		private:
			/// ;/ std::unique_ptr<H264PayloadDescriptor>   payload_descriptor_;
		};

	}
}

#endif // _LIBRTP_RTP_PAYLOAD_DESCRIPTOR_H_