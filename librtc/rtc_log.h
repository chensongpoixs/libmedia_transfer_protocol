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
				   date:  2025-11-12




 ******************************************************************************/


#ifndef _C_LIBRTC_LOG____H_
#define _C_LIBRTC_LOG____H_

#include <cstddef>
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"


 
#define MS_TRACE()
#define MS_ERROR PrintE
#define MS_THROW_ERROR(...) do { PrintE(__VA_ARGS__); throw std::runtime_error("MS_THROW_ERROR"); } while(false)
#define MS_DUMP PrintT
#define MS_DEBUG_2TAGS(tag1, tag2, ...) PrintD(__VA_ARGS__)
#define MS_WARN_2TAGS(tag1, tag2, ...) PrintW(__VA_ARGS__)
#define MS_DEBUG_TAG(tag, ...) PrintD(__VA_ARGS__)
#define MS_ASSERT(con, ...) do { if(!(con)) { PrintE(__VA_ARGS__); std::runtime_error("MS_ASSERT"); } } while(false)
#define MS_ABORT(...) do { PrintE(__VA_ARGS__); abort(); } while(false)
#define MS_WARN_TAG(tag, ...) PrintW(__VA_ARGS__)
#define MS_DEBUG_DEV PrintD


#endif // _C_LIBRTC_LOG____H_