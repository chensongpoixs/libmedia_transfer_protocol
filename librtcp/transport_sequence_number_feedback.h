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

 purpose:		Transport Sequence Number Feedback Generator
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

#ifndef _TRANSPORT_SEQUENCE_NUMBER_FEEDBACK_H_
#define _TRANSPORT_SEQUENCE_NUMBER_FEEDBACK_H_


 
#include "libmedia_transfer_protocol/rtp_receiver_interface.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"
#include "api/units/data_rate.h"
#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "rtc_base/numerics/sequence_number_util.h"
#include "modules/remote_bitrate_estimator/packet_arrival_map.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <cstdint>
//#include "libmedia_transfer_protocol/librtcp/packet_arrival_map.h"


namespace libmedia_transfer_protocol {
    namespace librtcp {  
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 传输序列号反馈生成器（Transport Sequence Number Feedback Generator）
        *  
        *  TransportSequenceNumberFeedback是用于生成TWCC（传输层拥塞控制）反馈报文的类。
        *  该类在启用发送端带宽估算（Send-side BWE）时使用，负责收集RTP包的接收信息
        *  并生成RTCP反馈报文发送给发送端。
        *  
        *  TWCC机制说明：
        *  - TWCC是WebRTC中的传输层拥塞控制机制
        *  - 接收端记录每个RTP包的到达时间
        *  - 定期生成TWCC反馈报文，包含包状态和到达时间
        *  - 发送端根据反馈计算网络延迟、丢包率，动态调整发送码率
        *  
        *  RTP扩展头要求：
        *  - 接收的RTP包必须包含传输序列号扩展头
        *  - 支持两种扩展头格式：
        *    1. draft-holmer-rmcat-transport-wide-cc-extensions-01
        *    2. http://www.webrtc.org/experiments/rtp-hdrext/transport-wide-cc-02
        *  
        *  工作流程：
        *  1. 接收RTP包时，调用OnReceivedPacket()记录到达信息
        *  2. 定期调用Process()检查是否需要发送反馈
        *  3. 满足条件时，生成TWCC反馈报文并发送
        *  4. 清理旧的包到达记录，避免内存无限增长
        *  
        *  反馈触发条件：
        *  - 定期反馈：每隔send_interval_发送一次
        *  - 请求反馈：收到带有反馈请求标志的RTP包
        *  - 包数量达到阈值：收集的包数量达到一定数量
        *  
        *  @note 该类不是线程安全的，需要在单线程环境中使用
        *  @note 使用PacketArrivalTimeMap存储包到达时间
        *  @note 支持动态调整反馈发送间隔
        *  
        *  使用示例：
        *  @code
        *  TransportSequenceNumberFeedback feedback_gen;
        *  
        *  // 设置带宽估算值
        *  feedback_gen.OnSendBandwidthEstimateChanged(webrtc::DataRate::KilobitsPerSec(1000));
        *  
        *  // 收到RTP包时记录
        *  RtpPacketReceived packet;
        *  feedback_gen.OnReceivedPacket(packet);
        *  
        *  // 定期处理
        *  int64_t now_ms = getCurrentTimeMs();
        *  feedback_gen.Process(now_ms);
        *  @endcode
        */
#if 1
        class TransportSequenceNumberFeedback
        {
         public:
           /**
           *  @brief 构造函数
           */
           TransportSequenceNumberFeedback( );

           /**
           *  @brief 析构函数
           */
          virtual ~TransportSequenceNumberFeedback();

          /**
          *  @author chensong
          *  @date 2025-11-09
          *  @brief 处理接收到的RTP包（On Received Packet）
          *  
          *  当接收到RTP包时调用该方法，记录包的传输序列号和到达时间。
          *  如果包中包含反馈请求标志，会立即触发反馈发送。
          *  
          *  处理流程：
          *  1. 从RTP扩展头中提取传输序列号
          *  2. 记录包的到达时间到packet_arrival_times_
          *  3. 检查是否包含反馈请求标志
          *  4. 如果有请求，调用SendFeedbackOnRequest()
          *  5. 清理过期的旧包记录
          *  
          *  @param packet 接收到的RTP包
          *  @note 包必须包含传输序列号扩展头
          *  @note 该方法会自动处理序列号回绕
          */
          void OnReceivedPacket(const libmedia_transfer_protocol::RtpPacketReceived& packet)  ;

          /**
          *  @author chensong
          *  @date 2025-11-09
          *  @brief 带宽估算值变化通知（On Send Bandwidth Estimate Changed）
          *  
          *  当发送端带宽估算值发生变化时调用该方法。
          *  该方法可以根据带宽估算值动态调整反馈发送间隔。
          *  
          *  @param estimate 新的带宽估算值
          *  @note 带宽越高，可以更频繁地发送反馈
          *  @note 带宽越低，应减少反馈频率以节省带宽
          */
          void OnSendBandwidthEstimateChanged(webrtc::DataRate estimate)  ;

          /**
          *  @author chensong
          *  @date 2025-11-09
          *  @brief 定期处理（Process）
          *  
          *  定期调用该方法以检查是否需要发送TWCC反馈报文。
          *  该方法会检查时间间隔，满足条件时发送定期反馈。
          *  
          *  处理流程：
          *  1. 检查距离上次处理的时间间隔
          *  2. 如果达到send_interval_，调用SendPeriodicFeedbacks()
          *  3. 更新last_process_time_
          *  
          *  @param now_ms 当前时间戳，单位毫秒
          *  @note 建议每隔几十毫秒调用一次
          *  @note 实际发送间隔由send_interval_控制
          */
          void Process(int64_t  now_ms)  ;

