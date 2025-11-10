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

#include "libmedia_transfer_protocol/librtcp/buffer.h"

#include <cstdlib>
 

namespace libmedia_transfer_protocol {
    namespace librtcp { 
StatisticImp(Buffer)
StatisticImp(BufferRaw)
StatisticImp(BufferLikeString)

BufferRaw::Ptr BufferRaw::create() {
#if 0
    static ResourcePool<BufferRaw> packet_pool;
    static onceToken token([]() {
        packet_pool.setSize(1024);
    });
    auto ret = packet_pool.obtain2();
    ret->setSize(0);
    return ret;
#else
    return Ptr(new BufferRaw);
#endif
}
    }
}//namespace  
