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

 purpose:		RTCP Receiver Context
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


#ifndef _C_LIBRTCP_RTCP_CONTEXT_RECV_H_
#define _C_LIBRTCP_RTCP_CONTEXT_RECV_H_
#include "libmedia_transfer_protocol/librtcp/rtcp_context.h"
namespace libmedia_transfer_protocol
{
    namespace librtcp {
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief RTCP接收者上下文类（RTCP Receiver Context）
        *  
        *  RtcpContextRecv是RTCP接收者的上下文管理类，继承自RtcpContext基类。
        *  该类负责统计RTP包的接收情况，计算丢包率、抖动、延迟等质量指标，
        *  并生成接收者报告（Receiver Report, RR）RTCP报文。
        *  
        *  主要功能：
        *  - 统计RTP包接收情况（序列号、时间戳、字节数）
        *  - 计算丢包率（Packet Loss Rate）
        *  - 计算时间戳抖动（Jitter）
        *  - 处理序列号回绕（Sequence Number Wrap-around）
        *  - 生成RR RTCP报文
        *  - 处理SR RTCP报文，计算往返时延（RTT）
        *  
        *  统计指标说明：
        *  
        *  1. 丢包率（Packet Loss）：
        *     - 理应收到的包数 = (seq_cycles << 16) + seq_max - seq_base + 1
        *     - 实际收到的包数 = _packets
        *     - 丢包数 = 理应收到的包数 - 实际收到的包数
        *     - 丢包率 = 丢包数 / 理应收到的包数
        *  
        *  2. 时间戳抖动（Jitter）：
        *     - 抖动反映了包到达时间的变化程度
        *     - 计算公式：J(i) = J(i-1) + (|D(i-1,i)| - J(i-1)) / 16
        *     - D(i-1,i) = (Ri - Ri-1) - (Si - Si-1)
        *     - Ri: 包i的到达时间（系统时间）
        *     - Si: 包i的RTP时间戳
        *     - 单位为采样次数（不是毫秒）
        *  
        *  3. 序列号回绕（Sequence Number Wrap-around）：
        *     - RTP序列号为16位，范围0-65535
        *     - 当序列号从65535跳到0时，发生回绕
        *     - 使用_seq_cycles记录回绕次数
        *     - 扩展序列号 = (seq_cycles << 16) + seq
        *  
        *  4. 往返时延（RTT）：
        *     - 通过SR和RR报文计算
        *     - LSR（Last SR）：上次收到的SR的NTP时间戳中间32位
        *     - DLSR（Delay since Last SR）：收到SR后的延迟时间
        *     - RTT = 当前时间 - LSR - DLSR
        *  
        *  工作流程：
        *  1. 收到RTP包时，调用onRtp()更新统计信息
        *  2. 计算时间戳抖动，更新序列号范围
        *  3. 检测序列号回绕，更新回绕计数器
        *  4. 定期调用createRtcpRR()生成RR报文
        *  5. 收到SR报文时，调用onRtcp()记录时间戳
        *  
        *  @note 该类实现了接收者的所有统计逻辑
        *  @note 序列号回绕检测需要满足一定条件，避免误判
        *  @note 抖动计算使用指数加权移动平均（EWMA）
        *  
        *  使用示例：
        *  @code
        *  RtcpContextRecv recv_ctx;
        *  
        *  // 收到RTP包时更新统计
        *  uint16_t seq = rtp_packet.GetSequenceNumber();
        *  uint32_t stamp = rtp_packet.GetTimestamp();
        *  uint64_t ntp_ms = getCurrentTimeMs();
        *  uint32_t sample_rate = 90000; // 视频
        *  size_t bytes = rtp_packet.GetPayloadSize();
        *  recv_ctx.onRtp(seq, stamp, ntp_ms, sample_rate, bytes);
        *  
        *  // 生成RR报文
        *  uint32_t rtcp_ssrc = 12345;
        *  uint32_t rtp_ssrc = 67890;
        *  auto rr_packet = recv_ctx.createRtcpRR(rtcp_ssrc, rtp_ssrc);
        *  
        *  // 查询统计信息
        *  size_t lost = recv_ctx.getLost();
        *  size_t expected = recv_ctx.getExpectedPackets();
        *  double loss_rate = (double)lost / expected;
        *  @endcode
        */
        class RtcpContextRecv : public RtcpContext
        {
        public:
            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 处理RTP包（On RTP Packet）
            *  
            *  重写基类方法，实现接收者的统计逻辑。该方法会计算时间戳抖动、
            *  检测序列号回绕、更新序列号范围等。
            *  
            *  处理流程：
            *  1. 计算时间戳抖动（Jitter）
            *  2. 检测序列号回绕（Wrap-around）
            *  3. 更新最大序列号（seq_max）
            *  4. 更新基准序列号（seq_base）
            *  5. 调用基类方法更新基本统计信息
            *  
            *  序列号回绕检测条件：
            *  - 上次seq > 0xFF00（接近65535）
            *  - 本次seq < 0xFF（接近0）
            *  - 未发生回绕或距离上次回绕超过0x1FFF个包
            *  
            *  @param seq RTP序列号
            *  @param stamp RTP时间戳，单位采样数
            *  @param ntp_stamp_ms NTP时间戳，单位毫秒
            *  @param sample_rate 采样率
            *  @param bytes 数据字节数
            */
            void onRtp(uint16_t seq, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t sample_rate, size_t bytes) override;

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 生成RR RTCP报文（Create Receiver Report）
            *  
            *  生成接收者报告（Receiver Report）RTCP报文。RR报文包含接收统计信息：
            *  - SSRC：同步源标识符
            *  - Fraction Lost：丢包率（8位）
            *  - Cumulative Lost：累计丢包数（24位）
            *  - Extended Highest Sequence Number：扩展最高序列号
            *  - Jitter：时间戳抖动
            *  - LSR：上次SR时间戳
            *  - DLSR：上次SR后的延迟
            *  
            *  @param rtcp_ssrc RTCP的SSRC
            *  @param rtp_ssrc RTP的SSRC
            *  @return RR RTCP报文缓冲区
            */
            rtc::Buffer createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc) override;

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 获取理应收到的包数（Get Expected Packets）
            *  
            *  根据序列号范围计算理应收到的RTP包总数。
            *  计算公式：(seq_cycles << 16) + seq_max - seq_base + 1
            *  
            *  @return 理应收到的包数
            */
            size_t getExpectedPackets() const override;

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 获取应收包数增量（Get Expected Packets Interval）
            *  
            *  返回自上次调用以来理应收到的包数增量。
            *  用于计算时间间隔内的丢包率。
            *  
            *  @return 应收包数增量
            *  @note 该方法会更新_last_expected状态
            */
            size_t getExpectedPacketsInterval() override;

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 获取总丢包数（Get Lost Packets）
            *  
            *  计算从开始统计到现在的总丢包数。
            *  计算公式：理应收到的包数 - 实际收到的包数
            *  
            *  @return 总丢包数
            */
            size_t getLost() override;

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 获取丢包数增量（Get Lost Packets Interval）
            *  
            *  返回自上次调用以来的丢包数增量。
            *  用于计算时间间隔内的丢包率。
            *  
            *  @return 丢包数增量
            *  @note 该方法会更新_last_lost状态
            */
            size_t getLostInterval() override;

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 处理RTCP报文（On RTCP Packet）
            *  
            *  处理接收到的RTCP报文，主要处理SR（发送者报告）。
            *  从SR中提取NTP时间戳，用于计算往返时延（RTT）。
            *  
            *  LSR计算：
            *  - 取NTP时间戳的中间32位
            *  - LSR = ((ntp_sec & 0xFFFF) << 16) | ((ntp_frac >> 16) & 0xFFFF)
            *  
            *  @param rtcp RTCP报文指针
            */
            void onRtcp(rtcp::RtcpPacket* rtcp) override;

        private:
            // 时间戳抖动值，单位为采样次数
            double _jitter = 0;
            // 第一个seq的值，用作基准
            uint16_t _seq_base = 0;
            // RTP最大seq，当前回绕周期内的最大序列号
            uint16_t _seq_max = 0;
            // RTP回绕次数，序列号从65535跳到0的次数
            uint16_t _seq_cycles = 0;
            // 上次回绕发生时，记录的RTP包数
            size_t _last_cycle_packets = 0;
            // 上次的seq，用于检测回绕
            uint16_t _last_rtp_seq = 0;
            // 上次的RTP的系统时间戳（毫秒），用于统计抖动
            uint64_t _last_rtp_sys_stamp = 0;
            // 上次统计的丢包总数
            size_t _last_lost = 0;
            // 上次统计应收RTP包总数
            size_t _last_expected = 0;
            // 上次收到SR包时计算出的Last SR timestamp
            uint32_t _last_sr_lsr = 0;
            // 上次收到SR时的系统时间戳，单位毫秒
            uint64_t _last_sr_ntp_sys = 0;
        };
    }
}



#endif // _C_LIBRTCP_H_
