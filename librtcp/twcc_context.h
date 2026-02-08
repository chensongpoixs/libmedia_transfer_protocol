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

 purpose:		TWCC (Transport Wide Congestion Control) Context
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


#ifndef _C_LIBRTCP_TWCC_CONTEXT_H_
#define _C_LIBRTCP_TWCC_CONTEXT_H_
#include <cstdio>
#include <cstdint>
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/sender_report.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtcp_packet/compound_packet.h"
#include <map>
namespace libmedia_transfer_protocol
{
	namespace librtcp {
		/**
		*  @author chensong
		*  @date 2025-11-09
		*  @brief TWCC上下文管理类（TWCC Context）
		*  
		*  TwccContext是WebRTC中用于实现传输层拥塞控制（Transport Wide Congestion Control）的核心类。
		*  该类负责收集RTP包的接收状态信息，并生成TWCC反馈报文（RTCP Feedback）发送给发送端，
		*  帮助发送端实时调整发送码率，实现网络拥塞控制。
		*  
		*  TWCC协议说明：
		*  - TWCC是WebRTC标准中定义的拥塞控制机制（RFC 8888）
		*  - 通过在RTP扩展头中携带传输层序列号（Transport Sequence Number）
		*  - 接收端收集RTP包的接收时间戳，定期生成TWCC反馈报文
		*  - 发送端根据反馈报文计算网络延迟、丢包率等指标，动态调整发送码率
		*  - 相比传统的REMB（Receiver Estimated Maximum Bitrate），TWCC提供更精确的拥塞控制
		*  
		*  TWCC反馈报文结构：
		*  - Base Sequence Number：基准序列号，表示本次反馈的起始序列号
		*  - Packet Status Count：包状态数量，表示本次反馈包含多少个RTP包的状态
		*  - Reference Time：参考时间戳，精度为64ms，作为时间基准
		*  - Feedback Packet Count：反馈包计数器，用于标识反馈报文的顺序
		*  - Packet Chunks：包状态块，使用位图或游程编码表示包的接收状态
		*  - Receive Deltas：接收时间增量，精度为250us或1ms，表示相对于参考时间的偏移
		*  
		*  工作流程：
		*  1. 接收端收到RTP包时，调用onRtp()记录TWCC扩展序列号和接收时间戳
		*  2. 检查序列号状态（正常递增、回绕、跳跃等）
		*  3. 将接收状态存储到_rtp_recv_status映射表中
		*  4. 判断是否满足发送TWCC反馈的条件（序列号数量或时间间隔）
		*  5. 满足条件时，调用onSendTwcc()生成TWCC反馈报文
		*  6. 通过回调函数将反馈报文发送给发送端
		*  7. 清空接收状态，准备下一轮收集
		*  
		*  触发TWCC反馈的条件：
		*  - 收集的RTP包数量达到kMaxSeqSize（20个包）
		*  - 时间间隔达到kMaxTimeDelta（256ms）
		*  - 序列号发生回绕（从65535跳到0）
		*  
		*  序列号处理：
		*  - 正常递增：序列号连续增长，正常记录
		*  - 回绕（Looped）：序列号从65535跳到0，触发立即发送TWCC反馈
		*  - 跳跃（Jumped）：序列号异常跳变（过大或过小），丢弃该包
		*  - 乱序（Out of Order）：序列号回退但在合理范围内，正常记录
		*  
		*  @note 该类使用std::map存储接收状态，自动按序列号排序
		*  @note TWCC扩展序列号为16位，范围0-65535，需要处理回绕问题
		*  @note 时间戳精度为毫秒级别，但TWCC反馈中使用250us或64ms精度
		*  @note 该类线程不安全，需要在单线程环境中使用或外部加锁
		*  
		*  使用示例：
		*  @code
		*  TwccContext twcc_ctx;
		*  // 设置TWCC反馈回调
		*  twcc_ctx.setOnSendTwccCB([](uint32_t ssrc, std::string fci) {
		*      // 将FCI封装为RTCP包并发送
		*      sendRtcpFeedback(ssrc, fci);
		*  });
		*  
		*  // 收到RTP包时记录接收状态
		*  uint32_t ssrc = 12345;
		*  uint16_t twcc_seq = rtp_packet.GetExtension<TransportSequenceNumber>();
		*  uint64_t recv_time_ms = getCurrentTimeMs();
		*  twcc_ctx.onRtp(ssrc, twcc_seq, recv_time_ms);
		*  @endcode
		*/
		class TwccContext
		{
		public:
			/**
			*  @author chensong
			*  @date 2025-11-09
			*  @brief 默认构造函数（Default Constructor）
			*  
			*  使用默认构造函数创建TWCC上下文实例。所有成员变量会被初始化为默认值。
			*  
			*  @note 构造后需要调用setOnSendTwccCB()设置反馈回调函数
			*/
			TwccContext() = default;

