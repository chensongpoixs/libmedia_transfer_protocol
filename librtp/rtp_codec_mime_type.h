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
                  date:  2025-11-13

                TODO: 2025-11-13 chensong   rtcp

******************************************************************************/


#ifndef _LIBRTP_RTP_CODEC_MIME_TYPE_H_
#define _LIBRTP_RTP_CODEC_MIME_TYPE_H_

#include <cstdint>


namespace libmedia_transfer_protocol
{
    namespace librtp {




        typedef enum {
            TrackInvalid = -1,
            TrackAudio = 0,
            TrackVideo, 
            TrackTitle,
            TrackApplication,
            TrackMax
        } TrackType;

#define CODEC_MAP(XX) \
    XX(CodecH264,  TrackVideo, 0, "H264")          \
    XX(CodecH265,  TrackVideo, 1, "H265")          \
    XX(CodecAAC,   TrackAudio, 2, "mpeg4-generic")   \
    XX(CodecG711A, TrackAudio, 3, "PCMA")  \
    XX(CodecG711U, TrackAudio, 4, "PCMU")  \
    XX(CodecOpus,  TrackAudio, 5, "opus")    \
    XX(CodecL16,   TrackAudio, 6, "L16")       \
    XX(CodecVP8,   TrackVideo, 7, "VP8")             \
    XX(CodecVP9,   TrackVideo, 8, "VP9")             \
    XX(CodecAV1,   TrackVideo, 9, "AV1")             \
    XX(CodecJPEG,  TrackVideo, 10, "JPEG")    \
    XX(CodecH266,  TrackVideo, 11, "H266")         \
    XX(CodecTS,    TrackVideo, 12, "MP2T")     \
    XX(CodecPS,    TrackVideo, 13, "MPEG")     \
    XX(CodecMP3,   TrackAudio, 14, "MP3")           \
    XX(CodecADPCM, TrackAudio, 15, "ADPCM")    \
    XX(CodecSVACV, TrackVideo, 16, "SVACV")  \
    XX(CodecSVACA, TrackAudio, 17, "SVACA")  \
    XX(CodecG722,  TrackAudio, 18, "G722")   \
    XX(CodecG723,  TrackAudio, 19, "G723")   \
    XX(CodecG728,  TrackAudio, 20, "G728")     \
    XX(CodecG729,  TrackAudio, 21, "G729")
     
        typedef enum {
            CodecInvalid = -1,
#define XX(name, type, value, str) name = value,
            CODEC_MAP(XX)
#undef XX
            CodecMax
        } CodecId;
        /*
        X_H264UC,
			H265,
			// Complementary codecs:
			CN = 300,
			TELEPHONE_EVENT,
			// Feature codecs:
			RTX = 400,
			ULPFEC,
			X_ULPFECUC,
			FLEXFEC,
			RED
        
        */
        class RtpCodecMimeType
        {
        public: 
            TrackType       track_type_{ TrackInvalid };
            CodecId         sub_type_{ CodecInvalid };
        };


    }
}


#endif // _LIBRTP_RTP_CODEC_MIME_TYPE_H_