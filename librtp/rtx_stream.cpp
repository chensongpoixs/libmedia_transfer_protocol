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

******************************************************************************/
#include "libmedia_transfer_protocol/librtp/rtx_stream.h"
#include "libmedia_transfer_protocol/librtc/rtc_log.h"
#include "rtc_base/time_utils.h"
namespace libmedia_transfer_protocol
{
    namespace librtp
    {
        namespace {
            /* Static. */

            static const uint16_t MaxDropout{ 3000 };
            static const uint16_t MaxMisorder{ 1500 };
            static const uint32_t RtpSeqMod{ 1 << 16 };
        }
        bool RtxStream::UpdateSeq(RtpPacket* packet)
        {
			uint16_t seq = packet->GetSequenceNumber();
			uint16_t udelta = seq - this->max_seq_;

			// If the new packet sequence number is greater than the max seen but not
			// "so much bigger", accept it.
			// NOTE: udelta also handles the case of a new cycle, this is:
			//    maxSeq:65536, seq:0 => udelta:1
			if (udelta < MaxDropout)
			{
				// In order, with permissible gap.
				if (seq < this->max_seq_)
				{
					// Sequence number wrapped: count another 64K cycle.
					this->cycles_ += RtpSeqMod;
				}

				this->max_seq_ = seq;
			}
			// Too old packet received (older than the allowed misorder).
			// Or to new packet (more than acceptable dropout).
			else if (udelta <= RtpSeqMod - MaxMisorder)
			{
				// The sequence number made a very large jump. If two sequential packets
				// arrive, accept the latter.
				if (seq == this->bad_seq_)
				{
					// Two sequential packets. Assume that the other side restarted without
					// telling us so just re-sync (i.e., pretend this was the first packet).
					MS_WARN_TAG(
						rtx,
						"too bad sequence number, re-syncing RTP [ssrc:%" PRIu32 ", seq:%" PRIu16 "]",
						packet->GetSsrc(),
						packet->GetSequenceNumber());

					_InitSeq(seq);

					this->max_packet_ts_ = packet->GetTimestamp();
					this->max_packet_ms_ = rtc::SystemTimeMillis();//DepLibUV::GetTimeMs();
				}
				else
				{
					MS_WARN_TAG(
						rtx,
						"bad sequence number, ignoring packet [ssrc:%" PRIu32 ", seq:%" PRIu16 "]",
						packet->GetSsrc(),
						packet->GetSequenceNumber());

					this->bad_seq_ = (seq + 1) & (RtpSeqMod - 1);

					// Packet discarded due to late or early arriving.
					this->packets_discarded_++;

					return false;
				}
			}
			// Acceptable misorder.
			else
			{
				// Do nothing.
			}

			return true;
        }
        void RtxStream::_InitSeq(uint16_t seq)
        {
            // Initialize/reset RTP counters.
            this->base_seq_ = seq;
            this->max_seq_ = seq;
            this->bad_seq_ = RtpSeqMod + 1; // So seq == badSeq is false.
        }
    }
}