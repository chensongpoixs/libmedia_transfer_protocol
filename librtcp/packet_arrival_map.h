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

 purpose:		Packet Arrival Time Map for TWCC
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

#ifndef _LIBMEDIA_LIBRTCP_PACKET_ARRIVAL_MAP_H_
#define _LIBMEDIA_LIBRTCP_PACKET_ARRIVAL_MAP_H_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include "absl/algorithm/algorithm.h"
#include "api/units/timestamp.h"
#include "rtc_base/checks.h"

namespace libmedia_transfer_protocol {
    namespace librtcp {
        /**
        *  @brief 通用范围限制函数（Clamp Function）
        *  
        *  将值限制在指定的范围内。如果值小于最小值，返回最小值；
        *  如果值大于最大值，返回最大值；否则返回原值。
        *  
        *  @tparam T 值的类型
        *  @param v 待限制的值
        *  @param lo 最小值
        *  @param hi 最大值
        *  @return 限制后的值
        */
        template <typename T>
        T clamp(T v, T lo, T hi) {
            return std::min(std::max(v, lo), hi);
        }

        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 数据包到达时间映射表（Packet Arrival Time Map）
        *  
        *  PacketArrivalTimeMap是用于TWCC（传输层拥塞控制）的核心数据结构，
        *  负责记录RTP包的到达时间。该类使用循环缓冲区存储包的到达时间戳，
        *  支持高效的序列号查询和范围管理。
        *  
        *  设计特点：
        *  - 使用循环缓冲区（Circular Buffer）存储到达时间
        *  - 容量为2的幂次方，支持快速取模运算（使用位运算）
        *  - 动态调整容量，根据数据量自动扩容或缩容
        *  - 支持序列号回绕（Sequence Number Wrap-around）
        *  - 最大支持32768个包（kMaxNumberOfPackets）
        *  
        *  循环缓冲区原理：
        *  - 序列号通过取模映射到缓冲区索引：index = seq % capacity
        *  - 由于capacity是2的幂次方，可以优化为：index = seq & (capacity - 1)
        *  - 当序列号回绕时（65535 -> 0），映射关系仍然正确
        *  
        *  容量管理：
        *  - 初始容量：kMinCapacity（128）
        *  - 最大容量：kMaxNumberOfPackets（32768）
        *  - 扩容策略：当数据量超过容量时，容量翻倍
        *  - 缩容策略：当数据量小于容量的1/4时，容量减半
        *  
        *  使用场景：
        *  - TWCC反馈生成：记录RTP包的到达时间，用于生成TWCC反馈报文
        *  - 丢包检测：通过序列号范围判断哪些包未到达
        *  - 延迟计算：通过到达时间计算网络延迟
        *  
        *  @note 该类不是线程安全的，需要外部同步
        *  @note 序列号使用64位整数，避免回绕问题
        *  @note 到达时间使用webrtc::Timestamp，精度为微秒
        *  
        *  使用示例：
        *  @code
        *  PacketArrivalTimeMap arrival_map;
        *  
        *  // 添加包到达记录
        *  int64_t seq = 12345;
        *  webrtc::Timestamp arrival_time = webrtc::Timestamp::Millis(1000);
        *  arrival_map.AddPacket(seq, arrival_time);
        *  
        *  // 查询包是否已接收
        *  if (arrival_map.has_received(seq)) {
        *      webrtc::Timestamp time = arrival_map.get(seq);
        *  }
        *  
        *  // 查找下一个已接收的包
        *  auto next = arrival_map.FindNextAtOrAfter(seq);
        *  
        *  // 删除旧包
        *  arrival_map.EraseTo(seq - 1000);
        *  @endcode
        */
    class PacketArrivalTimeMap {
    public:
        /**
        *  @brief 包到达时间结构体
        *  
        *  用于返回包的到达时间和序列号信息。
        */
        struct PacketArrivalTime {
            webrtc::Timestamp arrival_time;  // 到达时间戳
            int64_t sequence_number;         // 序列号
        };
       
        /**
        *  @brief 最大包数量限制
        *  
        *  循环缓冲区支持的最大包数量，设置为2^15（32768）。
        *  这是一个合理的上限，既能满足大部分场景，又不会占用过多内存。
        */
        static constexpr int kMaxNumberOfPackets = (1 << 15);

        /**
        *  @brief 默认构造函数
        */
        PacketArrivalTimeMap() = default;

