

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
 

#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
#include <cstdarg>
#include <stdarg.h>
namespace libmedia_transfer_protocol
{
	namespace {
		static const unsigned int LOG_BUF_MAX_SIZE = 1024 * 50; // default 1024 * 1024 statck win too small


		#define LIBSCTP_LOG(sev, file_, line_)                        \
			!rtc::LogMessage::IsNoop<::rtc::sev>() && \
				RTC_LOG_FILE_LINE(::rtc::sev, file_, line_)
	}


	void printLog(rtc::LoggingSeverity level, const char* file, const char* function, int line, const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		printLogV(  level, file, function, line, fmt, ap);
		va_end(ap);
	}
	void printLogV(rtc::LoggingSeverity level, const char* file, const char* func, int line, const char* fmt, va_list ap)
	{

		int32_t cnt = -1;
		char buffer[LOG_BUF_MAX_SIZE] = { 0 };
		cnt = vsnprintf(buffer, LOG_BUF_MAX_SIZE, fmt, ap);
		if (cnt <= 0)
		{
			return;
		}
		 
		switch (level)
		{
		case rtc::LS_VERBOSE:
			LIBSCTP_LOG(LS_VERBOSE, file, line) << "[libsctp]" << func << buffer;
			break;
		case rtc::LS_INFO:
			LIBSCTP_LOG(LS_INFO, file, line) << "[libsctp]" << func << buffer;
			break;
		case rtc::LS_WARNING:
			LIBSCTP_LOG(LS_WARNING, file, line) << "[libsctp]" << func << buffer;
			break;
		case rtc::LS_ERROR:
			LIBSCTP_LOG(LS_ERROR, file, line) << "[libsctp]" << func << buffer;
			break;
		case rtc::LS_NONE:
			LIBSCTP_LOG(LS_NONE, file, line) << "[libsctp]" << func << buffer;
			break;  
		default:
			break;
		}
		
	}

}