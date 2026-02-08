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

【HTTP消息缓冲区类文件】

本文件定义了MsgBuffer类，用于管理HTTP消息的接收和发送缓冲区。

【核心功能】
1. 动态缓冲区管理：自动扩展缓冲区大小
2. 数据读写操作：支持字节、整数、字符串的读写
3. 网络字节序转换：自动处理大小端转换
4. 高效内存管理：通过头尾指针避免频繁的内存拷贝

【缓冲区结构】
┌─────────────────────────────────────────────────────┐
│  预留空间  │  可读数据区  │  可写空间              │
│ (kBufferOffset) │ (head_→tail_) │ (tail_→end)    │
├────────────┼──────────────┼────────────────────────┤
│  0         │  head_       │  tail_        capacity │
└─────────────────────────────────────────────────────┘

预留空间(kBufferOffset=8字节)的作用：
- 允许在数据前面添加头部信息（AddInFront）
- 避免在头部插入数据时需要移动整个缓冲区

【使用场景】
1. HTTP请求解析：接收TCP数据流，解析HTTP请求
2. HTTP响应构造：构造HTTP响应消息
3. 流式数据处理：处理大文件上传/下载
4. 协议解析：查找CRLF、解析分块编码等

【使用示例】
@code
// 创建缓冲区
MsgBuffer buffer(1024);

// 追加数据
buffer.Append("GET / HTTP/1.1\r\n", 16);

// 查找CRLF
const char* crlf = buffer.FindCRLF();

// 读取数据
std::string line = buffer.Read(crlf - buffer.Peek());
buffer.RetrieveUntil(crlf + 2);

// 读取整数（网络字节序）
uint32_t value = buffer.ReadInt32();
@endcode

【性能优化】
1. 预留头部空间：避免头部插入时的数据移动
2. 延迟内存分配：只在需要时才扩展缓冲区
3. 内存复用：通过移动数据而非重新分配来腾出空间
4. 自动收缩：当缓冲区过大时自动收缩到初始大小

【作者的思考】
缓冲区设计是网络编程的核心。一个好的缓冲区设计应该：
1. 最小化内存拷贝次数
2. 支持高效的头部和尾部操作
3. 自动管理内存大小
4. 提供便捷的数据访问接口

MsgBuffer的设计借鉴了muduo网络库的Buffer类，通过头尾指针和预留空间
实现了高效的缓冲区管理。

 ******************************************************************************/

