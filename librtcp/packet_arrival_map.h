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
        template <typename T>
        T clamp(T v, T lo, T hi) {
            return std::min(std::max(v, lo), hi);
        }
    class PacketArrivalTimeMap {
    public:
        struct PacketArrivalTime {
            webrtc::Timestamp arrival_time;
            int64_t sequence_number;
        };
       
        static constexpr int kMaxNumberOfPackets = (1 << 15);

        PacketArrivalTimeMap() = default;
        PacketArrivalTimeMap(const PacketArrivalTimeMap&) = delete;
        PacketArrivalTimeMap& operator=(const PacketArrivalTimeMap&) = delete;
        ~PacketArrivalTimeMap() = default;
         
        bool has_received(int64_t sequence_number) const {
            return sequence_number >= begin_sequence_number() &&
                sequence_number < end_sequence_number() &&
                arrival_times_[Index(sequence_number)] >= webrtc::Timestamp::Zero();
        }
         
        int64_t begin_sequence_number() const { return begin_sequence_number_; }
         
        int64_t end_sequence_number() const { return end_sequence_number_; }
         
        webrtc::Timestamp get(int64_t sequence_number) {
            RTC_DCHECK_GE(sequence_number, begin_sequence_number());
            RTC_DCHECK_LT(sequence_number, end_sequence_number());
            return arrival_times_[Index(sequence_number)];
        }
         
        PacketArrivalTime FindNextAtOrAfter(int64_t sequence_number) const {
            RTC_DCHECK_GE(sequence_number, begin_sequence_number());
            RTC_DCHECK_LT(sequence_number, end_sequence_number());
            while (true) {
                webrtc::Timestamp t = arrival_times_[Index(sequence_number)];
                if (t >= webrtc::Timestamp::Zero()) 
                {
                    //struct PacketArrivalTime packet;

                    return {   t,   sequence_number };
                }
                ++sequence_number;
            }
        }
         
        int64_t clamp(int64_t sequence_number) const {
            /*return std::clamp(sequence_number, begin_sequence_number(),
                end_sequence_number());*/
            return libmedia_transfer_protocol::librtcp::clamp(sequence_number, begin_sequence_number(),
                end_sequence_number());
        }
         
        void EraseTo(int64_t sequence_number);
         
        void AddPacket(int64_t sequence_number, webrtc::Timestamp arrival_time);
         
        void RemoveOldPackets(int64_t sequence_number, webrtc::Timestamp arrival_time_limit);

    private:
        static constexpr int kMinCapacity = 128;

        int Index(int64_t sequence_number) const {
            // Note that sequence_number might be negative, thus taking '%' requires
            // extra handling and can be slow. Because capacity is a power of two, it
            // is much faster to use '&' operator.
            return sequence_number & capacity_minus_1_;
        }

        void SetNotReceived(int64_t begin_sequence_number_inclusive,
            int64_t end_sequence_number_exclusive);

        // Adjust capacity to match new_size, may reduce capacity.
        // On return guarantees capacity >= new_size.
        void AdjustToSize(int new_size);
        void Reallocate(int new_capacity);

        int capacity() const { return capacity_minus_1_ + 1; }
        bool has_seen_packet() const { return arrival_times_ != nullptr; }

        // Circular buffer. Packet with sequence number `sequence_number`
        // is stored in the slot `sequence_number % capacity_`
        std::unique_ptr<webrtc::Timestamp[]> arrival_times_ = nullptr;

        // Allocated size of the `arrival_times_`
        // capacity_ is a power of 2 in range [kMinCapacity, kMaxNumberOfPackets]
        // `capacity - 1` is used much more often than `capacity`, thus that value is
        // stored.
        int capacity_minus_1_ = -1;

        // The unwrapped sequence number for valid range of sequence numbers.
        // arrival_times_ entries only valid for sequence numbers in range
        // `begin_sequence_number_ <= sequence_number < end_sequence_number_`
        int64_t begin_sequence_number_ = 0;
        int64_t end_sequence_number_ = 0;
    };
}
}  // namespace  

#endif  // _LIBMEDIA_LIBRTCP_PACKET_ARRIVAL_MAP_H_
