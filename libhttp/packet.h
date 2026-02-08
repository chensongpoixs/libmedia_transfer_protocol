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
				   date:  2025-10-08

【HTTP数据包类文件】

本文件定义了HTTP数据包的封装类Packet，用于管理HTTP消息体数据。

【核心功能】
1. 数据包内存管理：动态分配和释放数据包内存
2. 数据包类型标识：区分视频、音频、元数据等不同类型
3. 时间戳管理：记录数据包的时间戳信息
4. 扩展数据支持：通过ext_字段支持附加任意类型的扩展数据

【数据包结构】
┌─────────────────────────────────────┐
│  Packet对象（sizeof(Packet)字节）   │
│  ├─ type_: 数据包类型               │
│  ├─ size_: 实际数据大小             │
│  ├─ index_: 数据包索引              │
│  ├─ timestamp_: 时间戳              │
│  ├─ capacity_: 容量                 │
│  └─ ext_: 扩展数据指针              │
├─────────────────────────────────────┤
│  数据区（capacity_字节）            │
│  实际的HTTP消息体数据               │
└─────────────────────────────────────┘

【使用场景】
1. HTTP响应体缓存：存储从服务器接收的响应数据
2. HTTP请求体构造：构造要发送的请求数据
3. 流媒体数据传输：传输视频、音频数据包
4. 分块传输：支持HTTP chunked编码

【使用示例】
@code
// 创建一个1024字节的数据包
auto packet = Packet::NewPacket(1024);

// 设置数据包类型为视频关键帧
packet->SetPacketType(kPacketTypeVideo | kFrameTypeKeyFrame);

// 设置时间戳
packet->SetTimeStamp(12345678);

// 写入数据
memcpy(packet->Data(), video_data, video_size);
packet->SetPacketSize(video_size);

// 检查数据包类型
if (packet->IsKeyFrame()) {
    // 处理关键帧
}
@endcode

【注意事项】
1. 使用NewPacket创建数据包，不要直接new Packet
2. 数据包通过shared_ptr管理，自动释放内存
3. Data()返回的指针指向Packet对象之后的内存区域
4. 写入数据后必须调用SetPacketSize或UpdatePacketSize更新大小

【作者的思考】
数据包的设计采用了"对象+数据"的内存布局，将元数据和实际数据连续存储。
这种设计减少了内存分配次数，提高了缓存命中率，是高性能网络编程的常用技巧。

 ******************************************************************************/

#ifndef _C_LIBHTTP_PACKET_H_
#define _C_LIBHTTP_PACKET_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"

#include "libmedia_transfer_protocol/libhttp/http_type.h"
#include <string>
#include <unordered_map>
#include <iostream>
 //#include <ctype.h>
#include <cstdint>
#include <vector>
#include <sstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <cassert>


#include <algorithm>


#include <assert.h>
#include <memory>

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		/**
		 * @brief 数据包类型标志位
		 * 
		 * 这些标志可以通过位或运算组合使用，例如：
		 * kPacketTypeVideo | kFrameTypeKeyFrame 表示视频关键帧
		 */
		enum
		{
			kPacketTypeVideo = 1,        // 视频数据包
			kPacketTypeAudio = 2,        // 音频数据包
			kPacketTypeMeta = 4,         // 元数据包
			kPacketTypeMeta3 = 8,        // 元数据包（扩展）
			kFrameTypeKeyFrame = 16,     // 关键帧标志（用于视频）
			kFrameTypeIDR = 32,          // IDR帧标志（H.264关键帧）
			kPacketTypeUnknowed = 255,   // 未知类型
		};
		
		class Packet;
		