#ifndef _C_LIBHTTP_MSG_BUFFER_H_
#define _C_LIBHTTP_MSG_BUFFER_H_

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

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		static constexpr size_t kBufferDefaultLength{ 2048 };  // 默认缓冲区大小：2KB
		

		/**
		 * @class MsgBuffer
		 * @brief HTTP消息缓冲区类
		 * 
		 * 用于管理HTTP消息的接收和发送缓冲区，支持动态扩展、高效读写和网络字节序转换。
		 * 
		 * 【缓冲区布局】
		 * ┌──────────────────────────────────────────────────────┐
		 * │ 预留区 │    可读数据区    │      可写空间           │
		 * │ (8字节) │  (head_→tail_)  │   (tail_→capacity)     │
		 * ├────────┼──────────────────┼─────────────────────────┤
		 * │   0    │     head_        │    tail_       size     │
		 * └──────────────────────────────────────────────────────┘
		 * 
		 * 【关键指针】
		 * - head_: 可读数据的起始位置
		 * - tail_: 可写空间的起始位置（也是可读数据的结束位置）
		 * - buffer_.size(): 缓冲区总容量
		 * 
		 * 【空间计算】
		 * - 可读字节数：tail_ - head_
		 * - 可写字节数：buffer_.size() - tail_
		 * - 预留空间：head_ (初始为kBufferOffset=8)
		 * 
		 * 【自动扩展策略】
		 * 1. 如果可写空间不足，先尝试移动数据到缓冲区前部
		 * 2. 如果移动后仍不足，则分配新的更大缓冲区
		 * 3. 新缓冲区大小为当前大小的2倍或所需大小（取较大值）
		 * 
		 * 【线程安全】
		 * 本类不是线程安全的，多线程访问需要外部同步。
		 */
		class MsgBuffer
		{
		public:
			/**
			 * @brief 构造函数
			 * @param len 初始缓冲区大小（不包括预留空间）
			 * 
			 * 实际分配的内存大小为：len + kBufferOffset
			 * 初始状态：head_ = tail_ = kBufferOffset
			 */
			MsgBuffer(size_t len = kBufferDefaultLength);

			/**
			 * @brief 获取可读数据的起始指针
			 * @return 指向可读数据起始位置的指针
			 * 
			 * 返回的指针指向第一个可读字节
			 */
			const char *Peek() const
			{
				return begin() + head_;
			}

			/**
			 * @brief 获取可写空间的起始指针（const版本）
			 * @return 指向可写空间起始位置的指针
			 * 
			 * 返回的指针指向第一个可写字节
			 */
			const char *BeginWrite() const
			{
				return begin() + tail_;
			}
			
			/**
			 * @brief 获取可写空间的起始指针（非const版本）
			 * @return 指向可写空间起始位置的指针
			 */
			char *BeginWrite()
			{
				return begin() + tail_;
			}

			/**
			 * @brief 查看一个字节（不移除）
			 * @return 8位无符号整数
			 * 
			 * 注意：不会移动head_指针，数据仍在缓冲区中
			 */
			uint8_t PeekInt8() const
			{
				assert(ReadableBytes() >= 1);
				return *(static_cast<const uint8_t *>((void *)Peek()));
			}

			/**
			 * @brief 查看一个16位整数（不移除）
			 * @return 16位无符号整数（主机字节序）
			 * 
			 * 自动将网络字节序（大端）转换为主机字节序
			 */
			uint16_t PeekInt16() const;

			/**
			 * @brief 查看一个32位整数（不移除）
			 * @return 32位无符号整数（主机字节序）
			 * 
			 * 自动将网络字节序（大端）转换为主机字节序
			 */
			uint32_t PeekInt32() const;

			/**
			 * @brief 查看一个64位整数（不移除）
			 * @return 64位无符号整数（主机字节序）
			 * 
			 * 自动将网络字节序（大端）转换为主机字节序
			 */
			uint64_t PeekInt64() const;

			/**
			 * @brief 读取并移除指定长度的数据
			 * @param len 要读取的字节数
			 * @return 读取的数据字符串
			 * 
			 * 如果len大于可读字节数，则只读取可读部分
			 * 读取后会移动head_指针
			 */
			std::string Read(size_t len);

			/**
			 * @brief 读取并移除一个字节
			 * @return 8位无符号整数
			 */
			uint8_t ReadInt8();

			/**
			 * @brief 读取并移除一个16位整数
			 * @return 16位无符号整数（主机字节序）
			 * 
			 * 自动将网络字节序转换为主机字节序
			 */
			uint16_t ReadInt16();

			/**
			 * @brief 读取并移除一个32位整数
			 * @return 32位无符号整数（主机字节序）
			 * 
			 * 自动将网络字节序转换为主机字节序
			 */
			uint32_t ReadInt32();

			/**
			 * @brief 读取并移除一个64位整数
			 * @return 64位无符号整数（主机字节序）
			 * 
			 * 自动将网络字节序转换为主机字节序
			 */
			uint64_t ReadInt64();

			/**
			 * @brief 与另一个缓冲区交换内容
			 * @param buf 要交换的缓冲区
			 * 
			 * 高效交换两个缓冲区的内容，不涉及数据拷贝
			 */
			void Swap(MsgBuffer &buf) noexcept;

			/**
			 * @brief 获取可读字节数
			 * @return 可读字节数
			 * 
			 * 计算公式：tail_ - head_
			 */
			size_t ReadableBytes() const
			{
				return tail_ - head_;
			}

			/**
			 * @brief 获取可写字节数
			 * @return 可写字节数
			 * 
			 * 计算公式：buffer_.size() - tail_
			 */
			size_t WritableBytes() const
			{
				return buffer_.size() - tail_;
			}

			/**
			 * @brief 追加数据到缓冲区末尾
			 * @param buf 另一个MsgBuffer对象
			 * 
			 * 将buf中的所有可读数据追加到当前缓冲区
			 */
			void Append(const MsgBuffer &buf);
			
			/**
			 * @brief 追加字符数组到缓冲区（模板版本）
			 * @tparam N 数组大小
			 * @param buf 字符数组
			 * 
			 * 注意：数组必须以'\0'结尾，实际追加N-1个字节
			 */
			template <int N>
			void Append(const char(&buf)[N])
			{
				assert(strnlen(buf, N) == N - 1);
				Append(buf, N - 1);
			}
			
			/**
			 * @brief 追加指定长度的数据到缓冲区
			 * @param buf 数据指针
			 * @param len 数据长度
			 * 
			 * 如果可写空间不足，会自动扩展缓冲区
			 */
			void Append(const char *buf, size_t len);
			
			/**
			 * @brief 追加字符串到缓冲区
			 * @param buf 字符串对象
			 */
			void Append(const std::string &buf)
			{
				Append(buf.c_str(), buf.length());
			}

			/**
			 * @brief 追加一个字节到缓冲区末尾
			 * @param b 字节值
			 */
			void AppendInt8(const uint8_t b)
			{
				Append(static_cast<const char *>((void *)&b), 1);
			}

			/**
			 * @brief 追加一个16位整数到缓冲区末尾
			 * @param s 16位整数（主机字节序）
			 * 
			 * 自动转换为网络字节序（大端）后追加
			 */
			void AppendInt16(const uint16_t s);

			/**
			 * @brief 追加一个32位整数到缓冲区末尾
			 * @param i 32位整数（主机字节序）
			 * 
			 * 自动转换为网络字节序（大端）后追加
			 */
			void AppendInt32(const uint32_t i);

			/**
			 * @brief 追加一个64位整数到缓冲区末尾
			 * @param l 64位整数（主机字节序）
			 * 
			 * 自动转换为网络字节序（大端）后追加
			 */
			void AppendInt64(const uint64_t l);

			/**
			 * @brief 在缓冲区头部插入数据
			 * @param buf 数据指针
			 * @param len 数据长度
			 * 
			 * 利用预留空间在数据前面插入内容
			 * 如果预留空间不足，会移动或扩展缓冲区
			 */
			void AddInFront(const char *buf, size_t len);

			/**
			 * @brief 在缓冲区头部插入一个字节
			 * @param b 字节值
			 */
			void AddInFrontInt8(const uint8_t b)
			{
				AddInFront(static_cast<const char *>((void *)&b), 1);
			}

			/**
			 * @brief 在缓冲区头部插入一个16位整数
			 * @param s 16位整数（主机字节序）
			 * 
			 * 自动转换为网络字节序后插入
			 */
			void AddInFrontInt16(const uint16_t s);

			/**
			 * @brief 在缓冲区头部插入一个32位整数
			 * @param i 32位整数（主机字节序）
			 * 
			 * 自动转换为网络字节序后插入
			 */
			void AddInFrontInt32(const uint32_t i);

			/**
			 * @brief 在缓冲区头部插入一个64位整数
			 * @param l 64位整数（主机字节序）
			 * 
			 * 自动转换为网络字节序后插入
			 */
			void AddInFrontInt64(const uint64_t l);

			/**
			 * @brief 清空缓冲区所有数据
			 * 
			 * 重置head_和tail_到初始位置
			 * 如果缓冲区过大（超过初始容量的2倍），会收缩到初始大小
			 */
			void RetrieveAll();

			/**
			 * @brief 移除指定长度的数据
			 * @param len 要移除的字节数
			 * 
			 * 移动head_指针，标记数据已被读取
			 * 如果len >= ReadableBytes()，则清空所有数据
			 */
			void Retrieve(size_t len);

			/**
			 * @brief 移除指定位置之前的所有数据
			 * @param end 结束位置指针
			 * 
			 * 移除从Peek()到end之间的数据
			 * end必须在[Peek(), BeginWrite()]范围内
			 */
			void RetrieveUntil(const char *end)
			{
				assert(Peek() <= end);
				assert(end <= BeginWrite());
				Retrieve(end - Peek());
			}

			/**
			 * @brief 查找CRLF（\r\n）的位置
			 * @return CRLF的起始位置指针，未找到返回NULL
			 * 
			 * 用于解析HTTP协议的行结束符
			 * 
			 * 示例：
			 * @code
			 * const char* crlf = buffer.FindCRLF();
			 * if (crlf) {
			 *     std::string line(buffer.Peek(), crlf);
			 *     buffer.RetrieveUntil(crlf + 2);
			 * }
			 * @endcode
			 */
			const char *FindCRLF() const;

			/**
			 * @brief 确保缓冲区有足够的可写空间
			 * @param len 需要的可写字节数
			 * 
			 * 如果可写空间不足，会：
			 * 1. 先尝试移动数据到缓冲区前部腾出空间
			 * 2. 如果仍不足，则分配新的更大缓冲区
			 */
			void EnsureWritableBytes(size_t len);

			/**
			 * @brief 标记已写入数据
			 * @param len 已写入的字节数
			 * 
			 * 移动tail_指针，表示有新数据写入
			 * len不能超过WritableBytes()
			 */
			void HasWritten(size_t len)
			{
				assert(len <= WritableBytes());
				tail_ += len;
			}

			/**
			 * @brief 撤销已写入的数据
			 * @param offset 要撤销的字节数
			 * 
			 * 向后移动tail_指针，丢弃末尾的数据
			 * offset不能超过ReadableBytes()
			 */
			void Unwrite(size_t offset)
			{
				assert(ReadableBytes() >= offset);
				tail_ -= offset;
			}

			/**
			 * @brief 访问缓冲区中的字节（const版本）
			 * @param offset 偏移量（相对于Peek()）
			 * @return 字节的const引用
			 */
			const char &operator[](size_t offset) const
			{
				assert(ReadableBytes() >= offset);
				return Peek()[offset];
			}
			
			/**
			 * @brief 访问缓冲区中的字节（非const版本）
			 * @param offset 偏移量（相对于Peek()）
			 * @return 字节的引用
			 */
			char &operator[](size_t offset)
			{
				assert(ReadableBytes() >= offset);
				return begin()[head_ + offset];
			}

		private:
			size_t head_;                    // 可读数据的起始位置
			size_t initCap_;                 // 初始容量（用于自动收缩）
			std::vector<char> buffer_;       // 实际存储数据的容器
			size_t tail_;                    // 可写空间的起始位置（可读数据的结束位置）
			
			/**
			 * @brief 获取缓冲区起始地址（const版本）
			 * @return 缓冲区起始地址
			 */
			const char *begin() const
			{
				return &buffer_[0];
			}
			
			/**
			 * @brief 获取缓冲区起始地址（非const版本）
			 * @return 缓冲区起始地址
			 */
			char *begin()
			{
				return &buffer_[0];
			}
		};

		/**
		 * @brief 交换两个MsgBuffer对象
		 * @param one 第一个缓冲区
		 * @param two 第二个缓冲区
		 * 
		 * 高效交换两个缓冲区的内容，不涉及数据拷贝
		 */
		inline void swap(MsgBuffer &one, MsgBuffer &two) noexcept
		{
			one.Swap(two);
		}
		
		/**
		 * @brief 将64位整数转换为网络字节序（大端）
		 * @param n 主机字节序的64位整数
		 * @return 网络字节序的64位整数
		 * 
		 * 在大端机器上直接返回，在小端机器上反转字节序
		 */
		inline uint64_t hton64(uint64_t n)
		{
			static const int one = 1;
			static const char sig = *(char *)&one;
			if (sig == 0)
				return n;  // 大端机器，直接返回
			char *ptr = reinterpret_cast<char *>(&n);
			std::reverse(ptr, ptr + sizeof(uint64_t));
			return n;
		}
		
		/**
		 * @brief 将64位整数从网络字节序转换为主机字节序
		 * @param n 网络字节序的64位整数
		 * @return 主机字节序的64位整数
		 * 
		 * 与hton64功能相同（网络字节序和主机字节序的转换是对称的）
		 */
		inline uint64_t ntoh64(uint64_t n)
		{
			return hton64(n);
		}
	}
}

//#define _C_LIBHTTP_MSG_BUFFER_H_
#endif // 