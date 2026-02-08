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


#ifndef _LIBRTP_RTP_PACKET_H_
#define _LIBRTP_RTP_PACKET_H_

#include <cstdint> 
#include <string>
#include <map>
#include <memory>
#include "libmedia_transfer_protocol/librtc/rtc_utils.h"
//#include "libmedia_transfer_protocol/librtp/rtp_payload_descriptor.h"
 
#ifdef _MSC_VER
#include <WinSock2.h>

#define MS_LITTLE_ENDIAN  (1)

#elif defined(__GNUC__) ||defined(__APPLE__)
 
 #include <arpa/inet.h>
#define MS_BIG_ENDIAN (1)
#else
// ������֧�ֵı�������Ҫ�Լ�ʵ���������
#error unexpected c complier (msc/gcc), Need to implement this method for demangle
 
#endif


namespace libmedia_transfer_protocol
{
    namespace librtp {

	
		class RtpPayloadDescriptorHeader;

		class RtpPacket
		{

		public:
			/* Struct for RTP header. */
			struct RtpHeader
			{
#if defined(MS_LITTLE_ENDIAN)
				uint8_t csrcCount : 4;
				uint8_t extension : 1;
				uint8_t padding : 1;
				uint8_t version : 2;
				uint8_t payloadType : 7;
				uint8_t marker : 1;
#elif defined(MS_BIG_ENDIAN)
				uint8_t version : 2;
				uint8_t padding : 1;
				uint8_t extension : 1;
				uint8_t csrcCount : 4;
				uint8_t marker : 1;
				uint8_t payloadType : 7;
#endif
				uint16_t sequenceNumber;
				uint32_t timestamp;
				uint32_t ssrc;
			};


			/* Struct for RTP header extension. */
			struct HeaderExtension
			{
				uint16_t id;
				uint16_t length; // Size of value in multiples of 4 bytes.
				uint8_t value[1];
			};
			/* Struct for One-Byte extension. */
			struct OneByteExtension
			{
#if defined(MS_LITTLE_ENDIAN)
				uint8_t len : 4;
				uint8_t id : 4;
#elif defined(MS_BIG_ENDIAN)
				uint8_t id : 4;
				uint8_t len : 4;
#endif
				uint8_t value[1];
			};
			/* Struct for Two-Bytes extension. */
			struct TwoBytesExtension
			{
				uint8_t id : 8;
				uint8_t len : 8;
				uint8_t value[1];
			};

			/* Struct for replacing and setting header extensions. */
			struct GenericExtension
			{
				GenericExtension(uint8_t id, uint8_t len, uint8_t* value) : id(id), len(len), value(value) {};

				uint8_t id : 8;
				uint8_t len : 8;
				uint8_t* value;
			};


			/* Struct with frame-marking information. */

	// Frame Marking.
	//
	// Meta-information about an RTP stream outside the encrypted media payload,
	// useful for an RTP switch to do codec-agnostic selective forwarding
	// without decrypting the payload.
	//
	// For non-scalable streams:
	//    0                   1
	//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
	//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//   |  ID   | L = 0 |S|E|I|D|0 0 0 0|
	//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//
	// For scalable streams:
	//    0                   1                   2                   3
	//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	//   |  ID   | L = 2 |S|E|I|D|B| TID |      LID      |   TL0PICIDX   |
	//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	///////////////////////////////////////////////////////////////////////////////////////
	//	�����ֶν������£�
	//	
	//		S				: Start of Frame(1 bit) ��һ֡�Ŀ�ʼ�� 
	//		E				: End of Frame(1 bit) ��һ֡�Ľ����� 
	//		I				: Independent Frame(1 bit) ���Ƿ��Ƕ����ɽ����һ֡����264��IDR�� 
	//		D				: Discardable Frame(1 bit) �����Ա�������֡����ʱ��ֲ���high layer������������֡��Ȼ���������롣 
	//		B				: Base Layer Sync(1 bit) ����TID��Ϊ0�����Ͷ�֪����ֻ֡����base layer������1��������0
	//	    TID				: Temporal ID(3 bits) ��ʱ��ֲ��еĲ�ID��0��ʾbase layer���̵�extension��ʽ�У���0.
	//		LID				: Layer ID(8 bits) ��������������ֲ�Ĳ�ID��0��ʶbase layer�� 
	//		TL0PICIDX		: Temporal Layer 0 Picture Index(8 bits) ��ʱ��0��picture index��
	//						  ��Ϊֻ��8��bit��������ѭ�������ġ�TID = 0 LID = 0��ʶ��֡ID��TID != 0����LID != 0��ʶ��֡������һ��picture index��
	//						  ���������ʱ��ֲ���߼���δ֪������ֶο���ʡ�ԡ�
	//	
	///////////////////////////////////////////////////////////////////////////////////////