#pragma pack(push)
#pragma pack(1)
		/**
		 * @class Packet
		 * @brief HTTP数据包封装类
		 * 
		 * 用于封装HTTP消息体数据，支持多种数据类型（视频、音频、元数据等）。
		 * 数据包采用"对象+数据"的内存布局，对象头部存储元信息，紧随其后是实际数据。
		 * 
		 * 【内存布局】
		 * ┌──────────────────┐ ← Packet对象起始地址
		 * │  type_           │   数据包类型
		 * │  size_           │   实际数据大小
		 * │  index_          │   数据包索引
		 * │  timestamp_      │   时间戳
		 * │  capacity_       │   数据区容量
		 * │  ext_            │   扩展数据指针
		 * ├──────────────────┤ ← Data()返回的地址
		 * │                  │
		 * │  实际数据区      │   capacity_字节
		 * │                  │
		 * └──────────────────┘
		 * 
		 * 【类型组合示例】
		 * - 视频关键帧：kPacketTypeVideo | kFrameTypeKeyFrame
		 * - 视频IDR帧：kPacketTypeVideo | kFrameTypeIDR
		 * - 普通音频帧：kPacketTypeAudio
		 * 
		 * 【线程安全】
		 * 本类不是线程安全的，多线程访问需要外部同步。
		 */
		class Packet
		{
		public:
			/**
			 * @brief 构造函数
			 * @param size 数据区大小（字节）
			 * 
			 * 注意：不要直接使用构造函数创建Packet，应使用NewPacket静态方法
			 */
			Packet(int32_t size)
				:capacity_(size)
			{

			}
			~Packet() {}
			
			/**
			 * @brief 创建新的数据包
			 * @param size 数据区大小（字节）
			 * @return 数据包智能指针
			 * 
			 * 分配连续内存：sizeof(Packet) + size
			 * 使用自定义删除器确保正确释放内存
			 * 
			 * 示例：
			 * @code
			 * auto packet = Packet::NewPacket(1024);
			 * @endcode
			 */
			static std::shared_ptr<Packet> NewPacket(int32_t size);

			/**
			 * @brief 判断是否为视频数据包
			 * @return true表示是视频数据包
			 */
			bool IsVideo() const
			{
				return (type_&kPacketTypeVideo) == kPacketTypeVideo;
			}
			
			/**
			 * @brief 判断是否为关键帧
			 * @return true表示是视频关键帧
			 * 
			 * 关键帧（I帧）是视频解码的起点，不依赖其他帧即可独立解码
			 */
			bool IsKeyFrame() const
			{
				return ((type_&kPacketTypeVideo) == kPacketTypeVideo)
					&& (type_&kFrameTypeKeyFrame) == kFrameTypeKeyFrame;
			}
			
			/**
			 * @brief 判断是否为音频数据包
			 * @return true表示是音频数据包
			 */
			bool IsAudio() const
			{
				return type_ == kPacketTypeAudio;
			}
			
			/**
			 * @brief 判断是否为元数据包
			 * @return true表示是元数据包
			 */
			bool IsMeta() const
			{
				return type_ == kPacketTypeMeta;
			}
			
			/**
			 * @brief 判断是否为扩展元数据包
			 * @return true表示是扩展元数据包
			 */
			bool IsMeta3() const
			{
				return type_ == kPacketTypeMeta3;
			}

			/**
			 * @brief 获取数据包实际大小
			 * @return 实际数据大小（字节）
			 */
			inline int32_t PacketSize() const
			{
				return size_;
			}
			
			/**
			 * @brief 获取剩余空间大小
			 * @return 剩余可写入的字节数
			 */
			inline int Space() const
			{
				return capacity_ - size_;
			}
			
			/**
			 * @brief 设置数据包大小
			 * @param len 新的数据大小
			 * 
			 * 注意：len不能超过capacity_
			 */
			inline void SetPacketSize(size_t len)
			{
				size_ = len;
			}
			
			/**
			 * @brief 增加数据包大小
			 * @param len 要增加的字节数
			 * 
			 * 用于追加数据后更新大小：size_ += len
			 */
			inline void UpdatePacketSize(size_t len)
			{
				size_ += len;
			}
			
			/**
			 * @brief 设置数据包索引
			 * @param index 索引值
			 * 
			 * 索引可用于标识数据包在序列中的位置
			 */
			void SetIndex(int32_t index)
			{
				index_ = index;
			}
			
			/**
			 * @brief 获取数据包索引
			 * @return 索引值
			 */
			int32_t Index() const
			{
				return index_;
			}
			
			/**
			 * @brief 设置数据包类型
			 * @param type 类型标志（可以是多个标志的位或组合）
			 * 
			 * 示例：
			 * @code
			 * packet->SetPacketType(kPacketTypeVideo | kFrameTypeKeyFrame);
			 * @endcode
			 */
			void SetPacketType(int32_t type)
			{
				type_ = type;
			}
			
			/**
			 * @brief 获取数据包类型
			 * @return 类型标志
			 */
			int32_t PacketType() const
			{
				return type_;
			}
			
			/**
			 * @brief 设置时间戳
			 * @param timestamp 时间戳（毫秒）
			 */
			void SetTimeStamp(uint64_t timestamp)
			{
				timestamp_ = timestamp;
			}
			
			/**
			 * @brief 获取时间戳
			 * @return 时间戳（毫秒）
			 */
			uint64_t TimeStamp() const
			{
				return timestamp_;
			}
			
			/**
			 * @brief 获取数据区指针
			 * @return 指向数据区的指针
			 * 
			 * 数据区紧跟在Packet对象之后，地址为：(char*)this + sizeof(Packet)
			 */
			inline char *Data()
			{
				return (char*)this + sizeof(Packet);
			}

			/**
			 * @brief 获取扩展数据
			 * @tparam T 扩展数据类型
			 * @return 扩展数据智能指针
			 * 
			 * 扩展数据可以是任意类型，用于附加额外信息
			 */
			template <typename T>
			inline std::shared_ptr<T> Ext() const
			{
				return std::static_pointer_cast<T>(ext_);
			}
			
			/**
			 * @brief 设置扩展数据
			 * @param ext 扩展数据智能指针
			 */
			inline void SetExt(const std::shared_ptr<void> &ext)
			{
				ext_ = ext;
			}

		private:
			int32_t type_{ kPacketTypeUnknowed };   // 数据包类型标志
			uint32_t size_{ 0 };                    // 实际数据大小
			int32_t index_{ -1 };                   // 数据包索引（-1表示未设置）
			uint64_t timestamp_{ 0 };               // 时间戳（毫秒）
			uint32_t capacity_{ 0 };                // 数据区容量
			std::shared_ptr<void> ext_;             // 扩展数据（可选）
		};
#pragma pack()        
	}
}

#endif // _C_LIBHTTP_PACKET_H_