         private:
          /**
          *  @brief 清理旧包记录（Maybe Cull Old Packets）
          *  
          *  删除过期的包到达记录，避免内存无限增长。
          *  
          *  @param sequence_number 当前序列号
          *  @param arrival_time 当前到达时间
          */
          void MaybeCullOldPackets(int64_t sequence_number, int64_t arrival_time) ;

          /**
          *  @brief 发送定期反馈（Send Periodic Feedbacks）
          *  
          *  发送定期的TWCC反馈报文，包含自上次反馈以来收到的所有包的信息。
          */
          void SendPeriodicFeedbacks()  ;

          /**
          *  @brief 根据请求发送反馈（Send Feedback On Request）
          *  
          *  当收到带有反馈请求标志的RTP包时，立即发送反馈报文。
          *  
          *  @param sequence_number 请求反馈的包的序列号
          *  @param feedback_request 反馈请求信息
          */
          void SendFeedbackOnRequest(int64_t sequence_number,
                                     const FeedbackRequest& feedback_request) ;

          /**
          *  @brief 构建反馈报文（Maybe Build Feedback Packet）
          *  
          *  构建TWCC反馈报文，包含指定范围内的包状态和到达时间。
          *  
          *  @param include_timestamps 是否包含时间戳信息
          *  @param begin_sequence_number_inclusive 起始序列号（包含）
          *  @param end_sequence_number_exclusive 结束序列号（不包含）
          *  @param is_periodic_update 是否为定期更新
          *  @return TWCC反馈报文，如果没有包则返回nullptr
          *  @note 如果是定期更新，会更新periodic_window_start_seq_
          */
          std::unique_ptr<rtcp::TransportFeedback> MaybeBuildFeedbackPacket(
              bool include_timestamps,
              int64_t begin_sequence_number_inclusive,
              int64_t end_sequence_number_exclusive,
              bool is_periodic_update)  ;
   
         // 上次处理时间戳，单位毫秒
          int64_t   last_process_time_;
          // 媒体SSRC
          uint32_t media_ssrc_  ;
          // 反馈包计数器
          uint8_t feedback_packet_count_  ;
          // 序列号展开器，处理16位序列号回绕
          webrtc::SeqNumUnwrapper<uint16_t> unwrapper_  ;
   
          // 定期反馈窗口的起始序列号
          int64_t periodic_window_start_seq_;
          // 包到达时间映射表，按序列号存储
          webrtc::PacketArrivalTimeMap packet_arrival_times_  ;

          // 反馈发送间隔，单位毫秒
          int64_t     send_interval_;
          // 是否发送定期反馈
          bool send_periodic_feedback_  ;
        };
#else 
class TransportSequenceNumberFeedbackGenenerator
    /*: public RtpTransportFeedbackGenerator*/ {
public:
    TransportSequenceNumberFeedbackGenenerator(
    /*RtpTransportFeedbackGenerator::RtcpSender feedback_sender*/);
    ~TransportSequenceNumberFeedbackGenenerator();

    void OnReceivedPacket(const RtpPacketReceived& packet)  ;
    void OnSendBandwidthEstimateChanged(webrtc::DataRate estimate)  ;

    webrtc::TimeDelta Process(webrtc::Timestamp now)  ;

private:
    void MaybeCullOldPackets(int64_t sequence_number, webrtc::Timestamp arrival_time) ;
    void SendPeriodicFeedbacks()  ;
    void SendFeedbackOnRequest(int64_t sequence_number,
        const FeedbackRequest& feedback_request) ;

    // Returns a Transport Feedback packet with information about as many
    // packets that has been received between [`begin_sequence_number_incl`,
    // `end_sequence_number_excl`) that can fit in it. If `is_periodic_update`,
    // this represents sending a periodic feedback message, which will make it
    // update the `periodic_window_start_seq_` variable with the first packet
    // that was not included in the feedback packet, so that the next update can
    // continue from that sequence number.
    //
    // If no incoming packets were added, nullptr is returned.
    //
    // `include_timestamps` decide if the returned TransportFeedback should
    // include timestamps.
    std::unique_ptr<rtcp::TransportFeedback> MaybeBuildFeedbackPacket(
        bool include_timestamps,
        int64_t begin_sequence_number_inclusive,
        int64_t end_sequence_number_exclusive,
        bool is_periodic_update)  ;

    //const RtcpSender feedback_sender_;
    webrtc::Timestamp last_process_time_;

    //Mutex lock_;
    uint32_t media_ssrc_  ;
    uint8_t feedback_packet_count_  ;
    webrtc::SeqNumUnwrapper<uint16_t> unwrapper_  ;

    // The next sequence number that should be the start sequence number during
    // periodic reporting. Will be std::nullopt before the first seen packet.
    absl::optional<int64_t> periodic_window_start_seq_  ;

    // Packet arrival times, by sequence number.
    PacketArrivalTimeMap packet_arrival_times_  ;

    webrtc::TimeDelta send_interval_  ;
    bool send_periodic_feedback_  ;
};
#endif // 
    }
}  // namespace  

#endif  //  periodic_window_start_seq_