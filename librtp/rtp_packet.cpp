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
#include "libmedia_transfer_protocol/librtp/rtp_packet.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
#include "libmedia_transfer_protocol/librtc/rtc_log.h"
#include "libmedia_transfer_protocol/librtp/rtp_payload_descriptor.h"


namespace libmedia_transfer_protocol
{
    namespace librtp {
		RtpPacket::RtpPacket(
			RtpHeader* rtp_header,
			HeaderExtension* headerExtension,
			const uint8_t* payload,
			size_t payloadLength,
			uint8_t payloadPadding,
			size_t size) : header(header), headerExtension(headerExtension), 
			payload(const_cast<uint8_t*>(payload)),
			payloadLength(payloadLength), payloadPadding(payloadPadding), size(size)

		{
			LIBRTP_LOG_T_F(LS_INFO);
			if (this->header->csrcCount != 0u)
			{
				this->csrcList = reinterpret_cast<uint8_t*>(header) + sizeof(rtp_header);
			}

			// Parse RFC 5285 header extension.
			ParseExtensions();
		}
		RtpPacket::~RtpPacket()
		{
			LIBRTP_LOG_T_F(LS_INFO);

		}

		RtpPacket* RtpPacket::Parse(const uint8_t* data, size_t len)
		{
			if (!RtpPacket::IsRtp(data, len))
			{
				return nullptr;
			}

			auto* ptr = const_cast<uint8_t*>(data);

			// Get the header.
			auto* header = reinterpret_cast<RtpHeader*>(ptr);

			// Inspect data after the minimum header size.
			ptr += sizeof(RtpHeader);

			// Check CSRC list.
			size_t csrcListSize{ 0u };

			if (header->csrcCount != 0u)
			{
				csrcListSize = header->csrcCount * sizeof(header->ssrc);

				// Packet size must be >= header size + CSRC list.
				if (len < (ptr - data) + csrcListSize)
				{
					MS_WARN_TAG(rtp, "not enough space for the announced CSRC list, packet discarded");

					return nullptr;
				}
				ptr += csrcListSize;
			}

			// Check header extension.
			HeaderExtension* headerExtension{ nullptr };
			size_t extensionValueSize{ 0u };

			if (header->extension == 1u)
			{
				// The header extension is at least 4 bytes.
				if (len < static_cast<size_t>(ptr - data) + 4)
				{
					MS_WARN_TAG(rtp, "not enough space for the announced header extension, packet discarded");

					return nullptr;
				}

				headerExtension = reinterpret_cast<HeaderExtension*>(ptr);

				// The header extension contains a 16-bit length field that counts the number of
				// 32-bit words in the extension, excluding the four-octet header extension.
				extensionValueSize = static_cast<size_t>(ntohs(headerExtension->length) * 4);

				// Packet size must be >= header size + CSRC list + header extension size.
				if (len < (ptr - data) + 4 + extensionValueSize)
				{
					MS_WARN_TAG(
						rtp, "not enough space for the announced header extension value, packet discarded");

					return nullptr;
				}
				ptr += 4 + extensionValueSize;
			}

			// Get payload.
			uint8_t* payload = ptr;
			size_t payloadLength = len - (ptr - data);
			uint8_t payloadPadding{ 0 };

			MS_ASSERT(len >= static_cast<size_t>(ptr - data), "payload has negative size");

			// Check padding field.
			if (header->padding != 0u)
			{
				// Must be at least a single payload byte.
				if (payloadLength == 0)
				{
					MS_WARN_TAG(rtp, "padding bit is set but no space for a padding byte, packet discarded");

					return nullptr;
				}

				payloadPadding = data[len - 1];
				if (payloadPadding == 0)
				{
					MS_WARN_TAG(rtp, "padding byte cannot be 0, packet discarded");

					return nullptr;
				}

				if (payloadLength < size_t{ payloadPadding })
				{
					MS_WARN_TAG(
						rtp,
						"number of padding octets is greater than available space for payload, packet "
						"discarded");

					return nullptr;
				}
				payloadLength -= size_t{ payloadPadding };
			}

			MS_ASSERT(
				len == sizeof(RtpHeader) + csrcListSize + (headerExtension ? 4 + extensionValueSize : 0) +
				payloadLength + size_t{ payloadPadding },
				"packet's computed size does not match received size");

			auto* packet =
				new RtpPacket(header, headerExtension, payload, payloadLength, payloadPadding, len);

			return packet;
		}
		void RtpPacket::RtpPacketInfo() const
		{
			std::stringstream cmd;
			cmd <<  "<RtpPacket>" ;
			cmd <<  "  padding           : " <<  (this->header->padding ? "true" : "false");
			if (HasHeaderExtension())
			{
				cmd << "  header extension  : id:"<< GetHeaderExtensionId() 
					<<", length: "  <<GetHeaderExtensionLength();
			}
			if (HasOneByteExtensions())
			{
				cmd << "  RFC5285 ext style : One-Byte Header";
			}
			if (HasTwoBytesExtensions())
			{
				cmd << "  RFC5285 ext style : Two-Bytes Header";
			}
			if (HasOneByteExtensions() || HasTwoBytesExtensions())
			{
				std::vector<std::string> extIds;
				std::ostringstream extIdsStream;

				if (HasOneByteExtensions())
				{
					extIds.reserve(this->mapOneByteExtensions.size());

					for (const auto& kv : this->mapOneByteExtensions)
					{
						extIds.push_back(std::to_string(kv.first));
					}
				}
				else
				{
					extIds.reserve(this->mapTwoBytesExtensions.size());

					for (const auto& kv : this->mapTwoBytesExtensions)
					{
						extIds.push_back(std::to_string(kv.first));
					}
				}

				if (!extIds.empty())
				{
					std::copy(
						extIds.begin(), extIds.end() - 1, std::ostream_iterator<std::string>(extIdsStream, ","));
					extIdsStream << extIds.back();

					cmd <<"  RFC5285 ext ids   : " <<  extIdsStream.str();
				}
			}
			if (this->midExtensionId != 0u)
			{
				std::string mid;

				if (ReadMid(mid))
				{
					cmd << "  mid               : extId:" << this->midExtensionId
						<< ", value:'" << mid << "'";
				}
			}
			if (this->ridExtensionId != 0u)
			{
				std::string rid;

				if (ReadRid(rid))
				{
					cmd << "  rid               : extId:"<< this->ridExtensionId 
						<< ", value:'"<< rid <<"'";//, , rid.c_str());
				}
			}
			if (this->rridExtensionId != 0u)
			{
				std::string rid;

				if (ReadRid(rid))
				{
					cmd << "  rrid              : extId:"<< this->rridExtensionId
						<<", value:'"<< rid <<"'";
				}
			}
			if (this->absSendTimeExtensionId != 0u)
			{
				cmd << "  absSendTime       : extId:" <<  this->absSendTimeExtensionId;
			}
			if (this->transportWideCc01ExtensionId != 0u)
			{
				uint16_t wideSeqNumber;

				if (ReadTransportWideCc01(wideSeqNumber))
				{
					
					cmd << "  transportWideCc01 : extId:" << transportWideCc01ExtensionId
						<< ", value:" << wideSeqNumber; 
				}
			}
			// Remove once it becomes RFC.
			if (this->frameMarking07ExtensionId != 0u)
			{
				cmd << "  frameMarking07    : extId:" <<  this->frameMarking07ExtensionId;
			}
			if (this->frameMarkingExtensionId != 0u)
			{
				cmd << "  frameMarking      : extId:" <<  this->frameMarkingExtensionId;
			}
			if (this->ssrcAudioLevelExtensionId != 0u)
			{
				uint8_t volume;
				bool voice;

				if (ReadSsrcAudioLevel(volume, voice))
				{
					
					cmd << "  ssrcAudioLevel    : extId:" << ssrcAudioLevelExtensionId
						<< ", volume:" << volume << ", voice: " << (voice ? "true" : "false");
					 
				}
			}
			if (this->videoOrientationExtensionId != 0u)
			{
				bool camera;
				bool flip;
				uint16_t rotation;

				if (ReadVideoOrientation(camera, flip, rotation))
				{
					cmd << "  videoOrientation  : extId:" << videoOrientationExtensionId
						<< ", camera:" << (camera ? "true" : "false")
						<< ", flip:" << (flip ? "true" : "false")
						<< ", rotation:" << rotation;
					 
				}
			}
			cmd << "  csrc count       : " <<  this->header->csrcCount;
			cmd <<"  marker            : " << ( HasMarker() ? "true" : "false");
			cmd <<"  payload type      : "  <<  GetPayloadType();
			cmd <<"  sequence number   : " <<  GetSequenceNumber();
			cmd <<"  timestamp         : " <<  GetTimestamp();
			cmd <<"  ssrc              : " <<  GetSsrc();
			cmd <<"  payload size      : "<< GetPayloadLength() <<" bytes" ;
			if (this->header->padding != 0u)
			{
				 cmd << "  padding size      : " << payloadPadding << " bytes";
			}
			cmd << "  packet size       : "<< GetSize() <<" bytes";
			cmd << "  spatial layer     : " <<  GetSpatialLayer();
			cmd << "  temporal layer    : " <<  GetTemporalLayer();
			if (payloadDescriptorHandler)
			{
				cmd << payloadDescriptorHandler->Info();
			}
			cmd << "</RtpPacket>" ;
		}
		void RtpPacket::ParseExtensions()
		{
			// Parse One-Byte header extension.
			if (HasOneByteExtensions())
			{
				// Clear the One-Byte extension elements map.
				this->mapOneByteExtensions.clear();

				uint8_t* extensionStart = reinterpret_cast<uint8_t*>(this->headerExtension) + 4;
				uint8_t* extensionEnd = extensionStart + GetHeaderExtensionLength();
				uint8_t* ptr = extensionStart;

				// One-Byte extensions cannot have length 0.
				while (ptr < extensionEnd)
				{
					uint8_t id = (*ptr & 0xF0) >> 4;
					size_t len = static_cast<size_t>(*ptr & 0x0F) + 1;

					// id=15 in One-Byte extensions means "stop parsing here".
					if (id == 15u)
						break;

					// Valid extension id.
					if (id != 0u)
					{
						if (ptr + 1 + len > extensionEnd)
						{
							MS_WARN_TAG(
								rtp, "not enough space for the announced One-Byte header extension element value");

							break;
						}

						// Store the One-Byte extension element in the map.
						this->mapOneByteExtensions[id] = reinterpret_cast<OneByteExtension*>(ptr);

						ptr += (1 + len);
					}
					// id=0 means alignment.
					else
					{
						++ptr;
					}

					// Counting padding bytes.
					while ((ptr < extensionEnd) && (*ptr == 0))
					{
						++ptr;
					}
				}
			}
			// Parse Two-Bytes header extension.
			else if (HasTwoBytesExtensions())
			{
				// Clear the Two-Bytes extension elements map.
				this->mapTwoBytesExtensions.clear();

				uint8_t* extensionStart = reinterpret_cast<uint8_t*>(this->headerExtension) + 4;
				uint8_t* extensionEnd = extensionStart + GetHeaderExtensionLength();
				uint8_t* ptr = extensionStart;

				// ptr points to the ID field (1 byte).
				// ptr+1 points to the length field (1 byte, can have value 0).

				// Two-Byte extensions can have length 0.
				while (ptr + 1 < extensionEnd)
				{
					uint8_t id = *ptr;
					uint8_t len = *(ptr + 1);

					// Valid extension id.
					if (id != 0u)
					{
						if (ptr + 2 + len > extensionEnd)
						{
							MS_WARN_TAG(
								rtp, "not enough space for the announced Two-Bytes header extension element value");

							break;
						}

						// Store the Two-Bytes extension element in the map.
						this->mapTwoBytesExtensions[id] = reinterpret_cast<TwoBytesExtension*>(ptr);

						ptr += (2 + len);
					}
					// id=0 means alignment.
					else
					{
						++ptr;
					}

					// Counting padding bytes.
					while ((ptr < extensionEnd) && (*ptr == 0))
					{
						++ptr;
					}
				}
			}
		}


		uint8_t RtpPacket::GetSpatialLayer() const
		{
			if (!this->payloadDescriptorHandler)
			{
				return 0u;
			}

			return payloadDescriptorHandler->GetSpatialLayer();
		}


		uint8_t RtpPacket::GetTemporalLayer() const
		{
			if (!this->payloadDescriptorHandler)
				return 0u;

			return this->payloadDescriptorHandler->GetTemporalLayer();
		}
		void RtpPacket::SetPayloadDescriptorHandler(RtpPayloadDescriptorHeader* payloadDescriptorHandler)
		{
			this->payloadDescriptorHandler.reset(payloadDescriptorHandler);
		}
    }
}
