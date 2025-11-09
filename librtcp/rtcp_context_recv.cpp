
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


 /*
  * 6.4.1 SR: Sender Report RTCP Packet

         0                   1                   2                   3
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 header |V=2|P|    RC   |   PT=SR=200   |             length            |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                         SSRC of sender                        |
        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 sender |              NTP timestamp, most significant word             |
 info   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |             NTP timestamp, least significant word             |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                         RTP timestamp                         |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                     sender's packet count                     |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                      sender's octet count                     |
        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 report |                 SSRC_1 (SSRC of first source)                 |
 block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   1    | fraction lost |       cumulative number of packets lost       |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |           extended highest sequence number received           |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                      interarrival jitter                      |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                         last SR (LSR)                         |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                   delay since last SR (DLSR)                  |
        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 report |                 SSRC_2 (SSRC of second source)                |
 block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   2    :                               ...                             :
        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
        |                  profile-specific extensions                  |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */
#include "libmedia_transfer_protocol/librtcp/rtcp_context_recv.h"


#include "rtc_base/system_time.h"
#include "rtc_base/time_utils.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/receiver_report.h"
#include <sys/types.h>
#include <WinSock2.h>

namespace libmedia_transfer_protocol
{
	namespace librtcp
	{

        void RtcpContextRecv::onRtp(
            uint16_t seq, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t sample_rate, size_t bytes) {
                {
                    // 接收者才做复杂的统计运算  [AUTO-TRANSLATED:853c68e0]
                    // The receiver performs complex statistical calculations
                    auto sys_stamp = rtc::SystemTimeMillis();
                    if (_last_rtp_sys_stamp) {
                        // 计算时间戳抖动值  [AUTO-TRANSLATED:cd3571b4]
                        // Calculate the timestamp jitter value
                        double diff = double(
                            (int64_t(sys_stamp) - int64_t(_last_rtp_sys_stamp)) * (sample_rate / double(1000.0))
                            - (int64_t(stamp) - int64_t(_last_rtp_stamp)));
                        if (diff < 0) {
                            diff = -diff;
                        }
                        // 抖动单位为采样次数  [AUTO-TRANSLATED:b713633a]
                        // Jitter unit is the number of samples
                        _jitter += (diff - _jitter) / 16.0;
                    }
                    else {
                        _jitter = 0;
                    }

                    if (_last_rtp_seq > 0xFF00 && seq < 0xFF && (!_seq_cycles || _packets - _last_cycle_packets > 0x1FFF)) {
                        // 上次seq大于0xFF00且本次seq小于0xFF，  [AUTO-TRANSLATED:82dd69fa]
                        // Last seq is greater than 0xFF00 and this seq is less than 0xFF,
                        // 且未发生回环或者距离上次回环间隔超过0x1FFF个包，则认为回环  [AUTO-TRANSLATED:2907b595]
                        // and no loopback occurs or the interval between the last loopback is greater than 0x1FFF packets, then it is considered a loopback
                        ++_seq_cycles;
                        _last_cycle_packets = _packets;
                        _seq_max = seq;
                    }
                    else if (seq > _seq_max) {
                        // 本次回环前最大seq  [AUTO-TRANSLATED:c02f6a87]
                        // Maximum seq before this loopback
                        _seq_max = seq;
                    }

                    if (!_seq_base) {
                        // 记录第一个rtp的seq  [AUTO-TRANSLATED:ce2bb7d7]
                        // Record the seq of the first rtp
                        _seq_base = seq;
                    }
                    else if (!_seq_cycles && seq < _seq_base) {
                        // 未发生回环，那么取最新的seq为基准seq  [AUTO-TRANSLATED:721b37fc]
                        // If no loopback occurs, then take the latest seq as the base seq
                        _seq_base = seq;
                    }

                    _last_rtp_seq = seq;
                    _last_rtp_sys_stamp = sys_stamp;
                }
                RtcpContext::onRtp(seq, stamp, ntp_stamp_ms, sample_rate, bytes);
        }

        void RtcpContextRecv::onRtcp(rtcp::RtcpPacket* rtcp) {
            rtcp::SenderReport* rtcp_sr = dynamic_cast<rtcp::SenderReport*>( rtcp);
            //auto rtcp_sr = (RtcpSR*)rtcp;
            /**
             last SR timestamp (LSR): 32 bits
              The middle 32 bits out of 64 in the NTP timestamp (as explained in
              Section 4) received as part of the most recent RTCP sender report
              (SR) packet from source SSRC_n.  If no SR has been received yet,
              the field is set to zero.
             */
            _last_sr_lsr = ((rtcp_sr->ntp().seconds() & 0xFFFF) << 16) | ((rtcp_sr->ntp().fractions() >> 16) & 0xFFFF);
            _last_sr_ntp_sys = rtc::SystemTimeMillis();
            //switch (sr->) {
            //case RtcpType::RTCP_SR: {
            //    
            //    break;
            //}
            //default:
            //    break;
            //}
        }

