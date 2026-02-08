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

 * 网络连接实现 - Connection类的具体实现
 * 
 * 本文件实现了Connection类的所有方法，包括：
 * - TCP/UDP连接的初始化
 * - 异步数据收发
 * - Socket事件处理
 * - 上下文管理
 * - 连接生命周期管理

 ******************************************************************************/


#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"

#include "libp2p_peerconnection/connection_context.h"
#include <atomic>
#include "libmedia_transfer_protocol/libnetwork/connection.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"


namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
			/**
		 * UDP连接构造函数实现
		 * 
		 * 初始化流程：
		 * 1. 保存网络线程和UDP套接字
		 * 2. 记录远程地址
		 * 3. 分配8MB接收缓冲区
		 * 4. 设置协议类型为UDP
		 * 5. 初始化Socket信号连接
		 */
		Connection::Connection(rtc::Thread* network_thread, rtc::AsyncPacketSocket* session, const rtc::SocketAddress& addr)
			: network_thread_(network_thread)
			, udp_session_(session)
			, socket_(nullptr)
			, remote_address_(addr)
			, recv_buffer_(1024 * 1024 * 8)
			, recv_buffer_size_(0)
			, available_write(true)
			, protocol_type_(ProtocolType::ProtocolUdp)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "remote:" << remote_address_.ToString();
			InitSocketSignals();
		}
		
		/**
		 * TCP连接构造函数实现
		 * 
		 * 初始化流程：
		 * 1. 保存网络线程和TCP套接字
		 * 2. 从Socket获取远程地址
		 * 3. 分配8MB接收缓冲区
		 * 4. 设置协议类型为TCP
		 * 5. 初始化Socket信号连接
		 */
		Connection::Connection(rtc::Thread* network_thread, rtc::Socket* session)
			: network_thread_(network_thread)
			, udp_session_(nullptr)
			, socket_(session)
			, remote_address_(session->GetRemoteAddress())
			, recv_buffer_(1024 * 1024 * 8)
			, recv_buffer_size_(0)
			, available_write(true)
			, protocol_type_(ProtocolType::ProtocolTcp)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "remote:" << remote_address_.ToString();
			InitSocketSignals();
		}

		/**
		 * 析构函数实现
		 * 
		 * 清理流程：
		 * 1. 断开所有Socket信号连接
		 * 2. 释放Socket资源（由外部管理）
		 */
		Connection::~Connection()
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "remote:" << remote_address_.ToString();
			if (socket_)
			{
				socket_->SignalCloseEvent.disconnect(this);
				socket_->SignalConnectEvent.disconnect(this);
				socket_->SignalReadEvent.disconnect(this);
				socket_->SignalWriteEvent.disconnect(this);
			}
		}
			/**
		 * 关闭连接实现
		 * 
		 * 处理流程：
		 * 1. 投递关闭任务到网络线程
		 * 2. 设置available_write为false，禁止继续发送
		 * 3. 调用Socket的Close方法
		 * 
		 * 注意：异步执行，立即返回
		 */
		void Connection::Close()
		{

			network_thread_->PostTask(RTC_FROM_HERE, [this]() {
				available_write = false;
				if (socket_)
				{
					socket_->Close();
				}
				});
		}
		
		/**
		 * 同步发送数据实现
		 * 
		 * 处理流程：
		 * 1. 根据协议类型选择发送方式
		 * 2. UDP：使用SendTo发送到远程地址
		 * 3. TCP：使用Send直接发送
		 * 
		 * 注意：此方法在当前线程执行，可能不是网络线程
		 */
		void Connection::AyncSend(const uint8_t* data, int32_t size)
		{

			if (protocol_type_ == ProtocolType::ProtocolUdp)
			{
				udp_session_->SendTo(data, size, remote_address_, rtc::PacketOptions());
			}
			else //if ()
			{
				socket_->Send(data, size);
			}


		}
		
		/**
		 * 异步发送数据实现
		 * 
		 * 处理流程：
		 * 1. 检查available_write标志
		 * 2. 投递发送任务到网络线程
		 * 3. 在网络线程中再次检查available_write
		 * 4. 根据协议类型执行发送
		 * 
		 * 优点：
		 * - 线程安全（在网络线程执行）
		 * - 使用移动语义避免数据拷贝
		 * - 双重检查避免关闭后发送
		 */
		void Connection::AsyncSend(rtc::CopyOnWriteBuffer&& data)
		{
			if (!available_write)
			{
				return;
			}
			network_thread_->PostTask(RTC_FROM_HERE, [this, new_data = std::move(data)]() {
				if (!available_write)
				{
					return;
				}
				if (protocol_type_ == ProtocolType::ProtocolUdp)
				{
					udp_session_->SendTo(new_data.data(), new_data.size(), remote_address_, rtc::PacketOptions());
				}
				else  if (socket_ && new_data.data())
				{
					socket_->Send(new_data.data(), new_data.size());
				}
				else
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << "ASYNC send data failed !!!";
				}
			});
		}
			/**
		 * 初始化Socket信号连接
		 * 
		 * 将Socket的四个事件信号连接到对应的处理函数：
		 * - SignalCloseEvent -> OnClose
		 * - SignalConnectEvent -> OnConnect
		 * - SignalReadEvent -> OnRead
		 * - SignalWriteEvent -> OnWrite
		 */
		void Connection::InitSocketSignals()
		{
			if (socket_)
			{
				socket_->SignalCloseEvent.connect(this, &Connection::OnClose);
				socket_->SignalConnectEvent.connect(this, &Connection::OnConnect);
				socket_->SignalReadEvent.connect(this, &Connection::OnRead);
				socket_->SignalWriteEvent.connect(this, &Connection::OnWrite);
			}

		}
		
		/**
		 * Socket连接成功回调
		 * 
		 * 当TCP连接建立成功时触发（UDP不会触发）
		 */
		void Connection::OnConnect(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		
		/**
		 * Socket关闭回调
		 * 
		 * 处理流程：
		 * 1. 设置available_write为false
		 * 2. 触发SignalOnClose信号通知上层
		 * 
		 * 触发场景：
		 * - 对端关闭连接
		 * - 本地调用Close()
		 * - 网络错误导致连接断开
		 */
		void Connection::OnClose(rtc::Socket* socket, int ret)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			available_write = false;
			SignalOnClose(this);
		}
		
		/**
		 * Socket可读回调
		 * 
		 * 处理流程：
		 * 1. 创建临时缓冲区（8MB）
		 * 2. 循环读取数据直到：
		 *    - 无数据可读（bytes <= 0）
		 *    - 缓冲区已满
		 * 3. 触发SignalOnRecv信号传递数据
		 * 
		 * 优化：
		 * - 一次性读取所有可用数据
		 * - 避免多次系统调用
		 * - 使用CopyOnWriteBuffer减少拷贝
		 */
		void Connection::OnRead(rtc::Socket* socket)
		{
			//LIBTCP_LOG_T_F(LS_INFO) << "";

			rtc::Buffer buffer(1024 * 1024 * 8);
			buffer.SetSize(0);


			do {
				int bytes = socket->Recv(buffer.begin() + buffer.size(), buffer.capacity() - buffer.size(), nullptr);
				if (bytes <= 0)
					break;
				//read_bytes += buffer;
				buffer.SetSize(buffer.size() + bytes);
				if (buffer.size() >= (buffer.capacity()))
				{
					break;
				}
			} while (true);

			SignalOnRecv(this, rtc::CopyOnWriteBuffer(buffer));

		}
		
		/**
		 * Socket可写回调
		 * 
		 * 当Socket从不可写变为可写时触发。
		 * 设置available_write为true，允许继续发送数据。
		 */
		void Connection::OnWrite(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			available_write = true;
		}
			/**
		 * 设置上下文对象（拷贝版本）
		 * 
		 * 将指定类型的上下文对象关联到此连接。
		 * 使用shared_ptr管理生命周期。
		 */
		void Connection::SetContext(int type, const std::shared_ptr<void>& context)
		{
			contexts_[type] = context;
		}
		
		/**
		 * 设置上下文对象（移动版本）
		 * 
		 * 使用移动语义避免引用计数增加，性能更优。
		 */
		void Connection::SetContext(int type, std::shared_ptr<void>&& context)
		{
			contexts_[type] = std::move(context);
		}
		
		/**
		 * 清除指定类型的上下文
		 * 
		 * 释放指定类型的上下文对象。
		 * 如果没有其他引用，对象将被销毁。
		 */
		void Connection::ClearContext(int type)
		{
			contexts_[type].reset();
		}
		
		/**
		 * 清除所有上下文
		 * 
		 * 释放所有关联的上下文对象。
		 */
		void Connection::ClearContext()
		{
			contexts_.clear();
		}
	}

}
