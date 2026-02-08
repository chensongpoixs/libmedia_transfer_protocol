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

 purpose:		TWCC (Transport Wide Congestion Control) Context Implementation
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
#include "libmedia_transfer_protocol/librtcp/twcc_context.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
#include "libmedia_transfer_protocol/librtcp/rtcp_feedback.h"
namespace libmedia_transfer_protocol
{
	namespace librtcp
	{
        /**
        *  @brief TWCC扩展序列号状态枚举（Extended Sequence Status）
        *  
        *  该枚举用于表示TWCC扩展序列号的状态，帮助判断RTP包的接收顺序。
        *  
        *  @note normal: 序列号正常递增或正常乱序（在合理范围内）
        *  @note looped: 序列号回绕（从65535跳到0），需要立即发送TWCC反馈
        *  @note jumped: 序列号异常跳跃（过大或过小），需要丢弃该包
        */
        enum class ExtSeqStatus : int {
            normal = 0,  // 正常状态
            looped,      // 回绕状态
            jumped,      // 跳跃状态
        };

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
        */
        void TwccContext::onRtp(uint32_t ssrc, uint16_t twcc_ext_seq, uint64_t stamp_ms) {
            // 检查序列号状态并根据不同状态进行处理
            switch ((ExtSeqStatus)checkSeqStatus(twcc_ext_seq)) {
            case ExtSeqStatus::jumped: 
                // 序列号异常跳跃，丢弃该包
                return;
            case ExtSeqStatus::looped: 
                // 序列号回绕，立即发送当前收集的TWCC反馈
                onSendTwcc(ssrc); 
                break;
            case ExtSeqStatus::normal: 
                // 序列号正常，继续处理
                break;
            default: 
                // 不可达的分支，断言失败
                assert(0); 
                break;
            }

            // 将序列号和接收时间戳插入映射表
            auto result = _rtp_recv_status.emplace(twcc_ext_seq, stamp_ms);
            if (!result.second) {
                // 收到重复的序列号，记录警告并返回
                LIBRTCP_LOG_T_F(LS_WARNING) << "recv same twcc ext seq:" << twcc_ext_seq;
                return;
            }

            // 更新最大时间戳
            _max_stamp = result.first->second;
            // 如果是第一个包，初始化最小时间戳
            if (!_min_stamp) {
                _min_stamp = _max_stamp;
            }

            // 判断是否满足发送TWCC反馈的条件
            if (needSendTwcc()) {
                // 满足条件，立即发送TWCC反馈
                onSendTwcc(ssrc);
            }
        }

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
        */
        bool TwccContext::needSendTwcc() const {
            // 如果没有收集到任何RTP包，不需要发送
            if (_rtp_recv_status.empty()) {
                return false;
            }
            // 判断是否达到数量阈值或时间阈值
            return (_rtp_recv_status.size() >= kMaxSeqSize) || (_max_stamp - _min_stamp >= kMaxTimeDelta);
        }

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
        */
        int TwccContext::checkSeqStatus(uint16_t twcc_ext_seq) const {
            // 如果是第一个包，返回正常状态
            if (_rtp_recv_status.empty()) {
                return (int)ExtSeqStatus::normal;
            }
            
            // 获取当前最大序列号
            auto max = _rtp_recv_status.rbegin()->first;
            // 计算序列号差值（使用int32_t以处理回绕）
            auto delta = (int32_t)twcc_ext_seq - (int32_t)max;
            
            // 判断是否为正常递增（差值为正且小于半个序列号空间）
            if (delta > 0 && delta < 0xFFFF / 2) {
                // 正常递增
                return (int)ExtSeqStatus::normal;
            }
            
            // 判断是否为序列号回绕（从65535跳到0附近）
            if (delta < -0xFF00) {
                // 序列号回绕
                LIBRTCP_LOG_T_F(LS_INFO) << "rtp twcc ext seq looped:" << max << " -> " << twcc_ext_seq;
                return (int)ExtSeqStatus::looped;
            }
            
            // 判断是否为回绕后的异常乱序包（差值过大）
            if (delta > 0xFF00) {
                // 回绕后收到前面的乱序包，但差值过大，无法处理，丢弃
                LIBRTCP_LOG_T_F(LS_INFO) << "rtp twcc ext seq jumped after looped:" << max << " -> " << twcc_ext_seq;
                return (int)ExtSeqStatus::jumped;
            }
            
            // 获取当前最小序列号
            auto min = _rtp_recv_status.begin()->first;
            // 判断是否在合理的乱序范围内
            if (min <= twcc_ext_seq || twcc_ext_seq <= max) {
                // 正常乱序（序列号在已收集的范围内）
                return (int)ExtSeqStatus::normal;
            }
            
            // 序列号异常跳跃（增加或减少过大），无法处理，丢弃
            LIBRTCP_LOG_T_F(LS_INFO) << "rtp twcc ext seq jumped:" << max << " -> " << twcc_ext_seq;
            return (int)ExtSeqStatus::jumped;
        }

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
        */
        void TwccContext::onSendTwcc(uint32_t ssrc) {
            // 获取最大序列号
            auto max = _rtp_recv_status.rbegin()->first;
            // 获取最小序列号的迭代器
            auto begin = _rtp_recv_status.begin();
            auto min = begin->first;
            
            // 计算参考时间戳，精度为64ms（右移6位相当于除以64）
            auto ref_time = begin->second >> 6;
            // 还原基准时间戳（左移6位相当于乘以64）
            auto last_time = ref_time << 6;
            
            // 创建包状态映射表
            FCI_TWCC::TwccPacketStatus status;
            
            // 遍历序列号范围[min, max]
            for (auto seq = min; seq <= max; ++seq) {
                int16_t delta = 0;
                SymbolStatus symbol = SymbolStatus::not_received;
                
                // 查找当前序列号是否已接收
                auto it = _rtp_recv_status.find(seq);
                if (it != _rtp_recv_status.end()) {
                    // 包已接收，计算接收时间增量
                    // 精度为250us，1ms等于4x250us（乘以4）
                    delta = (int16_t)(4 * ((int64_t)it->second - (int64_t)last_time));
                    
                    // 根据增量大小选择符号状态
                    if (delta < 0 || delta > 0xFF) {
                        // 增量超过255（63.75ms），使用大增量符号
                        symbol = SymbolStatus::large_delta;
                    }
                    else {
                        // 增量在0-255范围内，使用小增量符号
                        symbol = SymbolStatus::small_delta;
                    }
                    
                    // 更新上次接收时间
                    last_time = it->second;
                }
                // 如果包未接收，symbol保持为not_received，delta为0
                
                // 将包状态添加到映射表
                status.emplace(seq, std::make_pair(symbol, delta));
            }
            
            // 生成TWCC反馈报文的FCI数据
            auto fci = FCI_TWCC::create(ref_time, _twcc_pkt_count++, status);
            
            // 通过回调函数发送FCI数据
            if (_cb) {
                _cb(ssrc, std::move(fci));
            }
            
            // 清空接收状态，准备下一轮收集
            clearStatus();
        }

        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 清空接收状态（Clear Status）
        *  
        *  该方法用于清空当前收集的RTP接收状态，为下一轮TWCC反馈收集做准备。
        *  在发送TWCC反馈报文后调用。
        */
        void TwccContext::clearStatus() {
            // 清空RTP接收状态映射表
            _rtp_recv_status.clear();
            // 重置最小时间戳
            _min_stamp = 0;
        }

        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 设置TWCC反馈回调函数（Set TWCC Callback）
        *  
        *  该方法用于设置TWCC反馈报文的回调函数。当生成TWCC反馈报文时，
        *  会通过该回调函数将反馈报文传递给上层模块进行发送。
        *  
        *  @param cb 回调函数，接收ssrc和fci两个参数
        */
        void TwccContext::setOnSendTwccCB(TwccContext::onSendTwccCB cb) {
            // 使用移动语义保存回调函数
            _cb = std::move(cb);
        }
	}
}