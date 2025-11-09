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

#include "libmedia_transfer_protocol/librtcp/rtcp_context.h"
namespace libmedia_transfer_protocol
{
	namespace librtcp {

        void RtcpContext::onRtp(
            uint16_t /*seq*/, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t /*sample_rate*/, size_t bytes) {
            ++_packets;
            _bytes += bytes;
            _last_rtp_stamp = stamp;
            _last_ntp_stamp_ms = ntp_stamp_ms;
        }

        size_t RtcpContext::getExpectedPackets() const {
            throw std::runtime_error("没有实现, rtp发送者无法统计应收包数");
        }

        size_t RtcpContext::getExpectedPacketsInterval() {
            throw std::runtime_error("没有实现, rtp发送者无法统计应收包数");
        }

        size_t RtcpContext::getLost() {
            throw std::runtime_error("没有实现, rtp发送者无法统计丢包率");
        }

        size_t RtcpContext::getLostInterval() {
            throw std::runtime_error("没有实现, rtp发送者无法统计丢包率");
        }

        rtc::Buffer RtcpContext::createRtcpSR(uint32_t rtcp_ssrc) {
            throw std::runtime_error("没有实现, rtp接收者尝试发送sr包");
        }

        rtc::Buffer RtcpContext::createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) {
            throw std::runtime_error("没有实现, rtp发送者尝试发送rr包");
        }

        rtc::Buffer RtcpContext::createRtcpXRDLRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) {
            throw std::runtime_error("没有实现, rtp发送者尝试发送xr dlrr包");
        }

	}
}
