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
				   date:  2025-11-11



 ******************************************************************************/

#include "libmedia_transfer_protocol/librtc/rtc_usr_sctp.h"

#include <mutex>
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
namespace libmedia_transfer_protocol
{
	namespace librtc
	{

		namespace {
			static std::mutex globalSyncMutex;
			static size_t globalInstances = 0;
		

			/* Static methods for usrsctp global callbacks. */

			inline static int onSendSctpData(void* addr, void* data, size_t len, uint8_t /*tos*/, uint8_t /*setDf*/)
			{
				auto* sctpAssociation = RtcUsrSctp::RetrieveSctpAssociation(reinterpret_cast<uintptr_t>(addr));

				if (!sctpAssociation)
				{
					//MS_WARN_TAG(sctp, "no SctpAssociation found");
					LIBRTC_LOG_F(LS_WARNING) << "no SctpAssociation found ";
					return -1;
				}

				sctpAssociation->OnUsrSctpSendSctpData(data, len);

				// NOTE: Must not free data, usrsctp lib does it.

				return 0;
			}

			// Static method for printing usrsctp debug.
			inline static void sctpDebug(const char* format, ...)
			{
				char buffer[10000] = {0};
				va_list ap;

				va_start(ap, format);
				vsprintf(buffer, format, ap);

				// Remove the artificial carriage return set by usrsctp.
				buffer[std::strlen(buffer) - 1] = '\0';

				//MS_DEBUG_TAG(sctp, "%s", buffer);
				LIBRTC_LOG(LS_INFO) << "sctp: " << buffer;


				va_end(ap);
			}
		}
		uint64_t RtcUsrSctp::num_sctp_associations_{ 0u };
		uintptr_t RtcUsrSctp::next_sctp_association_id_{ 0u };
		std::unordered_map<uintptr_t, SctpAssociation*> RtcUsrSctp::mapid_sctp_association_;






		bool RtcUsrSctp::Init()
		{
			{
				std::lock_guard<std::mutex> lock(globalSyncMutex);

				if (globalInstances == 0)
				{
					usrsctp_init_nothreads(0, onSendSctpData, sctpDebug);

					// Disable explicit congestion notifications (ecn).
					usrsctp_sysctl_set_sctp_ecn_enable(0);

#ifdef SCTP_DEBUG
					usrsctp_sysctl_set_sctp_debug_on(SCTP_DEBUG_ALL);
#endif
				}

				++globalInstances;
			}
			return true;
		}
		void RtcUsrSctp::Destroy()
		{
			{
				std::lock_guard<std::mutex> lock(globalSyncMutex);
				--globalInstances;

				if (globalInstances == 0)
				{
					usrsctp_finish();

					num_sctp_associations_ = 0u;
					next_sctp_association_id_ = 0u;

					RtcUsrSctp::mapid_sctp_association_.clear();
				}
			}
		}
		uintptr_t RtcUsrSctp::GetNextSctpAssociationId()
		{
			std::lock_guard<std::mutex> lock(globalSyncMutex);

			// NOTE: usrsctp_connect() fails with a value of 0.
			if (RtcUsrSctp::next_sctp_association_id_ == 0u)
			{
				++RtcUsrSctp::next_sctp_association_id_;
			}

			// In case we've wrapped around and need to find an empty spot from a removed
			// SctpAssociation. Assumes we'll never be full.
			while (RtcUsrSctp::mapid_sctp_association_.find(RtcUsrSctp::next_sctp_association_id_) !=
				RtcUsrSctp::mapid_sctp_association_.end())
			{
				++RtcUsrSctp::next_sctp_association_id_;

				if (RtcUsrSctp::next_sctp_association_id_ == 0u)
				{
					++RtcUsrSctp::next_sctp_association_id_;
				}
			}

			return RtcUsrSctp::next_sctp_association_id_++;
		}
		void RtcUsrSctp::RegisterSctpAssociation(SctpAssociation* sctpAssociation)
		{
			std::lock_guard<std::mutex> lock(globalSyncMutex);

			//MS_ASSERT(DepUsrSCTP::checker != nullptr, "Checker not created");

			auto it = RtcUsrSctp::mapid_sctp_association_.find(sctpAssociation->id);

			//MS_ASSERT(
			//	it == RtcUsrSctp::mapid_sctp_association_.end(),
			//	"the id of the SctpAssociation is already in the map");

			RtcUsrSctp::mapid_sctp_association_[sctpAssociation->id] = sctpAssociation;

			if (++RtcUsrSctp::num_sctp_associations_ == 1u)
			{
				//RtcUsrSctp::checker->Start();
			}
		}
		void RtcUsrSctp::DeregisterSctpAssociation(SctpAssociation* sctpAssociation)
		{
			std::lock_guard<std::mutex> lock(globalSyncMutex);

			//MS_ASSERT(DepUsrSCTP::checker != nullptr, "Checker not created");

			auto found = RtcUsrSctp::mapid_sctp_association_.erase(sctpAssociation->id);

			//MS_ASSERT(found > 0, "SctpAssociation not found");
			//MS_ASSERT(DepUsrSCTP::numSctpAssociations > 0u, "numSctpAssociations was not higher than 0");

			if (--RtcUsrSctp::num_sctp_associations_ == 0u)
			{
				//RtcUsrSctp::checker->Stop();
			}
		}
		SctpAssociation* RtcUsrSctp::RetrieveSctpAssociation(uintptr_t id)
		{
			std::lock_guard<std::mutex> lock(globalSyncMutex);

			auto it = RtcUsrSctp::mapid_sctp_association_.find(id);

			if (it == RtcUsrSctp::mapid_sctp_association_.end())
				return nullptr;

			return it->second;
		}
	}
}