			struct FrameMarking
			{
#if defined(MS_LITTLE_ENDIAN)
				uint8_t tid : 3;
				uint8_t base : 1;
				uint8_t discardable : 1; //���Ա�������֡����ʱ��ֲ���high layer������������֡��Ȼ���������롣 
				uint8_t independent : 1; // �Ƿ��Ƕ����ɽ����һ֡����264��IDR�� 
				uint8_t end : 1;
				uint8_t start : 1;
#elif defined(MS_BIG_ENDIAN)
				uint8_t start : 1;
				uint8_t end : 1;
				uint8_t independent : 1;
				uint8_t discardable : 1;
				uint8_t base : 1;
				uint8_t tid : 3;
#endif
				uint8_t lid;
				uint8_t tl0picidx;
			};

		public:
			static bool IsRtp(const uint8_t* data, size_t len)
			{
				// NOTE: RtcpPacket::IsRtcp() must always be called before this method.

				auto header = const_cast<RtpHeader*>(reinterpret_cast<const RtpHeader*>(data));

				// clang-format off
				return (
					(len >= sizeof(RtpHeader)) &&
					// DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
					(data[0] > 127 && data[0] < 192) &&
					// RTP Version must be 2.
					(header->version == 2)
					);
				// clang-format on
			}

			static RtpPacket* Parse(const uint8_t* data, size_t len);
		public:

			RtpPacket(
				RtpHeader* rtp_header,
				HeaderExtension* headerExtension,
				const uint8_t* payload,
				size_t payloadLength,
				uint8_t payloadPadding,
				size_t size);

			~RtpPacket();


		public:
			void RtpPacketInfo() const;



		public:
			const uint8_t* GetData() const
			{
				return (const uint8_t*)this->header;
			}

			size_t GetSize() const
			{
				return this->size;
			}

			uint8_t GetPayloadType() const
			{
				return this->header->payloadType;
			}

			void SetPayloadType(uint8_t payloadType)
			{
				this->header->payloadType = payloadType;
			}

			bool HasMarker() const
			{
				return this->header->marker;
			}

			void SetMarker(bool marker)
			{
				this->header->marker = marker;
			}

			void SetPayloadPaddingFlag(bool flag)
			{
				this->header->padding = flag;
			}

			uint16_t GetSequenceNumber() const
			{
				return uint16_t{ ntohs(this->header->sequenceNumber) };
			}

			void SetSequenceNumber(uint16_t seq)
			{
				this->header->sequenceNumber = uint16_t{ htons(seq) };
			}

			uint32_t GetTimestamp() const
			{
				return uint32_t{ ntohl(this->header->timestamp) };
			}

			void SetTimestamp(uint32_t timestamp)
			{
				this->header->timestamp = uint32_t{ htonl(timestamp) };
			}

			uint32_t GetSsrc() const
			{
				return uint32_t{ ntohl(this->header->ssrc) };
			}

			void SetSsrc(uint32_t ssrc)
			{
				this->header->ssrc = uint32_t{ htonl(ssrc) };
			}
			uint8_t* GetPayload() const
			{
				return this->payloadLength != 0u ? this->payload : nullptr;
			}
			size_t GetPayloadLength() const
			{
				return this->payloadLength;
			}


			uint8_t GetSpatialLayer() const;

			uint8_t GetTemporalLayer() const;

			void SetPayloadDescriptorHandler(RtpPayloadDescriptorHeader* payloadDescriptorHandler);
		public:

			bool ReadMid(std::string& mid) const
			{
				uint8_t extenLen;
				uint8_t* extenValue = GetExtension(this->midExtensionId, extenLen);

				if (!extenValue || extenLen == 0u)
					return false;

				mid.assign(reinterpret_cast<const char*>(extenValue), static_cast<size_t>(extenLen));

				return true;
			}

			bool ReadRid(std::string& rid) const
			{
				// First try with the RID id then with the Repaired RID id.
				uint8_t extenLen;
				uint8_t* extenValue = GetExtension(this->ridExtensionId, extenLen);

				if (extenValue && extenLen > 0u)
				{
					rid.assign(reinterpret_cast<const char*>(extenValue), static_cast<size_t>(extenLen));

					return true;
				}

				extenValue = GetExtension(this->rridExtensionId, extenLen);

				if (extenValue && extenLen > 0u)
				{
					rid.assign(reinterpret_cast<const char*>(extenValue), static_cast<size_t>(extenLen));

					return true;
				}

				return false;
			}

			bool ReadTransportWideCc01(uint16_t& wideSeqNumber) const
			{
				uint8_t extenLen;
				uint8_t* extenValue = GetExtension(this->transportWideCc01ExtensionId, extenLen);

				if (!extenValue || extenLen != 2u)
					return false;

				wideSeqNumber = librtc::Byte::Get2Bytes(extenValue, 0);

				return true;
			}

			bool ReadSsrcAudioLevel(uint8_t& volume, bool& voice) const
			{
				uint8_t extenLen;
				uint8_t* extenValue = GetExtension(this->ssrcAudioLevelExtensionId, extenLen);

				if (!extenValue || extenLen != 1u)
					return false;

				volume = librtc::Byte::Get1Byte(extenValue, 0);
				voice = (volume & (1 << 7)) ? true : false;
				volume &= ~(1 << 7);

				return true;
			}

