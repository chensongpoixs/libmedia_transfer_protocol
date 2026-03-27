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

 * 网络连接抽象层 - 统一TCP/UDP连接管理
 * 
 * "在网络的世界里，连接是信息流动的桥梁。无论是可靠的TCP还是快速的UDP，
 *  它们都承载着数据的使命。我们需要一个统一的抽象，让上层应用无需关心
 *  底层传输协议的差异，专注于业务逻辑的实现。"
 *                                                    —— 陈松
 * 
 * 本文件实现了网络连接的统一抽象层，提供以下核心功能：
 * 
 * 1. 协议统一：
 *    - 统一的TCP/UDP连接接口
 *    - 透明的协议切换
 *    - 一致的数据收发API
 * 
 * 2. 上下文管理：
 *    - 多类型上下文存储（RTMP、HTTP、FLV、RTC等）
 *    - 类型安全的上下文访问
 *    - 灵活的上下文生命周期管理
 * 
 * 3. 异步通信：
 *    - 基于信号槽的事件通知
 *    - 异步数据发送
 *    - 非阻塞IO操作
 * 
 * 4. 资源管理：
 *    - 自动的连接生命周期管理
 *    - 优雅的连接关闭
 *    - 缓冲区自动管理

 ******************************************************************************/


#ifndef _C_LIBHTTP_CONNECTION_H_
#define _C_LIBHTTP_CONNECTION_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"

#include "libp2p_peerconnection/connection_context.h"
#include <atomic>
#include "libmedia_transfer_protocol/libnetwork/tcp_server.h"
#include "libmedia_transfer_protocol/libnetwork/udp_server.h"
#include <unordered_map>



namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		/**
		 * @brief 上下文类型枚举
		 * 
		 * 定义了连接可以关联的各种协议上下文类型。
		 * 每个连接可以同时持有多个不同类型的上下文对象。
		 */
		enum
		{
			kNormalContext = 0,        ///< 普通上下文（默认）
			kRtmpContext,              ///< RTMP协议上下文
			kHttpContext,              ///< HTTP协议上下文
			kShareResourceContext,     ///< 共享资源上下文
			kFlvContext,               ///< FLV格式上下文
			kRtcContext,               ///< WebRTC协议上下文
			kGb28181Context,           ///< GB28181协议上下文
		};


		/**
		 * @brief 传输协议类型
		 * 
		 * 标识连接使用的底层传输协议。
		 */
		enum class  ProtocolType {
			ProtocolUdp = 1,           ///< UDP协议（无连接、不可靠）
			ProtocolTcp                ///< TCP协议（面向连接、可靠）
		};
		/**
		 * @class Connection
		 * @brief 网络连接统一抽象类
		 * 
		 * 功能说明：
		 * Connection类提供了TCP和UDP连接的统一抽象接口，屏蔽了底层传输协议的差异。
		 * 它支持异步数据收发、多类型上下文管理、以及基于信号槽的事件通知机制。
		 * 
		 * 工作原理：
		 * 1. 协议适配：根据构造函数参数自动识别TCP或UDP协议
		 * 2. 事件驱动：通过信号槽机制通知连接状态变化和数据到达
		 * 3. 上下文管理：使用unordered_map存储多种类型的协议上下文
		 * 4. 异步发送：通过网络线程的PostTask实现非阻塞发送
		 * 
		 * 连接生命周期：
		 * 
		 *   创建连接
		 *      ↓
		 *   初始化Socket信号
		 *      ↓
		 *   ┌─────────────┐
		 *   │  连接活跃   │ ←──┐
		 *   └─────────────┘    │
		 *      ↓               │
		 *   数据收发           │
		 *      ↓               │
		 *   触发信号 ──────────┘
		 *      ↓
		 *   关闭连接
		 *      ↓
		 *   清理资源
		 * 
		 * 使用场景：
		 * - WebRTC媒体传输（UDP）
		 * - HTTP/RTMP流媒体传输（TCP）
		 * - GB28181视频监控（TCP/UDP）
		 * - FLV直播推流（TCP）
		 * 
		 * 使用示例：
		 * @code
		 * // TCP连接示例
		 * rtc::Socket* tcp_socket = ...;
		 * Connection* conn = new Connection(network_thread, tcp_socket);
		 * 
		 * // 设置HTTP上下文
		 * auto http_ctx = std::make_shared<HttpContext>();
		 * conn->SetContext(kHttpContext, http_ctx);
		 * 
		 * // 监听数据接收
		 * conn->SignalOnRecv.connect([](Connection* c, const rtc::CopyOnWriteBuffer& data) {
		 *     // 处理接收到的数据
		 *     ProcessData(data);
		 * });
		 * 
		 * // 异步发送数据
		 * rtc::CopyOnWriteBuffer send_data = ...;
		 * conn->AsyncSend(std::move(send_data));
		 * 
		 * // UDP连接示例
		 * rtc::AsyncPacketSocket* udp_socket = ...;
		 * rtc::SocketAddress remote_addr("192.168.1.100", 5000);
		 * Connection* udp_conn = new Connection(network_thread, udp_socket, remote_addr);
		 * 
		 * // 发送RTP数据包
		 * udp_conn->AsyncSend(std::move(rtp_packet));
		 * @endcode
		 * 
		 * 注意事项：
		 * - 所有网络操作必须在network_thread中执行
		 * - AsyncSend会自动切换到网络线程执行
		 * - 连接关闭后不能再发送数据（available_write标志控制）
		 * - UDP连接需要指定远程地址
		 * - 上下文对象使用shared_ptr管理生命周期
		 */
		class Connection : public   sigslot::has_slots<>
		{
		public:
			/**
			 * @brief UDP连接构造函数
			 * @param network_thread 网络线程（所有网络操作必须在此线程执行）
			 * @param session UDP套接字
			 * @param addr 远程地址
			 */
			Connection(rtc::Thread* network_thread, rtc::AsyncPacketSocket* session, const rtc::SocketAddress& addr);
			
			/**
			 * @brief TCP连接构造函数
			 * @param network_thread 网络线程
			 * @param session TCP套接字
			 */
			Connection(rtc::Thread* network_thread, rtc::Socket* session);
			
			/**
			 * @brief 析构函数
			 * 
			 * 断开所有信号连接，释放资源
			 */
			virtual ~Connection();
		public:
			/**
			 * @brief 关闭连接
			 * 
			 * 异步关闭连接，设置available_write标志为false，
			 * 并在网络线程中执行实际的关闭操作。
			 */
			void  Close();

			/**
			 * @brief 同步发送数据（不推荐使用）
			 * @param data 数据指针
			 * @param size 数据长度
			 * 
			 * 注意：此方法直接在当前线程发送，可能导致线程安全问题。
			 * 建议使用AsyncSend代替。
			 */
			void AyncSend(const uint8_t* data, int32_t  size);

			/**
			 * @brief 异步发送数据（推荐）
			 * @param data 数据缓冲区（移动语义）
			 * 
			 * 将数据发送任务投递到网络线程执行，保证线程安全。
			 * 使用移动语义避免数据拷贝。
			 */
			void AsyncSend(rtc::CopyOnWriteBuffer&& data);
			
			/**
			 * @brief 获取底层Socket对象
			 * @return TCP Socket指针（UDP连接返回nullptr）
			 */
			rtc::Socket* GetSocket() const { return socket_; }

		public:
			// ========== 信号定义 ==========
			
			/**
			 * @brief 连接关闭信号
			 * @param Connection* 关闭的连接对象
			 * 
			 * 当连接被关闭时触发（主动关闭或对端关闭）
			 */
			sigslot::signal1<libmedia_transfer_protocol::libnetwork::Connection*> SignalOnClose;
			
			/**
			 * @brief 数据接收信号
			 * @param Connection* 接收数据的连接对象
			 * @param rtc::CopyOnWriteBuffer& 接收到的数据
			 * 
			 * 当接收到数据时触发，数据已完整读取到缓冲区
			 */
			sigslot::signal2<libmedia_transfer_protocol::libnetwork::Connection*, const rtc::CopyOnWriteBuffer&> SignalOnRecv;
			
			/**
			 * @brief 数据发送完成信号
			 * @param Connection* 发送数据的连接对象
			 * 
			 * 当数据发送完成且Socket可写时触发
			 */
			sigslot::signal1<libmedia_transfer_protocol::libnetwork::Connection*> SignalOnSent;
		public:


		public:
		private:

		public:
			// ========== 上下文管理接口 ==========
			
			/**
			 * @brief 设置上下文对象（拷贝版本）
			 * @param type 上下文类型（如kHttpContext、kRtmpContext等）
			 * @param context 上下文对象的shared_ptr
			 * 
			 * 将指定类型的上下文对象关联到此连接。
			 * 如果该类型已存在上下文，则会被覆盖。
			 */
			void SetContext(int type, const std::shared_ptr<void>& context);
			
			/**
			 * @brief 设置上下文对象（移动版本）
			 * @param type 上下文类型
			 * @param context 上下文对象的shared_ptr（移动语义）
			 * 
			 * 使用移动语义避免引用计数增加，性能更优。
			 */
			void SetContext(int type, std::shared_ptr<void>&& context);
			
			/**
			 * @brief 获取上下文对象
			 * @tparam T 上下文对象的实际类型
			 * @param type 上下文类型
			 * @return 类型安全的上下文对象指针，如果不存在则返回空指针
			 * 
			 * 使用示例：
			 * @code
			 * auto http_ctx = conn->GetContext<HttpContext>(kHttpContext);
			 * if (http_ctx) {
			 *     http_ctx->ProcessRequest();
			 * }
			 * @endcode
			 */
			template <typename T> std::shared_ptr<T> GetContext(int type) const
			{
				auto iter = contexts_.find(type);
				if (iter != contexts_.end())
				{
					return std::static_pointer_cast<T>(iter->second);
				}
				return std::shared_ptr<T>();
			}
			
			/**
			 * @brief 清除指定类型的上下文
			 * @param type 上下文类型
			 */
			void ClearContext(int type);
			
			/**
			 * @brief 清除所有上下文
			 * 
			 * 释放所有关联的上下文对象
			 */
			void ClearContext();

		private:
			// ========== Socket事件处理 ==========
			
			/**
			 * @brief 初始化Socket信号连接
			 * 
			 * 将Socket的各种事件信号连接到对应的处理函数
			 */
			void InitSocketSignals();
			
			/**
			 * @brief Socket连接成功回调
			 * @param socket 连接的Socket对象
			 */
			void OnConnect(rtc::Socket* socket);
			
			/**
			 * @brief Socket关闭回调
			 * @param socket 关闭的Socket对象
			 * @param ret 关闭原因代码
			 */
			void OnClose(rtc::Socket* socket, int ret);
			
			/**
			 * @brief Socket可读回调
			 * @param socket 可读的Socket对象
			 * 
			 * 循环读取所有可用数据，直到缓冲区满或无数据可读
			 */
			void OnRead(rtc::Socket* socket);
			
			/**
			 * @brief Socket可写回调
			 * @param socket 可写的Socket对象
			 * 
			 * 设置available_write标志，允许继续发送数据
			 */
			void OnWrite(rtc::Socket* socket);
			
		private:
			// ========== 成员变量 ==========
			
			rtc::Thread* network_thread_;                                    ///< 网络线程（所有网络操作必须在此线程执行）
			rtc::AsyncPacketSocket* udp_session_;                            ///< UDP套接字（UDP连接时使用）
			rtc::Socket* socket_;                                            ///< TCP套接字（TCP连接时使用）
			rtc::SocketAddress  remote_address_;                             ///< 远程地址（UDP连接必需）
			rtc::Buffer  recv_buffer_;                                       ///< 接收缓冲区（8MB）
			int32_t  recv_buffer_size_;                                      ///< 接收缓冲区当前大小
			std::atomic_bool available_write;                                ///< 是否可写标志（原子操作保证线程安全）
			ProtocolType protocol_type_;                                     ///< 传输协议类型（TCP或UDP）
			std::unordered_map<uint32_t, std::shared_ptr<void>> contexts_;   ///< 上下文对象映射表
		};
	}

}


#endif // _C_LIBHTTP_TCP_SESSION_H_