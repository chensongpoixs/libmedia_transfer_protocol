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


#ifndef _C_LIBRTC_RTC_USR_SCTP__H_
#define _C_LIBRTC_RTC_USR_SCTP__H_

#include <cstdint>
#include "usrsctp.h"
#include <unordered_map>
#include "libmedia_transfer_protocol/librtc/sctp_association.h"


namespace libmedia_transfer_protocol {

	namespace librtc
	{

		class RtcUsrSctp
		{
		public:
			  RtcUsrSctp() = default;
			  ~RtcUsrSctp() = default;
		public:


			static bool Init();
			static void Destroy();
			static uintptr_t GetNextSctpAssociationId();
			static void RegisterSctpAssociation( SctpAssociation* sctpAssociation);
			static void DeregisterSctpAssociation( SctpAssociation* sctpAssociation);
			static  SctpAssociation* RetrieveSctpAssociation(uintptr_t id);


		public:
			static uint64_t num_sctp_associations_;
			static uintptr_t next_sctp_association_id_;
			static std::unordered_map<uintptr_t,  SctpAssociation*> mapid_sctp_association_;
		};
	}
}

#endif // _C_LIBRTC_RTC_USR_SCTP__H_