			bool ReadVideoOrientation(bool& camera, bool& flip, uint16_t& rotation) const
			{
				uint8_t extenLen;
				uint8_t* extenValue = GetExtension(this->videoOrientationExtensionId, extenLen);

				if (!extenValue || extenLen != 1u)
					return false;

				uint8_t cvoByte = librtc::Byte::Get1Byte(extenValue, 0);
				uint8_t cameraValue = ((cvoByte & 0b00001000) >> 3);
				uint8_t flipValue = ((cvoByte & 0b00000100) >> 2);
				uint8_t rotationValue = (cvoByte & 0b00000011);

				camera = cameraValue ? true : false;
				flip = flipValue ? true : false;

				// Using counter clockwise values.
				switch (rotationValue)
				{
				case 3:
					rotation = 270;
					break;
				case 2:
					rotation = 180;
					break;
				case 1:
					rotation = 90;
					break;
				default:
					rotation = 0;
				}

				return true;
			}

			bool ReadFrameMarking(RtpPacket::FrameMarking** frameMarking, uint8_t& length) const
			{
				uint8_t extenLen;
				uint8_t* extenValue = GetExtension(this->frameMarkingExtensionId, extenLen);

				// NOTE: Remove this once framemarking draft becomes RFC.
				if (!extenValue)
				{
					extenValue = GetExtension(this->frameMarking07ExtensionId, extenLen);
				}
				if (!extenValue || extenLen > 3u)
				{
					return false;
				}

				*frameMarking = reinterpret_cast<RtpPacket::FrameMarking*>(extenValue);
				length = extenLen;

				return true;
			}
		public:


			uint8_t* GetExtension(uint8_t id, uint8_t& len) const
			{
				len = 0u;

				if (id == 0u)
				{
					return nullptr;
				}
				else if (HasOneByteExtensions())
				{
					auto it = this->mapOneByteExtensions.find(id);

					if (it == this->mapOneByteExtensions.end())
						return nullptr;

					auto* extension = it->second;

					// In One-Byte extensions value length 0 means 1.
					len = extension->len + 1;

					return extension->value;
				}
				else if (HasTwoBytesExtensions())
				{
					auto it = this->mapTwoBytesExtensions.find(id);

					if (it == this->mapTwoBytesExtensions.end())
						return nullptr;

					auto* extension = it->second;

					len = extension->len;

					// In Two-Byte extensions value length may be zero. If so, return nullptr.
					if (extension->len == 0u)
						return nullptr;

					return extension->value;
				}
				else
				{
					return nullptr;
				}
			}

			 
			bool HasHeaderExtension() const
			{
				return (this->headerExtension ? true : false);
			}
			uint16_t GetHeaderExtensionId() const
			{
				if (!this->headerExtension)
					return 0u;

				return uint16_t{ ntohs(this->headerExtension->id) };
			}
			size_t GetHeaderExtensionLength() const
			{
				if (!this->headerExtension)
					return 0u;

				return static_cast<size_t>(ntohs(this->headerExtension->length) * 4);
			}
			bool HasOneByteExtensions() const
			{
				return GetHeaderExtensionId() == 0xBEDE;
			}

			bool HasTwoBytesExtensions() const
			{
				return (GetHeaderExtensionId() & 0b1111111111110000) == 0b0001000000000000;
			}
		private:
			void ParseExtensions();
		private:
			// Passed by argument.
			RtpHeader* header{ nullptr };
			uint8_t* csrcList{ nullptr };
			HeaderExtension* headerExtension{ nullptr };   // rtp����չͷ 
			std::map<uint8_t, OneByteExtension*> mapOneByteExtensions;
			std::map<uint8_t, TwoBytesExtension*> mapTwoBytesExtensions;
			uint8_t midExtensionId{ 0u };
			uint8_t ridExtensionId{ 0u };
			uint8_t rridExtensionId{ 0u };
			uint8_t absSendTimeExtensionId{ 0u };
			uint8_t transportWideCc01ExtensionId{ 0u };
			uint8_t frameMarking07ExtensionId{ 0u }; // NOTE: Remove once RFC.
			uint8_t frameMarkingExtensionId{ 0u };
			uint8_t ssrcAudioLevelExtensionId{ 0u };
			uint8_t videoOrientationExtensionId{ 0u };
			uint8_t* payload{ nullptr };
			size_t payloadLength{ 0u };
			uint8_t payloadPadding{ 0u };
			size_t size{ 0u }; // Full size of the packet in bytes.
			// Codecs  Opus��H264��H265��AV1
			 std::unique_ptr<libmedia_transfer_protocol::librtp::RtpPayloadDescriptorHeader> payloadDescriptorHandler;


		};


    }
}

#endif // _LIBRTP_RTP_PACKET_H_
