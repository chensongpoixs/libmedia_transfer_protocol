

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



 ******************************************************************************/

#ifndef _LIBMEDIA_TRANSFER_PROTOCOL_LOG_H_
#define _LIBMEDIA_TRANSFER_PROTOCOL_LOG_H_
#include "rtc_base/logging.h"
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 





 // librtc mobule
#define  LIBRTC_LOG(sev)  RTC_LOG(sev)  << "[librtc]"
#define  LIBRTC_LOG_F(sev)  RTC_LOG_F(sev) << "[librtc]"
#define  LIBRTC_LOG_T_F(sev)  RTC_LOG_T_F(sev) << "[librtc]"


 // libssl mobule
#define  LIBSSL_LOG(sev)  RTC_LOG(sev)  << "[libssl]"
#define  LIBSSL_LOG_F(sev)  RTC_LOG_F(sev) << "[libssl]"
#define  LIBSSL_LOG_T_F(sev)  RTC_LOG_T_F(sev) << "[libssl]"

// libsrtp mobule
#define  LIBSRTP_LOG(sev)  RTC_LOG(sev)  << "[libsrtp]"
#define  LIBSRTP_LOG_F(sev)  RTC_LOG_F(sev) << "[libsrtp]"
#define  LIBSRTP_LOG_T_F(sev)  RTC_LOG_T_F(sev) << "[libsrtp]"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// libtcp mobule
#define  LIBNETWORK_LOG(sev) RTC_LOG(sev)  << "[libnetwork]"
#define  LIBNETWORK_LOG_F(sev) RTC_LOG_F(sev)<< "[libnetwork]"
#define  LIBNETWORK_LOG_T_F(sev)  RTC_LOG_T_F(sev)<< "[libnetwork]"
// libhttp mobule
#define  LIBHTTP_LOG(sev) RTC_LOG(sev)  << "[libhttp]"
#define  LIBHTTP_LOG_F(sev) RTC_LOG_F(sev)<< "[libhttp]"
#define  LIBHTTP_LOG_T_F(sev)  RTC_LOG_T_F(sev)<< "[libhttp]"
// libmpeg mobule
#define  LIBMPEG_LOG(sev) RTC_LOG(sev)  << "[libmpeg]"
#define  LIBMPEG_LOG_F(sev) RTC_LOG_F(sev)<< "[libmpeg]"
#define  LIBMPEG_LOG_T_F(sev)  RTC_LOG_T_F(sev)<< "[libmpeg]"
// libflv mobule
#define  LIBFLV_LOG(sev) RTC_LOG(sev)  << "[libflv]"
#define  LIBFLV_LOG_F(sev) RTC_LOG_F(sev)<< "[libflv]"
#define  LIBFLV_LOG_T_F(sev)  RTC_LOG_T_F(sev)<< "[libflv]"
// librtcp 
#define  LIBRTCP_LOG(sev) RTC_LOG(sev)  << "[librtcp]"
#define  LIBRTCP_LOG_F(sev) RTC_LOG_F(sev)<< "[librtcp]"
#define  LIBRTCP_LOG_T_F(sev)  RTC_LOG_T_F(sev)<< "[librtcp]"
// libmuxer
#define  LIBMUXER_LOG(sev) RTC_LOG(sev)  << "[libmuxer]"
#define  LIBMUXER_LOG_F(sev) RTC_LOG_F(sev)<< "[libmuxer]"
#define  LIBMUXER_LOG_T_F(sev)  RTC_LOG_T_F(sev)<< "[libmuxer]"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





namespace libmedia_transfer_protocol
{
	  void printLog(rtc::LoggingSeverity level, const char* file, const char* function, int line, const char* fmt, ...);
	  void printLogV(rtc::LoggingSeverity level, const char* file, const char* function, int line, const char* fmt, va_list ap);

}

//ÓÃ·¨: PrintD("%d + %s = %c", 1 "2", 'c');  [AUTO-TRANSLATED:1217cc82]
//Usage: PrintD("%d + %s = %c", 1, "2", 'c');
#define PrintLog(level, ...) ::libmedia_transfer_protocol::printLog(  level, __FILE__, FUNCTION, __LINE__, ##__VA_ARGS__)


#define PrintT(...) PrintLog(rtc::LS_VERBOSE, ##__VA_ARGS__)
#define PrintD(...) PrintLog(rtc::LS_INFO, ##__VA_ARGS__)
#define PrintI(...) PrintLog(rtc::LS_INFO, ##__VA_ARGS__)
#define PrintW(...) PrintLog(rtc::LS_WARNING, ##__VA_ARGS__)
#define PrintE(...) PrintLog(rtc::LS_ERROR, ##__VA_ARGS__)

 
#endif // _LIBMEDIA_TRANSFER_PROTOCOL_LOG_H_