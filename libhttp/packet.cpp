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

【HTTP数据包实现文件】

本文件实现了Packet类的工厂方法NewPacket，用于创建数据包对象。

【内存分配策略】
采用"对象+数据"的连续内存布局：
1. 分配 sizeof(Packet) + size 字节的连续内存
2. 在内存起始位置构造Packet对象（placement new）
3. Packet对象之后紧跟数据区

这种设计的优点：
- 减少内存分配次数（一次分配完成）
- 提高缓存命中率（对象和数据连续存储）
- 简化内存管理（一次释放完成）

【自定义删除器】
使用lambda表达式作为shared_ptr的删除器：
- 将Packet*转换为char*进行delete[]
- 确保正确释放通过new char[]分配的内存

【作者的思考】
这种内存布局技巧在高性能网络编程中很常见，类似于Linux内核中的sk_buff设计。
通过将元数据和实际数据连续存储，可以显著提高内存访问效率。

 ******************************************************************************/

 
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
#include "libmedia_transfer_protocol/libhttp/packet.h"

#include <algorithm>
#include "libmedia_transfer_protocol/libhttp/packet.h"

#include <assert.h>

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		/**
		 * @brief 创建新的数据包
		 * @param size 数据区大小（字节）
		 * @return 数据包智能指针
		 * 
		 * 【实现细节】
		 * 1. 计算总内存大小：block_size = size + sizeof(Packet)
		 * 2. 分配连续内存：new char[block_size]
		 * 3. 在内存起始位置构造Packet对象（placement new）
		 * 4. 初始化Packet成员变量
		 * 5. 返回带自定义删除器的shared_ptr
		 * 
		 * 【内存布局】
		 * ┌────────────────────┐ ← packet指针
		 * │  Packet对象        │   sizeof(Packet)字节
		 * ├────────────────────┤ ← packet->Data()
		 * │  数据区            │   size字节
		 * └────────────────────┘
		 * 
		 * 【自定义删除器】
		 * lambda表达式 [](Packet *p) { delete[](char*)p; }
		 * - 将Packet*转换为char*
		 * - 使用delete[]释放内存（因为是通过new char[]分配的）
		 * 
		 * 【使用示例】
		 * @code
		 * // 创建1KB数据包
		 * auto packet = Packet::NewPacket(1024);
		 * 
		 * // 写入数据
		 * memcpy(packet->Data(), data, data_size);
		 * packet->SetPacketSize(data_size);
		 * 
		 * // shared_ptr自动管理生命周期，无需手动释放
		 * @endcode
		 */
		std::shared_ptr<Packet> Packet::NewPacket(int32_t size)
		{
			// 计算总内存大小：对象大小 + 数据区大小
			auto block_size = size + sizeof(Packet);
			
			// 分配连续内存块
			Packet * packet = (Packet*)new char[block_size];
			
			// 初始化内存为0
			memset((void*)packet, 0x00, block_size);
			
			// 初始化Packet成员变量
			packet->index_ = -1;                      // 索引初始化为-1（未设置）
			packet->type_ = kPacketTypeUnknowed;      // 类型初始化为未知
			packet->capacity_ = size;                 // 设置数据区容量
			packet->ext_.reset();                     // 扩展数据初始化为空

			// 返回带自定义删除器的shared_ptr
			// 删除器将Packet*转换为char*后使用delete[]释放
			return std::shared_ptr<Packet>(packet, [](Packet *p) {
				delete[](char*)p;
			});
		}
	}
}