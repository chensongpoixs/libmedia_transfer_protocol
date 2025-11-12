///******************************************************************************
// *  Copyright (c) 2025 The CRTC project authors . All Rights Reserved.
// *
// *  Please visit https://chensongpoixs.github.io for detail
// *
// *  Use of this source code is governed by a BSD-style license
// *  that can be found in the LICENSE file in the root of the source
// *  tree. An additional intellectual property rights grant can be found
// *  in the file PATENTS.  All contributing project authors may
// *  be found in the AUTHORS file in the root of the source tree.
// ******************************************************************************/
// /*****************************************************************************
//				   Author: chensong
//				   date:  2025-11-11
//
//
//
// ******************************************************************************/
//
//
//#ifndef _C_LIBRTC_SCTP_ASSOCIATION__H_
//#define _C_LIBRTC_SCTP_ASSOCIATION__H_
//
//#include <cstdint>
//#include "usrsctp.h"
//#include <unordered_map>
//
//
//#if 0
//namespace libmedia_transfer_protocol {
//
//	namespace librtc
//	{
//
//		class SctpAssociation
//		{
//		public:
//			enum class SctpState
//			{
//				NEW = 1,
//				CONNECTING,
//				CONNECTED,
//				FAILED,
//				CLOSED
//			};
//		private:
//			enum class StreamDirection
//			{
//				INCOMING = 1,
//				OUTGOING
//			};
//		public:
//			SctpAssociation(bool isDataChannel)  ;
//			~SctpAssociation()  ;
//
//
//		public:
//			static bool IsSctp(const uint8_t* data, size_t len)
//			{
//				// clang-format off
//				return (
//					(len >= 12) &&
//					// Must have Source Port Number and Destination Port Number set to 5000 (hack).
//					//(Utils::Byte::Get2Bytes(data, 0) == 5000) &&
//					//(Utils::Byte::Get2Bytes(data, 2) == 5000)
//					(uint16_t{ data[  1] } | uint16_t{ data[0] } << 8) == 5000 &&
//					(uint16_t{ data[3] } | uint16_t{ data[2] } << 8) == 5000
//					);
//				// clang-format on
//			}
//		public:
//
//			void TransportConnected();
//
//			SctpState GetState() const
//			{
//				return this->state;
//			}
//			size_t GetSctpBufferedAmount() const
//			{
//				return this->sctpBufferedAmount;
//			}
//			void ProcessSctpData(const uint8_t* data, size_t len);
//			//void SendSctpMessage(
//			//	RTC::DataConsumer* dataConsumer,
//			//	uint32_t ppid,
//			//	const uint8_t* msg,
//			//	size_t len,
//			//	onQueuedCallback* cb = nullptr);
//			//void HandleDataConsumer(RTC::DataConsumer* dataConsumer);
//			//void DataProducerClosed(RTC::DataProducer* dataProducer);
//			//void DataConsumerClosed(RTC::DataConsumer* dataConsumer);
//
//		private:
//			void ResetSctpStream(uint16_t streamId, StreamDirection);
//			void AddOutgoingStreams(bool force = false);
//
//			/* Callbacks fired by usrsctp events. */
//		public:
//			void OnUsrSctpSendSctpData(void* buffer, size_t len);
//			void OnUsrSctpReceiveSctpData(
//				uint16_t streamId, uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len);
//			void OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len);
//			void OnUsrSctpSentData(uint32_t freeBuffer);
//
//		public:
//			uintptr_t id{ 0u };
//		public:
//			uint16_t os{ 1024u };
//			uint16_t mis{ 1024u };
//			size_t maxSctpMessageSize{ 262144u };
//			size_t sctpSendBufferSize{ 262144u };
//			size_t sctpBufferedAmount{ 0u };
//			bool isDataChannel{ false };
//			// Allocated by this.
//			uint8_t* messageBuffer{ nullptr };
//			// Others.
//			SctpState state{ SctpState::NEW };
//			struct socket* socket{ nullptr };
//			uint16_t desiredOs{ 0u };
//			size_t messageBufferLen{ 0u };
//			uint16_t lastSsnReceived{ 0u }; // Valid for us since no SCTP I-DATA support.
//		};
//	}
//}
//
//
//#endif // 
//
//
//#endif //  