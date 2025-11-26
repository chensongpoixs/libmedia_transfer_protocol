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

purpose:		RTC工具函数（RTC Utility Functions）


RTC工具函数说明：
- 该文件提供了RTC模块使用的工具函数
- 主要是字节序转换函数（网络字节序与主机字节序的转换）
- 支持1、2、3、4、8字节的整数类型
- 支持4字节对齐的填充计算

字节序说明（Byte Order）：
- 网络字节序（Network Byte Order）：大端字节序（Big-Endian）
- 主机字节序（Host Byte Order）：根据CPU架构不同（x86为小端Little-Endian）
- RTP/RTCP等网络协议使用网络字节序
- 需要在网络字节序和主机字节序之间进行转换

字节序示例（Byte Order Example）：
  数值: 0x12345678
  
  大端字节序（Big-Endian，网络字节序）：
    [12] [34] [56] [78]
     高地址 -> 低地址
  
  小端字节序（Little-Endian，x86主机字节序）：
    [78] [56] [34] [12]
     低地址 -> 高地址


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


#ifndef _C_LIBRTC_UTILS_H_
#define _C_LIBRTC_UTILS_H_
#if 0
//#if defined(_WIN32)
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#pragma comment (lib, "Ws2_32.lib")
//#else
//#include <arpa/inet.h>
//#endif // defined(_WIN32)
#endif // 
#include <cinttypes>// PRIu64, etc
#include <cstddef>// size_t
#include <cstdint>// uint8_t, etc

namespace libmedia_transfer_protocol
{
	namespace librtc
	{

        /**
        *  @author chensong
        *  @date 2025-11-12
        *  @brief 字节工具类（Byte Utility Class）
        *  
        *  Byte类提供了字节序转换的工具函数。所有Getter函数从网络字节序（大端）
        *  读取数据并返回主机字节序；所有Setter函数将主机字节序的值写入为网络字节序。
        *  
        *  主要功能：
        *  1. 读取1/2/3/4/8字节整数（网络字节序 -> 主机字节序）
        *  2. 写入1/2/3/4/8字节整数（主机字节序 -> 网络字节序）
        *  3. 计算4字节对齐的填充大小
        *  
        *  @note 所有函数都是inline静态函数，性能高效
        *  @note Getters: 网络字节序（大端）-> 主机字节序
        *  @note Setters: 主机字节序 -> 网络字节序（大端）
        *  
        *  使用示例：
        *  @code
        *  uint8_t data[4] = {0x12, 0x34, 0x56, 0x78};
        *  
        *  // 读取2字节（网络字节序）
        *  uint16_t val = Byte::Get2Bytes(data, 0);  // val = 0x1234
        *  
        *  // 写入4字节（转换为网络字节序）
        *  Byte::Set4Bytes(data, 0, 0x12345678);
        *  // data = {0x12, 0x34, 0x56, 0x78}
        *  @endcode
        */
        class Byte {
        public:
            /**
             * Getters below get value in Host Byte Order.
             * Setters below set value in Network Byte Order.
             */
            
            /** 读取1字节（无需转换） */
            static uint8_t Get1Byte(const uint8_t* data, size_t i);
            
            /** 读取2字节（网络字节序 -> 主机字节序） */
            static uint16_t Get2Bytes(const uint8_t* data, size_t i);
            
            /** 读取3字节（网络字节序 -> 主机字节序） */
            static uint32_t Get3Bytes(const uint8_t* data, size_t i);
            
            /** 读取4字节（网络字节序 -> 主机字节序） */
            static uint32_t Get4Bytes(const uint8_t* data, size_t i);
            
            /** 读取8字节（网络字节序 -> 主机字节序） */
            static uint64_t Get8Bytes(const uint8_t* data, size_t i);
            
            /** 写入1字节（无需转换） */
            static void Set1Byte(uint8_t* data, size_t i, uint8_t value);
            
            /** 写入2字节（主机字节序 -> 网络字节序） */
            static void Set2Bytes(uint8_t* data, size_t i, uint16_t value);
            
            /** 写入3字节（主机字节序 -> 网络字节序） */
            static void Set3Bytes(uint8_t* data, size_t i, uint32_t value);
            
            /** 写入4字节（主机字节序 -> 网络字节序） */
            static void Set4Bytes(uint8_t* data, size_t i, uint32_t value);
            
