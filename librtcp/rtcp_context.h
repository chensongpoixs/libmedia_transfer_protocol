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


#ifndef _C_LIBRTCP_RTCP_CONTEXT_H_
#define _C_LIBRTCP_RTCP_CONTEXT_H_
#include <cstdio>
#include <cstdint>
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/sender_report.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/compound_packet.h"
namespace libmedia_transfer_protocol
{
	namespace librtcp {
		class  RtcpContext
		{
		public:
            RtcpContext() = default ;
            virtual  ~RtcpContext() = default  ;
		public:
            /**
             * 输出或输入rtp时调用
             * @param seq rtp的seq
             * @param stamp rtp的时间戳，单位采样数(非毫秒)
             * @param ntp_stamp_ms ntp时间戳
             * @param rtp rtp时间戳采样率，视频一般为90000，音频一般为采样率
             * @param bytes rtp数据长度
             * Called when outputting or inputting rtp
             * @param seq rtp's seq
             * @param stamp rtp's timestamp, unit is sample number (not millisecond)
             * @param ntp_stamp_ms ntp timestamp
             * @param rtp rtp timestamp sampling rate, video is generally 90000, audio is generally sampling rate
             * @param bytes rtp data length
      
             */
            virtual void onRtp(uint16_t seq, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t sample_rate, size_t bytes);

            /**
             * 输入sr rtcp包
             * @param rtcp 输入一个rtcp
             * Input sr rtcp packet
             * @param rtcp input an rtcp
              
             */
            virtual void onRtcp(rtcp::RtcpPacket* rtcp) = 0;

            /**
             * 计算总丢包数
             * Calculate the total number of lost packets
              
             */
            virtual size_t getLost();

            /**
             * 返回理应收到的rtp数
             * Return the number of rtp that should be received
              
             */
            virtual size_t getExpectedPackets() const;

            /**
             * 创建SR rtcp包
             * @param rtcp_ssrc rtcp的ssrc
             * @return rtcp包
             * Create SR rtcp packet
             * @param rtcp_ssrc rtcp's ssrc
             * @return rtcp packet
              
             */
            virtual rtc::Buffer createRtcpSR(uint32_t rtcp_ssrc);

            /**
             * @brief 创建xr的dlrr包，用于接收者估算rtt
             *
             * @return toolkit::Buffer::Ptr
             * @brief Create xr's dlrr packet, used by receiver to estimate rtt
             *
             * @return toolkit::Buffer::Ptr
              
             */
            virtual rtc::Buffer createRtcpXRDLRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc);

            /**
             * 创建RR rtcp包
             * @param rtcp_ssrc rtcp的ssrc
             * @param rtp_ssrc rtp的ssrc
             * @return rtcp包
             * Create RR rtcp packet
             * @param rtcp_ssrc rtcp's ssrc
             * @param rtp_ssrc rtp's ssrc
             * @return rtcp packet
              
             */
            virtual rtc::Buffer createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc);

            /**
             * 上次结果与本次结果间应收包数
             * Number of packets that should be received between the last result and the current result
              
             */
            virtual size_t getExpectedPacketsInterval();

            /**
             * 上次结果与本次结果间丢包个数
             * Number of lost packets between the last result and the current result
              
             */
            virtual size_t getLostInterval();

		protected:

			// 收到或发送的rtp的字节数  [AUTO-TRANSLATED:a38d88a9]
				// Number of bytes of rtp received or sent
			int32_t _bytes = 0;
			// 收到或发送的rtp的个数  [AUTO-TRANSLATED:b28c3c90]
			// Number of rtp received or sent
			int32_t _packets = 0;
			// 上次的rtp时间戳,毫秒  [AUTO-TRANSLATED:99eecec6]
			// Last rtp timestamp, milliseconds
			uint32_t _last_rtp_stamp = 0;
			uint64_t _last_ntp_stamp_ms = 0;
		};
	}
}



#endif // _C_LIBRTCP_H_