			/**
			*  @author chensong
			*  @date 2025-11-09
			*  @brief 虚析构函数（Virtual Destructor）
			*  
			*  清理TWCC上下文资源。使用虚析构函数以支持多态。
			*  
			*  @note 析构时会自动清理_rtp_recv_status映射表
			*/
			virtual ~TwccContext() = default;

		public:
			/**
			*  @brief TWCC反馈回调函数类型定义
			*  
			*  该回调函数用于将生成的TWCC反馈报文发送给上层模块。
			*  
			*  @param ssrc 同步源标识符（Synchronization Source），标识RTP流
			*  @param fci 反馈控制信息（Feedback Control Information），TWCC反馈报文的FCI部分
			*/
			using onSendTwccCB = std::function<void(uint32_t ssrc, std::string fci)>;

			/**
			*  @brief 每次TWCC RTCP反馈包含的最大RTP扩展序列号数量
			*  
			*  当收集的RTP包数量达到该阈值时，会触发TWCC反馈报文的发送。
			*  设置为20是为了平衡反馈频率和网络开销。
			*/
			static constexpr size_t kMaxSeqSize = 20;

			/**
			*  @brief 每次TWCC RTCP反馈发送的最大时间间隔，单位毫秒
			*  
			*  当时间间隔达到该阈值时，即使未达到kMaxSeqSize，也会触发TWCC反馈报文的发送。
			*  设置为256ms是为了确保反馈的实时性，避免拥塞控制响应过慢。
			*/
			static constexpr size_t kMaxTimeDelta = 256;

			/**
			*  @author chensong
			*  @date 2025-11-09
			*  @brief 处理接收到的RTP包（On RTP Packet）
			*  
			*  该方法在接收到RTP包时被调用，用于记录RTP包的TWCC扩展序列号和接收时间戳。
			*  方法会检查序列号的状态（正常、回绕、跳跃），并在满足条件时触发TWCC反馈的发送。
			*  
			*  处理流程：
			*  1. 调用checkSeqStatus()检查序列号状态
			*  2. 如果序列号跳跃异常，直接返回，丢弃该包
			*  3. 如果序列号回绕，先发送当前收集的TWCC反馈
			*  4. 将序列号和接收时间戳存储到_rtp_recv_status映射表
			*  5. 更新最小和最大时间戳
			*  6. 调用needSendTwcc()判断是否需要发送TWCC反馈
			*  7. 如果需要，调用onSendTwcc()生成并发送反馈报文
			*  
			*  @param ssrc 同步源标识符，用于标识RTP流，会传递给TWCC反馈回调
			*  @param twcc_ext_seq TWCC扩展序列号，从RTP扩展头中提取，范围0-65535
			*  @param stamp_ms 接收时间戳，单位毫秒，表示RTP包到达的时间
			*  @note 如果收到重复的序列号，会记录警告日志并忽略
			*  @note 序列号回绕时会立即触发TWCC反馈发送
			*  @note 该方法不是线程安全的，需要在单线程环境中调用
			*  
			*  使用示例：
			*  @code
			*  // 解析RTP包，提取TWCC扩展序列号
			*  uint16_t twcc_seq = rtp_packet.GetExtension<TransportSequenceNumber>();
			*  uint64_t recv_time = getCurrentTimeMs();
			*  twcc_ctx.onRtp(rtp_packet.Ssrc(), twcc_seq, recv_time);
			*  @endcode
			*/
			void onRtp(uint32_t ssrc, uint16_t twcc_ext_seq, uint64_t stamp_ms);

			/**
			*  @author chensong
			*  @date 2025-11-09
			*  @brief 设置TWCC反馈回调函数（Set TWCC Callback）
			*  
			*  该方法用于设置TWCC反馈报文的回调函数。当生成TWCC反馈报文时，
			*  会通过该回调函数将反馈报文传递给上层模块进行发送。
			*  
			*  @param cb 回调函数，接收ssrc和fci两个参数
			*  @note 必须在调用onRtp()之前设置回调函数，否则反馈报文无法发送
			*  @note 回调函数会在onSendTwcc()中被调用
			*  
			*  使用示例：
			*  @code
			*  twcc_ctx.setOnSendTwccCB([this](uint32_t ssrc, std::string fci) {
			*      // 封装为RTCP包并发送
			*      auto rtcp_packet = createTwccFeedback(ssrc, fci);
			*      sendRtcpPacket(rtcp_packet);
			*  });
			*  @endcode
			*/
			void setOnSendTwccCB(onSendTwccCB cb);

