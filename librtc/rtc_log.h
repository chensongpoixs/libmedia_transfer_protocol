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
created: 		2025-11-12

author:			chensong

purpose:		RTC日志宏定义（RTC Log Macros）


RTC日志说明：
- 该文件定义了RTC模块使用的日志宏
- 所有日志宏都映射到libmedia_transfer_protocol的日志系统
- 提供多种日志级别：TRACE、DEBUG、WARN、ERROR等
- 支持带标签的日志输出

日志宏说明（Log Macro Descriptions）：

1. MS_TRACE() - 跟踪日志，记录函数调用和执行流程
2. MS_ERROR - 错误日志，记录错误信息
3. MS_THROW_ERROR - 记录错误并抛出异常
4. MS_DUMP - 转储日志，记录详细的数据信息
5. MS_DEBUG_2TAGS - 带两个标签的调试日志
6. MS_WARN_2TAGS - 带两个标签的警告日志
7. MS_DEBUG_TAG - 带标签的调试日志
8. MS_ASSERT - 断言宏，条件不满足时记录错误并抛出异常
9. MS_ABORT - 致命错误宏，记录错误并终止程序
10. MS_WARN_TAG - 带标签的警告日志
11. MS_DEBUG_DEV - 开发调试日志

使用示例：
  MS_TRACE();  // 记录当前函数调用
  MS_ERROR("Connection failed: %s", error_msg);
  MS_DEBUG_TAG("DTLS", "Handshake completed");


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


#ifndef _C_LIBRTC_LOG____H_
#define _C_LIBRTC_LOG____H_

#include <cstddef>
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"


 
/**
*  @brief 跟踪日志宏（Trace Log Macro）
*  
*  记录函数调用和执行流程，自动包含文件名、行号、函数名。
*  
*  使用示例：
*  MS_TRACE();  // 输出：[INFO] [file:line] function_name()
*/
#define MS_TRACE()   LIBRTC_LOG_F(LS_INFO)

/**
*  @brief 错误日志宏（Error Log Macro）
*  
*  记录错误信息，支持格式化输出。
*  
*  使用示例：
*  MS_ERROR("Connection failed: %s", error_msg);
*/
#define MS_ERROR PrintE

/**
*  @brief 抛出错误宏（Throw Error Macro）
*  
*  记录错误信息并抛出std::runtime_error异常。
*  
*  使用示例：
*  MS_THROW_ERROR("Invalid parameter: %d", param);
*/
#define MS_THROW_ERROR(...) do { PrintE(__VA_ARGS__); throw std::runtime_error("MS_THROW_ERROR"); } while(false)

/**
*  @brief 转储日志宏（Dump Log Macro）
*  
*  记录详细的数据信息，用于调试和分析。
*  
*  使用示例：
*  MS_DUMP("Packet data: %02x %02x %02x", data[0], data[1], data[2]);
*/
#define MS_DUMP PrintT

/**
*  @brief 带两个标签的调试日志宏（Debug with 2 Tags）
*  
*  记录带两个标签的调试信息。
*  
*  使用示例：
*  MS_DEBUG_2TAGS("DTLS", "HANDSHAKE", "Starting handshake");
*/
#define MS_DEBUG_2TAGS(tag1, tag2, ...) PrintD(__VA_ARGS__)

/**
*  @brief 带两个标签的警告日志宏（Warning with 2 Tags）
*  
*  记录带两个标签的警告信息。
*  
*  使用示例：
*  MS_WARN_2TAGS("SRTP", "DECRYPT", "Decryption failed");
*/
#define MS_WARN_2TAGS(tag1, tag2, ...) PrintW(__VA_ARGS__)

/**
*  @brief 带标签的调试日志宏（Debug with Tag）
*  
*  记录带标签的调试信息。
*  
*  使用示例：
*  MS_DEBUG_TAG("SDP", "Parsing offer: %s", sdp);
*/
#define MS_DEBUG_TAG(tag, ...) PrintD(__VA_ARGS__)

/**
*  @brief 断言宏（Assert Macro）
*  
*  检查条件，如果条件不满足则记录错误并抛出异常。
*  
*  使用示例：
*  MS_ASSERT(ptr != nullptr, "Pointer is null");
*/
#define MS_ASSERT(con, ...) do { if(!(con)) { PrintE(__VA_ARGS__); std::runtime_error("MS_ASSERT"); } } while(false)

/**
*  @brief 致命错误宏（Abort Macro）
*  
*  记录致命错误并终止程序。
*  
*  使用示例：
*  MS_ABORT("Unrecoverable error: %s", error_msg);
*/
#define MS_ABORT(...) do { PrintE(__VA_ARGS__); abort(); } while(false)

/**
*  @brief 带标签的警告日志宏（Warning with Tag）
*  
*  记录带标签的警告信息。
*  
*  使用示例：
*  MS_WARN_TAG("ICE", "Connection timeout");
*/
#define MS_WARN_TAG(tag, ...) PrintW(__VA_ARGS__)

/**
*  @brief 开发调试日志宏（Development Debug Macro）
*  
*  记录开发过程中的调试信息。
*  
*  使用示例：
*  MS_DEBUG_DEV("Testing feature X: result=%d", result);
*/
#define MS_DEBUG_DEV PrintD


#endif // _C_LIBRTC_LOG____H_