        /**
        *  @brief 禁用拷贝构造函数
        */
        PacketArrivalTimeMap(const PacketArrivalTimeMap&) = delete;

        /**
        *  @brief 禁用拷贝赋值运算符
        */
        PacketArrivalTimeMap& operator=(const PacketArrivalTimeMap&) = delete;

        /**
        *  @brief 析构函数
        */
        ~PacketArrivalTimeMap() = default;
         
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 判断包是否已接收（Has Received）
        *  
        *  检查指定序列号的包是否已经被记录。只有在序列号范围内且
        *  到达时间有效（>= Timestamp::Zero()）时才返回true。
        *  
        *  @param sequence_number 待查询的序列号
        *  @return 如果包已接收返回true，否则返回false
        */
        bool has_received(int64_t sequence_number) const {
            return sequence_number >= begin_sequence_number() &&
                sequence_number < end_sequence_number() &&
                arrival_times_[Index(sequence_number)] >= webrtc::Timestamp::Zero();
        }
         
        /**
        *  @brief 获取起始序列号
        *  @return 当前记录的最小序列号
        */
        int64_t begin_sequence_number() const { return begin_sequence_number_; }
         
        /**
        *  @brief 获取结束序列号
        *  @return 当前记录的最大序列号+1（左闭右开区间）
        */
        int64_t end_sequence_number() const { return end_sequence_number_; }
         
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 获取包的到达时间（Get Arrival Time）
        *  
        *  返回指定序列号的包的到达时间戳。调用前必须确保序列号在有效范围内。
        *  
        *  @param sequence_number 序列号
        *  @return 包的到达时间戳
        *  @note 调用前应使用has_received()检查包是否存在
        */
        webrtc::Timestamp get(int64_t sequence_number) {
            RTC_DCHECK_GE(sequence_number, begin_sequence_number());
            RTC_DCHECK_LT(sequence_number, end_sequence_number());
            return arrival_times_[Index(sequence_number)];
        }
         
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 查找下一个已接收的包（Find Next At Or After）
        *  
        *  从指定序列号开始，查找第一个已接收的包，返回其到达时间和序列号。
        *  该方法会一直向后查找，直到找到有效的包。
        *  
        *  @param sequence_number 起始序列号
        *  @return 包到达时间结构体，包含到达时间和序列号
        *  @note 调用前必须确保起始序列号在有效范围内
        *  @note 该方法假设一定能找到有效的包，否则会无限循环
        */
        PacketArrivalTime FindNextAtOrAfter(int64_t sequence_number) const {
            RTC_DCHECK_GE(sequence_number, begin_sequence_number());
            RTC_DCHECK_LT(sequence_number, end_sequence_number());
            while (true) {
                webrtc::Timestamp t = arrival_times_[Index(sequence_number)];
                if (t >= webrtc::Timestamp::Zero()) 
                {
                    return {   t,   sequence_number };
                }
                ++sequence_number;
            }
        }
         
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 将序列号限制在有效范围内（Clamp Sequence Number）
        *  
        *  将序列号限制在[begin_sequence_number, end_sequence_number]范围内。
        *  如果序列号小于起始值，返回起始值；如果大于结束值，返回结束值。
        *  
        *  @param sequence_number 待限制的序列号
        *  @return 限制后的序列号
        */
        int64_t clamp(int64_t sequence_number) const {
            return libmedia_transfer_protocol::librtcp::clamp(sequence_number, begin_sequence_number(),
                end_sequence_number());
        }
         
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 删除指定序列号之前的包（Erase To）
        *  
        *  删除所有序列号小于指定值的包，释放内存空间。
        *  该方法用于清理旧数据，避免内存无限增长。
        *  
        *  @param sequence_number 删除边界（不包含该序列号）
        */
        void EraseTo(int64_t sequence_number);
         
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 添加包到达记录（Add Packet）
        *  
        *  记录指定序列号的包的到达时间。如果序列号超出当前范围，
        *  会自动扩展缓冲区。如果缓冲区已满，会删除最旧的记录。
        *  
        *  处理逻辑：
        *  1. 如果是第一个包，初始化缓冲区
        *  2. 如果序列号在当前范围内，直接更新
        *  3. 如果序列号在范围之前，向前扩展（如果不超过最大限制）
        *  4. 如果序列号在范围之后，向后扩展，可能删除旧记录
        *  
        *  @param sequence_number 包的序列号
        *  @param arrival_time 包的到达时间戳
        *  @note 到达时间必须 >= Timestamp::Zero()
        */
        void AddPacket(int64_t sequence_number, webrtc::Timestamp arrival_time);
         
        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 删除旧包（Remove Old Packets）
        *  
        *  删除所有到达时间早于指定时间限制的包。该方法用于基于时间的
        *  数据清理，确保只保留最近的数据。
        *  
        *  @param sequence_number 检查的最大序列号
        *  @param arrival_time_limit 时间限制，早于该时间的包会被删除
        */
        void RemoveOldPackets(int64_t sequence_number, webrtc::Timestamp arrival_time_limit);

