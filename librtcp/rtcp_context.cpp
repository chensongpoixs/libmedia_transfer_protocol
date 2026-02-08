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
 /***********************************************************************************************
 created: 		2025-11-09

 author:			chensong

 purpose:		RTCP Context Implementation
 输赢不重要，答案对你们有什么意义才重要。

 光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


 我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
 然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
 3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
 然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
 于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
 我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
 从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
 我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
 沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
 安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
 ************************************************************************************************/

#include "libmedia_transfer_protocol/librtcp/rtcp_context.h"
namespace libmedia_transfer_protocol
{
	namespace librtcp {

        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 处理RTP包（On RTP Packet）
        *  
        *  基类实现，更新基本的统计信息：包数、字节数、时间戳。
        *  派生类可以重写该方法以实现更复杂的统计逻辑。
        *  
        *  @param seq RTP序列号（未使用）
        *  @param stamp RTP时间戳
        *  @param ntp_stamp_ms NTP时间戳，单位毫秒
        *  @param sample_rate 采样率（未使用）
        *  @param bytes 数据字节数
        */
        void RtcpContext::onRtp(
            uint16_t /*seq*/, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t /*sample_rate*/, size_t bytes) {
            ++_packets;
            _bytes += bytes;
            _last_rtp_stamp = stamp;
            _last_ntp_stamp_ms = ntp_stamp_ms;
        }

        /**
        *  @brief 获取理应收到的包数（Get Expected Packets）
        *  
        *  基类实现抛出异常，因为RTP发送者无法统计应收包数。
        *  只有接收者才能根据序列号范围计算理应收到的包数。
        *  
        *  @throws std::runtime_error 抛出异常提示未实现
        */
        size_t RtcpContext::getExpectedPackets() const {
            throw std::runtime_error("没有实现, rtp发送者无法统计应收包数");
        }

        /**
        *  @brief 获取应收包数增量（Get Expected Packets Interval）
        *  
        *  基类实现抛出异常，因为RTP发送者无法统计应收包数。
        *  
        *  @throws std::runtime_error 抛出异常提示未实现
        */
        size_t RtcpContext::getExpectedPacketsInterval() {
            throw std::runtime_error("没有实现, rtp发送者无法统计应收包数");
        }

        /**
        *  @brief 获取总丢包数（Get Lost Packets）
        *  
        *  基类实现抛出异常，因为RTP发送者无法统计丢包率。
        *  只有接收者才能通过对比理应收到的包数和实际收到的包数计算丢包。
        *  
        *  @throws std::runtime_error 抛出异常提示未实现
        */
        size_t RtcpContext::getLost() {
            throw std::runtime_error("没有实现, rtp发送者无法统计丢包率");
        }

        /**
        *  @brief 获取丢包数增量（Get Lost Packets Interval）
        *  
        *  基类实现抛出异常，因为RTP发送者无法统计丢包率。
        *  
        *  @throws std::runtime_error 抛出异常提示未实现
        */
        size_t RtcpContext::getLostInterval() {
            throw std::runtime_error("没有实现, rtp发送者无法统计丢包率");
        }

        /**
        *  @brief 创建SR RTCP报文（Create Sender Report）
        *  
        *  基类实现抛出异常，因为RTP接收者不应发送SR报文。
        *  SR报文只能由发送者生成，包含发送统计信息。
        *  
        *  @param rtcp_ssrc RTCP的SSRC
        *  @throws std::runtime_error 抛出异常提示未实现
        */
        rtc::Buffer RtcpContext::createRtcpSR(uint32_t rtcp_ssrc) {
            throw std::runtime_error("没有实现, rtp接收者尝试发送sr包");
        }

        /**
        *  @brief 创建RR RTCP报文（Create Receiver Report）
        *  
        *  基类实现抛出异常，因为RTP发送者不应发送RR报文。
        *  RR报文只能由接收者生成，包含接收统计信息。
        *  
        *  @param rtcp_ssrc RTCP的SSRC
        *  @param rtp_ssrc RTP的SSRC
        *  @throws std::runtime_error 抛出异常提示未实现
        */
        rtc::Buffer RtcpContext::createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) {
            throw std::runtime_error("没有实现, rtp发送者尝试发送rr包");
        }

        /**
        *  @brief 创建XR DLRR RTCP报文（Create Extended Report DLRR）
        *  
        *  基类实现抛出异常，因为RTP发送者不应发送XR DLRR报文。
        *  
        *  @param rtcp_ssrc RTCP的SSRC
        *  @param rtp_ssrc RTP的SSRC
        *  @throws std::runtime_error 抛出异常提示未实现
        */
        rtc::Buffer RtcpContext::createRtcpXRDLRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) {
            throw std::runtime_error("没有实现, rtp发送者尝试发送xr dlrr包");
        }

	}
}
