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
				   date:  2025-11-09

				 TODO: 2025-11-09 chensong   rtcp

 ******************************************************************************/


#ifndef _C_LIBRTCP_TWCC_CONTEXT_H_
#define _C_LIBRTCP_TWCC_CONTEXT_H_
#include <cstdio>
#include <cstdint>
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/sender_report.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/compound_packet.h"
#include <map>
namespace libmedia_transfer_protocol
{
	namespace librtcp {
		class TwccContext
		{
		public:
			TwccContext() = default;
			virtual ~TwccContext() = default;
		public:
            using onSendTwccCB = std::function<void(uint32_t ssrc, std::string fci)>;
            // 每个twcc rtcp包最多表明的rtp ext seq增量  [AUTO-TRANSLATED:530d1e35]
            // Maximum RTP ext seq increment indicated by each twcc rtcp packet
            static constexpr size_t kMaxSeqSize = 20;
            // 每个twcc rtcp包发送的最大时间间隔，单位毫秒  [AUTO-TRANSLATED:e45656da]
            // Maximum time interval for sending each twcc rtcp packet, in milliseconds
            static constexpr size_t kMaxTimeDelta = 256;

            void onRtp(uint32_t ssrc, uint16_t twcc_ext_seq, uint64_t stamp_ms);
            void setOnSendTwccCB(onSendTwccCB cb);

        private:
            void onSendTwcc(uint32_t ssrc);
            bool needSendTwcc() const;
            int checkSeqStatus(uint16_t twcc_ext_seq) const;
            void clearStatus();

        private:
            uint64_t _min_stamp = 0;
            uint64_t _max_stamp;
            std::map<uint32_t /*twcc_ext_seq*/, uint64_t/*recv time in ms*/> _rtp_recv_status;
            uint8_t _twcc_pkt_count = 0;
            onSendTwccCB _cb;
		};
	}
}

#endif // 