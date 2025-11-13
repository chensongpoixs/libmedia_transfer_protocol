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


#ifndef _LIBRTP_RTX_STREAM_H_
#define _LIBRTP_RTX_STREAM_H_

#include <cstdint>
#include "libmedia_transfer_protocol/librtp/rtp_codec_mime_type.h"
#include <string>
#include "libmedia_transfer_protocol/librtp/rtp_packet.h"

namespace libmedia_transfer_protocol
{
    namespace librtp {

        class RtxStream
        {
        public:
            struct Params
            {
                uint32_t ssrc{ 0 };
                uint8_t payload_type{ 0 };
                RtpCodecMimeType mime_type;
                uint32_t clock_rate{ 0 };
                std::string rrid;
                std::string cname;
            };
        public:
            RtxStream(const Params& params) : params_(params){};
            ~RtxStream() = default;
        public:

            uint32_t GetSsrc() const
            {
                return this->params_.ssrc;
            }
            uint8_t GetPayloadType() const
            {
                return this->params_.payload_type;
            }
            const RtpCodecMimeType& GetMimeType() const
            {
                return this->params_.mime_type;
            }
            uint32_t GetClockRate() const
            {
                return this->params_.clock_rate;
            }
            const std::string& GetRrid() const
            {
                return this->params_.rrid;
            }
            const std::string& GetCname() const
            {
                return this->params_.cname;
            }
            uint8_t GetFractionLost() const
            {
                return this->fraction_lost_;
            }
            float GetLossPercentage() const
            {
                return static_cast<float>(this->fraction_lost_) * 100 / 256;
            }
            size_t GetPacketsDiscarded() const
            {
                return this->packets_discarded_;
            }
            
        protected:
            bool UpdateSeq(RtpPacket* packet);
            
            uint32_t GetExpectedPackets() const
            {
                return (this->cycles_ + this->max_seq_) - this->base_seq_ + 1;
            }


            
        public:
            void _InitSeq(uint16_t seq);


        protected:
            Params    params_;
            // Others.
        //   https://tools.ietf.org/html/rfc3550#appendix-A.1 stuff.
            uint16_t max_seq_{ 0u };      // Highest seq. number seen.
            uint32_t cycles_{ 0u };      // Shifted count of seq. number cycles.
            uint32_t base_seq_{ 0u };     // Base seq number.
            uint32_t bad_seq_{ 0u };      // Last 'bad' seq number + 1.
            uint32_t max_packet_ts_{ 0u }; // Highest timestamp seen.
            uint64_t max_packet_ms_{ 0u }; // When the packet with highest timestammp was seen.
            uint32_t packets_lost_{ 0u };
            uint8_t fraction_lost_{ 0u };
            size_t packets_discarded_{ 0u };
            size_t packets_count_{ 0u };

        private:
            bool     started_{ false };

            // Fields for generating Receiver Reports.
            uint32_t expected_prior_{ 0u };
            uint32_t received_prior_{ 0u };
            uint32_t lastsr_timestamp_{ 0u };
            uint64_t lastsr_received_{ 0u };
            uint32_t reported_packet_lost_{ 0u };


        };

    }
}

#endif // _LIBRTP_RTX_STREAM_H_