    private:
        /**
        *  @brief 最小容量
        *  
        *  循环缓冲区的最小容量，设置为128。即使数据量很小，
        *  也会保持这个最小容量，避免频繁的内存分配。
        */
        static constexpr int kMinCapacity = 128;

        /**
        *  @brief 计算循环缓冲区索引（Calculate Index）
        *  
        *  将序列号映射到循环缓冲区的索引。使用位运算优化取模操作。
        *  
        *  @param sequence_number 序列号
        *  @return 缓冲区索引
        *  @note 序列号可能为负数，但位运算仍然正确
        *  @note capacity_minus_1_ = capacity - 1，用于位运算优化
        */
        int Index(int64_t sequence_number) const {
            // 注意：sequence_number可能为负数，因此使用'%'需要额外处理且速度较慢
            // 由于capacity是2的幂次方，使用'&'运算符更快
            return sequence_number & capacity_minus_1_;
        }

        /**
        *  @brief 设置未接收状态（Set Not Received）
        *  
        *  将指定范围内的所有序列号标记为未接收状态（MinusInfinity）。
        *  
        *  @param begin_sequence_number_inclusive 起始序列号（包含）
        *  @param end_sequence_number_exclusive 结束序列号（不包含）
        */
        void SetNotReceived(int64_t begin_sequence_number_inclusive,
            int64_t end_sequence_number_exclusive);

        /**
        *  @brief 调整缓冲区大小（Adjust To Size）
        *  
        *  根据新的数据量调整缓冲区容量。如果数据量增加，扩容；
        *  如果数据量减少，可能缩容以节省内存。
        *  
        *  @param new_size 新的数据量
        */
        void AdjustToSize(int new_size);

        /**
        *  @brief 重新分配缓冲区（Reallocate）
        *  
        *  分配新的缓冲区并复制现有数据。新容量必须是2的幂次方。
        *  
        *  @param new_capacity 新的容量
        */
        void Reallocate(int new_capacity);

        /**
        *  @brief 获取当前容量
        *  @return 缓冲区容量
        */
        int capacity() const { return capacity_minus_1_ + 1; }

        /**
        *  @brief 判断是否已接收过包
        *  @return 如果已接收过至少一个包返回true
        */
        bool has_seen_packet() const { return arrival_times_ != nullptr; }

        /**
        *  @brief 循环缓冲区
        *  
        *  存储包到达时间的数组。序列号sequence_number的包存储在
        *  索引sequence_number % capacity的位置。
        */
        std::unique_ptr<webrtc::Timestamp[]> arrival_times_ = nullptr;

        /**
        *  @brief 缓冲区容量减1
        *  
        *  存储capacity - 1的值，用于位运算优化取模操作。
        *  capacity是2的幂次方，范围[kMinCapacity, kMaxNumberOfPackets]。
        *  由于capacity - 1使用频率远高于capacity，因此直接存储该值。
        */
        int capacity_minus_1_ = -1;

        /**
        *  @brief 起始序列号
        *  
        *  当前有效序列号范围的起始值（包含）。
        *  arrival_times_中的有效条目对应序列号范围
        *  [begin_sequence_number_, end_sequence_number_)
        */
        int64_t begin_sequence_number_ = 0;

        /**
        *  @brief 结束序列号
        *  
        *  当前有效序列号范围的结束值（不包含）。
        *  使用左闭右开区间[begin_sequence_number_, end_sequence_number_)
        */
        int64_t end_sequence_number_ = 0;
    };
}
}  // namespace  

#endif  // _LIBMEDIA_LIBRTCP_PACKET_ARRIVAL_MAP_H_
