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

 purpose:		Packet Arrival Time Map Implementation
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
#include "libmedia_transfer_protocol/librtcp/packet_arrival_map.h"
#include <algorithm>
#include <cstdint>

#include "api/units/timestamp.h"
#include "rtc_base/checks.h"

namespace libmedia_transfer_protocol {
    namespace librtcp {
void PacketArrivalTimeMap::AddPacket(int64_t sequence_number,
                                     webrtc::Timestamp arrival_time) {
  RTC_DCHECK_GE(arrival_time, webrtc::Timestamp::Zero());
  if (!has_seen_packet()) {
    // First packet.
    Reallocate(kMinCapacity);
    begin_sequence_number_ = sequence_number;
    end_sequence_number_ = sequence_number + 1;
    arrival_times_[Index(sequence_number)] = arrival_time;
    return;
  }

  if (sequence_number >= begin_sequence_number() &&
      sequence_number < end_sequence_number()) {
    // The packet is within the buffer - no need to expand it.
    arrival_times_[Index(sequence_number)] = arrival_time;
    return;
  }

  if (sequence_number < begin_sequence_number()) {
    // The packet goes before the current buffer. Expand to add packet, but only
    // if it fits within kMaxNumberOfPackets.
    int64_t new_size = end_sequence_number() - sequence_number;
    if (new_size > kMaxNumberOfPackets) {
      // Don't expand the buffer further, as that would remove newly received
      // packets.
      return;
    }
    AdjustToSize(new_size);

    arrival_times_[Index(sequence_number)] = arrival_time;
    SetNotReceived(sequence_number + 1, begin_sequence_number_);
    begin_sequence_number_ = sequence_number;
    return;
  }

  // The packet goes after the buffer.
  RTC_DCHECK_GE(sequence_number, end_sequence_number_);
  int64_t new_end_sequence_number = sequence_number + 1;

  if (new_end_sequence_number >= end_sequence_number_ + kMaxNumberOfPackets) {
    // All old packets have to be removed.
    begin_sequence_number_ = sequence_number;
    end_sequence_number_ = new_end_sequence_number;
    arrival_times_[Index(sequence_number)] = arrival_time;
    return;
  }

  if (begin_sequence_number_ < new_end_sequence_number - kMaxNumberOfPackets) {
    // Remove oldest entries
    begin_sequence_number_ = new_end_sequence_number - kMaxNumberOfPackets;
    RTC_DCHECK_GT(end_sequence_number_, begin_sequence_number_);
  }

  AdjustToSize(new_end_sequence_number - begin_sequence_number_);

  // Packets can be received out-of-order. If this isn't the next expected
  // packet, add enough placeholders to fill the gap.
  SetNotReceived(end_sequence_number_, sequence_number);
  end_sequence_number_ = new_end_sequence_number;
  arrival_times_[Index(sequence_number)] = arrival_time;
}

void PacketArrivalTimeMap::SetNotReceived(
    int64_t begin_sequence_number_inclusive,
    int64_t end_sequence_number_exclusive) {
  static constexpr webrtc::Timestamp value = webrtc::Timestamp::MinusInfinity();

  int begin_index = Index(begin_sequence_number_inclusive);
  int end_index = Index(end_sequence_number_exclusive);

  if (begin_index <= end_index) {
    // Entries to clear are in single block:
    // [......{-----}....]
    std::fill(arrival_times_.get() + begin_index,
              arrival_times_.get() + end_index, value);
  } else {
    // Entries to clear span across arrival_times_ border:
    // [--}..........{---]
    std::fill(arrival_times_.get() + begin_index,
              arrival_times_.get() + capacity(), value);
    std::fill(arrival_times_.get(), arrival_times_.get() + end_index, value);
  }
}

void PacketArrivalTimeMap::RemoveOldPackets(int64_t sequence_number,
    webrtc::Timestamp arrival_time_limit) {
  int64_t check_to = std::min(sequence_number, end_sequence_number_);
  while (begin_sequence_number_ < check_to &&
         arrival_times_[Index(begin_sequence_number_)] <= arrival_time_limit) {
    ++begin_sequence_number_;
  }
  AdjustToSize(end_sequence_number_ - begin_sequence_number_);
}

void PacketArrivalTimeMap::EraseTo(int64_t sequence_number) {
  if (sequence_number < begin_sequence_number_) {
    return;
  }
  if (sequence_number >= end_sequence_number_) {
    // Erase all.
    begin_sequence_number_ = end_sequence_number_;
    return;
  }
  // Remove some.
  begin_sequence_number_ = sequence_number;
  AdjustToSize(end_sequence_number_ - begin_sequence_number_);
}

void PacketArrivalTimeMap::AdjustToSize(int new_size) {
  if (new_size > capacity()) {
    int new_capacity = capacity();
    while (new_capacity < new_size)
      new_capacity *= 2;
    Reallocate(new_capacity);
  }
  if (capacity() > std::max(kMinCapacity, 4 * new_size)) {
    int new_capacity = capacity();
    while (new_capacity > 2 * std::max(new_size, kMinCapacity)) {
      new_capacity /= 2;
    }
    Reallocate(new_capacity);
  }
  RTC_DCHECK_LE(new_size, capacity());
}

void PacketArrivalTimeMap::Reallocate(int new_capacity) {
  int new_capacity_minus_1 = new_capacity - 1;
  // Check capacity is a power of 2.
  RTC_DCHECK_EQ(new_capacity & new_capacity_minus_1, 0);
  // Create uninitialized memory.
  // All valid entries should be set by `AddPacket` before use.
  void* raw = operator new[](new_capacity * sizeof(webrtc::Timestamp));
  webrtc::Timestamp* new_buffer = static_cast<webrtc::Timestamp*>(raw);

  for (int64_t sequence_number = begin_sequence_number_;
       sequence_number < end_sequence_number_; ++sequence_number) {
    new_buffer[sequence_number & new_capacity_minus_1] =
        arrival_times_[sequence_number & capacity_minus_1_];
  }
  arrival_times_.reset(new_buffer);
  capacity_minus_1_ = new_capacity_minus_1;
}
}
}  // namespace  
