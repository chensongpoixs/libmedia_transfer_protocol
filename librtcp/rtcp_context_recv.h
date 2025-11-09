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


#ifndef _C_LIBRTCP_RTCP_CONTEXT_RECV_H_
#define _C_LIBRTCP_RTCP_CONTEXT_RECV_H_
#include "libmedia_transfer_protocol/librtcp/rtcp_context.h"
namespace libmedia_transfer_protocol
{
    namespace librtcp {
        class RtcpContextRecv : public RtcpContext
        {
        public:
            void onRtp(uint16_t seq, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t sample_rate, size_t bytes) override;
            rtc::Buffer createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) override;
            size_t getExpectedPackets() const override;
            size_t getExpectedPacketsInterval() override;
            size_t getLost() override;
            size_t getLostInterval() override;
            void onRtcp(rtcp::RtcpPacket* rtcp) override;

        private:
            // 时间戳抖动值  [AUTO-TRANSLATED:8100680c]
            // Timestamp jitter value
            double _jitter = 0;
            // 第一个seq的值  [AUTO-TRANSLATED:d893719d]
            // The value of the first seq
            uint16_t _seq_base = 0;
            // rtp最大seq  [AUTO-TRANSLATED:5cc9f775]
            // Maximum rtp seq
            uint16_t _seq_max = 0;
            // rtp回环次数  [AUTO-TRANSLATED:9fe9c340]
            // Rtp loopback times
            uint16_t _seq_cycles = 0;
            // 上次回环发生时，记录的rtp包数  [AUTO-TRANSLATED:c32cb555]
            // Number of rtp packets recorded when the last loopback occurred
            size_t _last_cycle_packets = 0;
            // 上次的seq  [AUTO-TRANSLATED:07364b7d]
            // Last seq
            uint16_t _last_rtp_seq = 0;
            // 上次的rtp的系统时间戳(毫秒)用于统计抖动  [AUTO-TRANSLATED:b1e8c89b]
            // Last rtp system timestamp (milliseconds) used for jitter statistics
            uint64_t _last_rtp_sys_stamp = 0;
            // 上次统计的丢包总数  [AUTO-TRANSLATED:242e75ed]
            // Last total number of lost packets counted
            size_t _last_lost = 0;
            // 上次统计应收rtp包总数  [AUTO-TRANSLATED:eb2d5f4d]
            // Last total number of rtp packets that should be received counted
            size_t _last_expected = 0;
            // 上次收到sr包时计算出的Last SR timestamp  [AUTO-TRANSLATED:fdec069e]
            // Last SR timestamp calculated when the last SR packet was received
            uint32_t _last_sr_lsr = 0;
            // 上次收到sr时的系统时间戳,单位毫秒  [AUTO-TRANSLATED:044fa0d5]
            // System timestamp when the last SR was received, unit is millisecond
            uint64_t _last_sr_ntp_sys = 0;
        };
    }
}



#endif // _C_LIBRTCP_H_
