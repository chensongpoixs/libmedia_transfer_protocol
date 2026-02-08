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

 purpose:		RTCP Context Base Class
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
		/**
		*  @author chensong
		*  @date 2025-11-09
		*  @brief RTCP上下文基类（RTCP Context Base Class）
		*  
		*  RtcpContext是RTCP统计信息管理的抽象基类，定义了RTCP报文生成和
		*  统计信息收集的基本接口。该类用于跟踪RTP流的发送或接收状态，
		*  并生成相应的RTCP报文（SR或RR）。
		*  
		*  RTCP协议说明：
		*  - RTCP（RTP Control Protocol）是RTP的控制协议
		*  - 用于监控数据传输质量，提供反馈信息
		*  - 主要报文类型：SR（发送者报告）、RR（接收者报告）
		*  - SR包含发送统计信息（包数、字节数、时间戳等）
		*  - RR包含接收统计信息（丢包率、抖动、延迟等）
		*  
		*  设计模式：
		*  - 使用抽象基类定义接口
		*  - 派生类实现具体的发送者或接收者逻辑
		*  - RtcpContextRecv：接收者上下文，生成RR报文
		*  - RtcpContextSend：发送者上下文，生成SR报文（未在此文件中）
		*  
		*  工作流程：
		*  1. 每次发送或接收RTP包时，调用onRtp()更新统计信息
		*  2. 定期调用createRtcpSR()或createRtcpRR()生成RTCP报文
		*  3. 接收到RTCP报文时，调用onRtcp()处理反馈信息
		*  4. 通过getLost()、getExpectedPackets()等方法查询统计信息
		*  
		*  @note 该类是抽象基类，不能直接实例化
		*  @note 派生类需要实现纯虚函数onRtcp()
		*  @note 部分方法在基类中抛出异常，需要派生类重写
		*  
		*  使用示例：
		*  @code
		*  // 创建接收者上下文
		*  RtcpContextRecv recv_ctx;
		*  
		*  // 收到RTP包时更新统计
		*  recv_ctx.onRtp(seq, stamp, ntp_stamp_ms, sample_rate, bytes);
		*  
		*  // 生成RR报文
		*  auto rr_packet = recv_ctx.createRtcpRR(rtcp_ssrc, rtp_ssrc);
		*  
		*  // 查询丢包数
		*  size_t lost = recv_ctx.getLost();
		*  @endcode
		*/
		class  RtcpContext
		{
		public:
            /**
            *  @brief 默认构造函数
            */
            RtcpContext() = default ;

            /**
            *  @brief 虚析构函数
            */
            virtual  ~RtcpContext() = default  ;

		public:
            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 处理RTP包（On RTP Packet）
            *  
            *  当发送或接收RTP包时调用该方法，用于更新统计信息。
            *  基类实现更新基本的包数、字节数和时间戳信息。
            *  派生类可以重写该方法以实现更复杂的统计逻辑。
            *  
            *  @param seq RTP序列号
            *  @param stamp RTP时间戳，单位为采样数（非毫秒）
            *  @param ntp_stamp_ms NTP时间戳，单位毫秒
            *  @param sample_rate RTP时间戳采样率，视频一般为90000，音频一般为采样率
            *  @param bytes RTP数据长度（字节数）
            *  @note 视频的采样率通常为90000Hz（90kHz）
            *  @note 音频的采样率通常为8000Hz、16000Hz、48000Hz等
            */
            virtual void onRtp(uint16_t seq, uint32_t stamp, uint64_t ntp_stamp_ms, uint32_t sample_rate, size_t bytes);

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 处理RTCP报文（On RTCP Packet）
            *  
            *  当接收到RTCP报文时调用该方法，用于处理反馈信息。
            *  派生类必须实现该纯虚函数。
            *  
            *  @param rtcp RTCP报文指针
            *  @note 该方法为纯虚函数，派生类必须实现
            */
            virtual void onRtcp(rtcp::RtcpPacket* rtcp) = 0;

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 计算总丢包数（Get Lost Packets）
            *  
            *  返回从开始统计到现在的总丢包数。
            *  基类实现抛出异常，派生类需要重写。
            *  
            *  @return 总丢包数
            *  @note 仅接收者上下文需要实现该方法
            *  @note 发送者无法统计丢包率
            */
            virtual size_t getLost();

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 返回理应收到的RTP包数（Get Expected Packets）
            *  
            *  根据序列号范围计算理应收到的RTP包总数。
            *  基类实现抛出异常，派生类需要重写。
            *  
            *  @return 理应收到的包数
            *  @note 仅接收者上下文需要实现该方法
            */
            virtual size_t getExpectedPackets() const;

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 创建SR RTCP报文（Create Sender Report）
            *  
            *  生成发送者报告（Sender Report）RTCP报文。
            *  SR报文包含发送统计信息：NTP时间戳、RTP时间戳、发送包数、发送字节数等。
            *  基类实现抛出异常，派生类需要重写。
            *  
            *  @param rtcp_ssrc RTCP的SSRC（同步源标识符）
            *  @return RTCP报文缓冲区
            *  @note 仅发送者上下文需要实现该方法
            *  @note 接收者不应发送SR报文
            */
            virtual rtc::Buffer createRtcpSR(uint32_t rtcp_ssrc);

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 创建XR DLRR RTCP报文（Create Extended Report DLRR）
            *  
            *  生成扩展报告（Extended Report）中的DLRR（Delay since Last Receiver Report）块。
            *  用于接收者估算往返时延（RTT）。
            *  基类实现抛出异常，派生类需要重写。
            *  
            *  @param rtcp_ssrc RTCP的SSRC
            *  @param rtp_ssrc RTP的SSRC
            *  @return RTCP报文缓冲区
            *  @note 该方法用于高级RTT估算场景
            */
            virtual rtc::Buffer createRtcpXRDLRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc);

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 创建RR RTCP报文（Create Receiver Report）
            *  
            *  生成接收者报告（Receiver Report）RTCP报文。
            *  RR报文包含接收统计信息：丢包率、抖动、延迟等。
            *  基类实现抛出异常，派生类需要重写。
            *  
            *  @param rtcp_ssrc RTCP的SSRC
            *  @param rtp_ssrc RTP的SSRC
            *  @return RTCP报文缓冲区
            *  @note 仅接收者上下文需要实现该方法
            *  @note 发送者不应发送RR报文
            */
            virtual rtc::Buffer createRtcpRR(uint32_t rtcp_ssrc, uint32_t rtp_ssrc);

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 上次结果与本次结果间应收包数（Get Expected Packets Interval）
            *  
            *  返回自上次调用以来理应收到的包数增量。
            *  用于计算时间间隔内的丢包率。
            *  基类实现抛出异常，派生类需要重写。
            *  
            *  @return 应收包数增量
            *  @note 该方法会更新内部状态，记录上次的值
            */
            virtual size_t getExpectedPacketsInterval();

            /**
            *  @author chensong
            *  @date 2025-11-09
            *  @brief 上次结果与本次结果间丢包个数（Get Lost Packets Interval）
            *  
            *  返回自上次调用以来的丢包数增量。
            *  用于计算时间间隔内的丢包率。
            *  基类实现抛出异常，派生类需要重写。
            *  
            *  @return 丢包数增量
            *  @note 该方法会更新内部状态，记录上次的值
            */
            virtual size_t getLostInterval();

		protected:
			// 收到或发送的RTP字节数
			int32_t _bytes = 0;
			// 收到或发送的RTP包个数
			int32_t _packets = 0;
			// 上次的RTP时间戳，单位毫秒
			uint32_t _last_rtp_stamp = 0;
			// 上次的NTP时间戳，单位毫秒
			uint64_t _last_ntp_stamp_ms = 0;
		};
	}
}



#endif // _C_LIBRTCP_H_
