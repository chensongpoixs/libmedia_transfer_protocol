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
        // Class used when send-side BWE is enabled.
        // The class is responsible for generating RTCP feedback packets based on
        // incoming media packets. Incoming packets must have a transport sequence
        // number, Ie. either the extension
        // http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01 or
        // http://www.webrtc.org/experiments/rtp-hdrext/transport-wide-cc-02 must be
        // used.
#if 1
        class TransportSequenceNumberFeedback
        {
         public:
           TransportSequenceNumberFeedback( );
          virtual ~TransportSequenceNumberFeedback();

          void OnReceivedPacket(const libmedia_transfer_protocol::RtpPacketReceived& packet)  ;
          void OnSendBandwidthEstimateChanged(webrtc::DataRate estimate)  ;

          void Process(int64_t  now_ms)  ;

         private:
          void MaybeCullOldPackets(int64_t sequence_number, int64_t arrival_time) ;
          void SendPeriodicFeedbacks()  ;
          void SendFeedbackOnRequest(int64_t sequence_number,
                                     const FeedbackRequest& feedback_request) ;

           
          std::unique_ptr<rtcp::TransportFeedback> MaybeBuildFeedbackPacket(
              bool include_timestamps,
              int64_t begin_sequence_number_inclusive,
              int64_t end_sequence_number_exclusive,
              bool is_periodic_update)  ;
   
         // webrtc::Timestamp last_process_time_;
          int64_t   last_process_time_;
          uint32_t media_ssrc_  ;
          uint8_t feedback_packet_count_  ;
          webrtc::SeqNumUnwrapper<uint16_t> unwrapper_  ;
   
          //std::optional<int64_t> periodic_window_start_seq_  ;
          int64_t periodic_window_start_seq_;
          // Packet arrival times, by sequence number.
          webrtc::PacketArrivalTimeMap packet_arrival_times_  ;

        //  webrtc::TimeDelta send_interval_  ;
          int64_t     send_interval_;
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