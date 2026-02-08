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
created: 		2025-10-22

author:			chensong

purpose:		RTC错误定义（RTC Error Definitions）


RTC错误说明：
- 该文件定义了RTC相关的错误类型和错误处理宏
- 当前版本中，错误类和宏已被注释，使用标准C++异常机制
- 保留此文件用于未来可能的错误处理扩展

错误处理策略：
- 使用std::runtime_error抛出异常
- 使用LIBRTC_LOG记录错误信息
- 错误信息包含详细的上下文和参数

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


#ifndef _C_RTC_ERRORS_H_
#define _C_RTC_ERRORS_H_

#include <cstddef>
 
//#include <cstdint>
//#include <string>
//
//#include <random>
//#include <map>
#include <stdexcept>

namespace libmedia_transfer_protocol {
	namespace librtc {

		/**
		*  @author chensong
		*  @date 2025-10-22
		*  @brief RTC错误类（RTC Error Class）
		*  
		*  该错误类继承自std::runtime_error，用于RTC相关的错误处理。
		*  当前版本中已被注释，使用标准异常机制代替。
		*  
		*  @note 该类已被注释，保留用于未来扩展
		*  @note 使用std::runtime_error作为替代
		*/
		//class RtcError :public std::runtime_error
		//{
		//public:
		//	explicit RtcError(const char* description) : std::runtime_error(description)
		//	{
		//	}
		//
		//public:
		//	static const size_t bufferSize{ 2000 };
		//	static char buffer[bufferSize] ;
		//};


	}
}


// clang-format off
/**
*  @brief 抛出错误宏（Throw Error Macro）
*  
*  该宏用于格式化错误描述并抛出RtcError异常。
*  当前版本中已被注释，使用rtc_log.h中的MS_THROW_ERROR代替。
*  
*  使用方法：
*  MS_THROW_ERROR("Error description: %s", error_msg);
*  
*  @note 该宏已被注释，使用rtc_log.h中的实现
*/
//#define MS_THROW_ERROR(desc, ...) \
//	do \
//	{ \
//		std::snprintf(libmedia_transfer_protocol::librtc::RtcError::buffer, libmedia_transfer_protocol::librtc::RtcError::bufferSize, desc, ##__VA_ARGS__); \
//		throw libmedia_transfer_protocol::librtc::RtcError(libmedia_transfer_protocol::librtc::RtcError::buffer); \
//	} while (false)


#endif 