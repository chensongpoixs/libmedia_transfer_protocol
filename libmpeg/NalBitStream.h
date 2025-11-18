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
				   date:  2025-10-09



 ******************************************************************************/

#ifndef _C_NAL_BIT_STREAM_H_
#define _C_NAL_BIT_STREAM_H_
#include <cstdint>
namespace libmedia_transfer_protocol
{
    namespace libmpeg
    {
        class NalBitStream 
        {
        public:
            NalBitStream(const char *data, int len);
            uint8_t GetBit();
            uint16_t GetWord(int bits);
            uint32_t GetBitLong(int bits);
            uint64_t GetBit64(int bits);
			//¸çÂ×²¼±àÂë
            uint32_t GetUE();
			// ÓÐ·ûºÅµÄ
            int32_t GetSE();
        private:
            char GetByte();
            const char * data_;
            int len_;
            int bits_count_;
            int byte_idx_;
            char byte_;
        };
    }
}


#endif // _C_NAL_BIT_STREAM_H_