		private:
			/**
			*  @author chensong
			*  @date 2025-11-09
			*  @brief 发送TWCC反馈报文（Send TWCC Feedback）
			*  
			*  该方法用于生成TWCC反馈报文并通过回调函数发送。方法会遍历_rtp_recv_status
			*  映射表，计算每个RTP包的接收状态和时间增量，生成符合RFC 8888标准的FCI数据。
			*  
			*  生成流程：
			*  1. 获取最小和最大序列号，确定反馈范围
			*  2. 计算参考时间戳（Reference Time），精度为64ms
			*  3. 遍历序列号范围，为每个序列号生成包状态
			*  4. 如果包已接收，计算接收时间增量（精度250us）
			*  5. 根据增量大小选择符号状态（小增量或大增量）
			*  6. 如果包未接收，标记为未接收状态
			*  7. 调用FCI_TWCC::create()生成FCI数据
			*  8. 通过回调函数发送FCI数据
			*  9. 调用clearStatus()清空接收状态
			*  
			*  @param ssrc 同步源标识符，传递给回调函数
			*  @note 参考时间戳精度为64ms，通过右移6位实现（除以64）
			*  @note 接收时间增量精度为250us，通过乘以4实现（1ms = 4 * 250us）
			*  @note 如果增量超过255（63.75ms），使用大增量符号状态
			*  @note 反馈包计数器_twcc_pkt_count会自动递增，用于标识反馈顺序
			*/
			void onSendTwcc(uint32_t ssrc);

			/**
			*  @author chensong
			*  @date 2025-11-09
			*  @brief 判断是否需要发送TWCC反馈（Need Send TWCC）
			*  
			*  该方法用于判断当前是否满足发送TWCC反馈的条件。满足以下任一条件时返回true：
			*  1. 收集的RTP包数量达到kMaxSeqSize（20个包）
			*  2. 时间间隔达到kMaxTimeDelta（256ms）
			*  
			*  @return 如果需要发送TWCC反馈返回true，否则返回false
			*  @note 如果_rtp_recv_status为空，返回false
			*  @note 时间间隔通过_max_stamp - _min_stamp计算
			*/
			bool needSendTwcc() const;

			/**
			*  @author chensong
			*  @date 2025-11-09
			*  @brief 检查序列号状态（Check Sequence Status）
			*  
			*  该方法用于检查新接收的TWCC扩展序列号的状态，判断序列号是正常递增、
			*  回绕、还是异常跳跃。返回值对应ExtSeqStatus枚举。
			*  
			*  检查逻辑：
			*  1. 如果_rtp_recv_status为空，返回normal（首个包）
			*  2. 计算新序列号与最大序列号的差值delta
			*  3. 如果delta > 0且 < 0x7FFF，返回normal（正常递增）
			*  4. 如果delta < -0xFF00，返回looped（序列号回绕）
			*  5. 如果delta > 0xFF00，返回jumped（回绕后的乱序包，丢弃）
			*  6. 如果序列号在[min, max]范围内，返回normal（正常乱序）
			*  7. 否则返回jumped（异常跳跃，丢弃）
			*  
			*  @param twcc_ext_seq 待检查的TWCC扩展序列号
			*  @return 返回ExtSeqStatus枚举值：normal(0)、looped(1)、jumped(2)
			*  @note 序列号为16位无符号整数，范围0-65535
			*  @note 使用int32_t计算差值以处理回绕情况
			*  @note 0xFF00阈值用于区分正常乱序和异常跳跃
			*/
			int checkSeqStatus(uint16_t twcc_ext_seq) const;

			/**
			*  @author chensong
			*  @date 2025-11-09
			*  @brief 清空接收状态（Clear Status）
			*  
			*  该方法用于清空当前收集的RTP接收状态，为下一轮TWCC反馈收集做准备。
			*  在发送TWCC反馈报文后调用。
			*  
			*  @note 清空_rtp_recv_status映射表
			*  @note 重置_min_stamp为0
			*  @note _max_stamp和_twcc_pkt_count不需要重置
			*/
			void clearStatus();

		private:
			/**
			*  @brief 最小接收时间戳，单位毫秒
			*  
			*  记录当前收集周期内第一个RTP包的接收时间戳，用于计算时间间隔。
			*  初始值为0，在收到第一个包时设置为该包的时间戳。
			*/
			uint64_t _min_stamp = 0;

			/**
			*  @brief 最大接收时间戳，单位毫秒
			*  
			*  记录当前收集周期内最后一个RTP包的接收时间戳，用于计算时间间隔。
			*  每次收到新包时更新为该包的时间戳。
			*/
			uint64_t _max_stamp;

			/**
			*  @brief RTP接收状态映射表
			*  
			*  存储TWCC扩展序列号到接收时间戳的映射关系。
			*  Key: TWCC扩展序列号（uint32_t，实际为16位）
			*  Value: 接收时间戳（uint64_t，单位毫秒）
			*  
			*  使用std::map自动按序列号排序，方便遍历生成TWCC反馈。
			*/
			std::map<uint32_t /*twcc_ext_seq*/, uint64_t/*recv time in ms*/> _rtp_recv_status;

			/**
			*  @brief TWCC反馈包计数器
			*  
			*  用于标识TWCC反馈报文的顺序，每次发送反馈时递增。
			*  范围0-255，溢出后自动回绕到0。
			*/
			uint8_t _twcc_pkt_count = 0;

			/**
			*  @brief TWCC反馈回调函数
			*  
			*  用于将生成的TWCC反馈报文发送给上层模块。
			*  通过setOnSendTwccCB()方法设置。
			*/
			onSendTwccCB _cb;
		};
	}
}

#endif // 