            /** 写入8字节（主机字节序 -> 网络字节序） */
            static void Set8Bytes(uint8_t* data, size_t i, uint64_t value);
            
            /** 计算4字节对齐的填充大小（uint16_t版本） */
            static uint16_t PadTo4Bytes(uint16_t size);
            
            /** 计算4字节对齐的填充大小（uint32_t版本） */
            static uint32_t PadTo4Bytes(uint32_t size);
        };

        /* Inline static methods. */
        
        /**
        *  @brief 读取1字节
        *  
        *  从指定位置读取1字节数据。1字节无需字节序转换。
        *  
        *  @param data 数据缓冲区指针
        *  @param i 读取位置偏移量
        *  @return 返回读取的uint8_t值
        */
        inline uint8_t Byte::Get1Byte(const uint8_t* data, size_t i) { return data[i]; }

        inline uint16_t Byte::Get2Bytes(const uint8_t* data, size_t i) {
            return uint16_t{ data[i + 1] } | uint16_t{ data[i] } << 8;
        }

        inline uint32_t Byte::Get3Bytes(const uint8_t* data, size_t i) {
            return uint32_t{ data[i + 2] } | uint32_t{ data[i + 1] } << 8 | uint32_t{ data[i] } << 16;
        }

        inline uint32_t Byte::Get4Bytes(const uint8_t* data, size_t i) {
            return uint32_t{ data[i + 3] } | uint32_t{ data[i + 2] } << 8 | uint32_t{ data[i + 1] } << 16 |
                uint32_t{ data[i] } << 24;
        }

        inline uint64_t Byte::Get8Bytes(const uint8_t* data, size_t i) {
            return uint64_t{ Byte::Get4Bytes(data, i) } << 32 | Byte::Get4Bytes(data, i + 4);
        }

        inline void Byte::Set1Byte(uint8_t* data, size_t i, uint8_t value) { data[i] = value; }

        inline void Byte::Set2Bytes(uint8_t* data, size_t i, uint16_t value) {
            data[i + 1] = static_cast<uint8_t>(value);
            data[i] = static_cast<uint8_t>(value >> 8);
        }

        inline void Byte::Set3Bytes(uint8_t* data, size_t i, uint32_t value) {
            data[i + 2] = static_cast<uint8_t>(value);
            data[i + 1] = static_cast<uint8_t>(value >> 8);
            data[i] = static_cast<uint8_t>(value >> 16);
        }

        inline void Byte::Set4Bytes(uint8_t* data, size_t i, uint32_t value) {
            data[i + 3] = static_cast<uint8_t>(value);
            data[i + 2] = static_cast<uint8_t>(value >> 8);
            data[i + 1] = static_cast<uint8_t>(value >> 16);
            data[i] = static_cast<uint8_t>(value >> 24);
        }

        inline void Byte::Set8Bytes(uint8_t* data, size_t i, uint64_t value) {
            data[i + 7] = static_cast<uint8_t>(value);
            data[i + 6] = static_cast<uint8_t>(value >> 8);
            data[i + 5] = static_cast<uint8_t>(value >> 16);
            data[i + 4] = static_cast<uint8_t>(value >> 24);
            data[i + 3] = static_cast<uint8_t>(value >> 32);
            data[i + 2] = static_cast<uint8_t>(value >> 40);
            data[i + 1] = static_cast<uint8_t>(value >> 48);
            data[i] = static_cast<uint8_t>(value >> 56);
        }

        /**
        *  @brief 计算4字节对齐的填充大小
        *  
        *  如果size不是4字节（32位）的倍数，则向上对齐到4字节边界。
        *  
        *  @param size 原始大小
        *  @return 返回对齐后的大小
        *  
        *  示例：
        *  - PadTo4Bytes(5) = 8
        *  - PadTo4Bytes(8) = 8
        *  - PadTo4Bytes(10) = 12
        */
        inline uint16_t Byte::PadTo4Bytes(uint16_t size) {
            // If size is not multiple of 32 bits then pad it.
            if (size & 0x03)
            {
                return (size & 0xFFFC) + 4;
            }
            else
            {
                return size;
            }
        }
	}
}

#endif // _C_LIBRTC_UTILS_H_