        size_t RtcpContextRecv::getExpectedPackets() const {
            return (_seq_cycles << 16) + _seq_max - _seq_base + 1;
        }

        size_t RtcpContextRecv::getExpectedPacketsInterval() {
            auto expected = getExpectedPackets();
            auto ret = expected - _last_expected;
            _last_expected = expected;
            return ret;
        }

        size_t RtcpContextRecv::getLost() {
            return getExpectedPackets() - _packets;
        }

        size_t RtcpContextRecv::getLostInterval() {
            auto lost = getLost();
            auto ret = lost - _last_lost;
            _last_lost = lost;
            return ret;
        }

        rtc::Buffer RtcpContextRecv::createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) {

            //  Example:
            //  ReportBlock report_block;
            //  report_block.SetMediaSsrc(234);
            //  report_block.SetFractionLost(10);
            //
            //  ReceiverReport rr;
            //  rr.SetSenderSsrc(123);
            //  rr.AddReportBlock(report_block);
            //
            //  Fir fir;
            //  fir.SetSenderSsrc(123);
            //  fir.AddRequestTo(234, 56);
            //
            //  size_t length = 0;                     // Builds an intra frame request
            //  uint8_t packet[kPacketSize];           // with sequence number 56.
            //  fir.Build(packet, &length, kPacketSize);
            //
            //  rtc::Buffer packet = fir.Build();      // Returns a RawPacket holding
            //                                         // the built rtcp packet.
            //
            //  CompoundPacket compound;               // Builds a compound RTCP packet with
            //  compound.Append(&rr);                  // a receiver report, report block
            //  compound.Append(&fir);                 // and fir message.
            //  rtc::Buffer packet = compound.Build();
            

            rtcp:: ReportBlock report_block;
            report_block.SetMediaSsrc(rtp_ssrc);
           
            std::unique_ptr<rtcp::ReceiverReport>  rtcp_rr = std::make_unique<rtcp::ReceiverReport>();
            rtcp_rr->SetSenderSsrc(rtcp_ssrc);
            
            //auto rtcp = RtcpRR::create(1);
            //rtcp->ssrc = htonl(rtcp_ssrc);

            //ReportItem* item = (ReportItem*)&rtcp->items;
            //item->ssrc = htonl(rtp_ssrc);

            uint8_t fraction = 0;
            auto expected_interval = getExpectedPacketsInterval();
            if (expected_interval) {
                fraction = uint8_t(getLostInterval() << 8 / expected_interval);
            }
            report_block.SetFractionLost(fraction);
           // item->fraction = fraction;
            //item->cumulative = htonl(uint32_t(getLost())) >> 8;
            report_block.SetCumulativeLost(htonl(uint32_t(getLost())) >> 8);
            //item->seq_cycles = htons(_seq_cycles);
            
            //item->seq_max = htons(_seq_max);
            //report_block.SetExtHighestSeqNum(htonl(uint32_t(_jitter)));
            //item->jitter = htonl(uint32_t(_jitter));
            report_block.SetJitter(htonl(uint32_t(_jitter)));
            //item->last_sr_stamp = htonl(_last_sr_lsr);
            report_block.SetLastSr(htonl(_last_sr_lsr));
            // now - Last SR time,单位毫秒  [AUTO-TRANSLATED:cc449199]
            // now - Last SR time, in milliseconds
            auto delay = rtc::SystemTimeMillis() - _last_sr_ntp_sys;
            // in units of 1/65536 seconds
            auto dlsr = (uint32_t)(delay / 1000.0f * 65536);
            //item->delay_since_last_sr = htonl(_last_sr_lsr ? dlsr : 0);
            report_block.SetDelayLastSr(htonl(_last_sr_lsr ? dlsr : 0));

            rtcp_rr->AddReportBlock(report_block);
          rtcp::  CompoundPacket compound;               // Builds a compound RTCP packet with
           compound.Append(std::move(rtcp_rr));                  // a receiver report, report block
          // compound.Append(&fir);                 // and fir message.
          // rtc::Buffer packet = compound.Build();
            return compound.Build();
        }
	}
}
