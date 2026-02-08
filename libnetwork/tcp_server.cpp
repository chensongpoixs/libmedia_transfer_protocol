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

 * TCP服务器实现文件
 * 
 * 实现了TcpServer类的所有方法，包括：
 * - 服务器启动和关闭
 * - 客户端连接管理
 * - Socket事件处理
 * - 上下文管理

 ******************************************************************************/
#include "libmedia_transfer_protocol/libnetwork/tcp_server.h"
#include "libmedia_transfer_protocol/libnetwork/connection.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"




namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{



			/**
		 * 构造函数实现
		 * 
		 * 创建ConnectionContext，初始化三个线程：
		 * - signaling_thread：信令线程
		 * - worker_thread：工作线程
		 * - network_thread：网络线程
		 */
		TcpServer::TcpServer()
			: context_(libp2p_peerconnection::ConnectionContext::Create())
			, tcp_sessions_()
			//, audio_play_(nullptr)
		{

			//context_->worker_thread()->PostTask([this]() {
			//	audio_play_ = std::make_unique<libcross_platform_collection_render::AudioCapture>(context_->worker_thread());
			//});
		}

		/**
		 * 析构函数实现
		 * 
		 * 清理流程：
		 * 1. 断开监听Socket的所有信号连接
		 * 2. 释放监听Socket
		 * 3. 释放ConnectionContext
		 */
		TcpServer::~TcpServer()
		{
			if (control_socket_)
			{
				control_socket_->SignalCloseEvent.disconnect(this);
				control_socket_->SignalConnectEvent.disconnect(this);
				control_socket_->SignalReadEvent.disconnect(this);
				control_socket_->SignalWriteEvent.disconnect(this);
				control_socket_.reset();
			}
			if (context_)
			{
				//context_
			}
		}

			/**
		 * 启动TCP服务器实现
		 * 
		 * 启动流程：
		 * 1. 设置服务器地址（IP + 端口）
		 * 2. 创建TCP Socket
		 * 3. 绑定到指定地址
		 * 4. 开始监听（队列长度500）
		 * 5. 初始化Socket信号连接
		 * 
		 * @param ip 监听IP地址
		 * @param port 监听端口号
		 * @return 成功返回true，失败返回false
		 */
		bool TcpServer::Startup(const std::string &ip, uint16_t port)
		{
			server_address_.SetIP(ip);
			server_address_.SetPort(port);

			control_socket_.reset(context_->network_thread()->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_STREAM));
			if (!context_)
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << "create socket failed !!! " << server_address_.ToString();
				return false;
			}
			InitSocketSignals();
			int32_t ret = control_socket_->Bind(server_address_);
			if (ret != 0)
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << "bind socket failed !!! " << server_address_.ToString();
				return false;
			}

			ret = control_socket_->Listen(500);
			if (ret != 0)
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << "Listen socket failed !!! " << server_address_.ToString();
				return false;
			}
			LIBNETWORK_LOG(LS_INFO) << " tcp start port:" << port << " OK !!!";
			return true;
		}


		/**
		 * 关闭指定连接实现
		 * 
		 * 调用Connection的Close方法，异步关闭连接
		 */
		void TcpServer::CloseSession(Connection *conn)
		{
			conn->Close();
		}
		
		/**
		 * 关闭指定Socket实现
		 * 
		 * 直接关闭Socket（不推荐使用）
		 */
		void TcpServer::Close(rtc::Socket *socket)
		{
			socket->Close();
		}
			/**
		 * 设置服务器级别的上下文对象（拷贝版本）
		 */
		void TcpServer::SetContext(int type, const std::shared_ptr<void> &context)
		{
			contexts_[type] = context;
		}
		
		/**
		 * 设置服务器级别的上下文对象（移动版本）
		 */
		void TcpServer::SetContext(int type, std::shared_ptr<void> &&context)
		{
			contexts_[type] = std::move(context);
		}
		
		/**
		 * 清除指定类型的上下文
		 */
		void TcpServer::ClearContext(int type)
		{
			contexts_[type].reset();
		}
		
		/**
		 * 清除所有上下文
		 */
		void TcpServer::ClearContext()
		{
			contexts_.clear();
		}
			/**
		 * Connection数据接收回调实现
		 * 
		 * 转发Connection的数据接收事件到SignalOnRecv信号
		 */
		void TcpServer::OnSessionRecv(Connection * conn, const rtc::CopyOnWriteBuffer & data)
		{
			SignalOnRecv(conn, data);
		}
		
		/**
		 * Connection关闭回调实现
		 * 
		 * 处理流程：
		 * 1. 投递清理任务到网络线程
		 * 2. 从tcp_sessions_中查找Connection
		 * 3. 触发SignalOnDestory信号
		 * 4. 断开Connection的所有信号连接
		 * 5. 释放Connection对象
		 * 6. 从tcp_sessions_中移除
		 */
		void TcpServer::OnSessionClose(Connection*  conn)
		{
			network_thread()->PostTask(RTC_FROM_HERE, [this, conn]() {
				//LIBTCP_LOG(LS_INFO) << "";
				LIBNETWORK_LOG_T_F(LS_INFO) << "";
				auto iter = tcp_sessions_.find(conn->GetSocket());
				if (iter == tcp_sessions_.end())
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << " tcp session not find socket :" << conn->GetSocket()->GetRemoteAddress().ToString();
					SignalOnDestory(conn);
					conn->SignalOnClose.disconnect_all();
					conn->SignalOnRecv.disconnect_all();
					return;
				}
				SignalOnDestory(iter->second.get());
				conn->SignalOnClose.disconnect_all();
				conn->SignalOnRecv.disconnect_all();
				 
				iter->second.reset();
				tcp_sessions_.erase(iter);
			});
			
		}
			/**
		 * 初始化监听Socket的信号连接
		 * 
		 * 将监听Socket的四个事件信号连接到对应的处理函数
		 */
		void TcpServer::InitSocketSignals()
		{
			control_socket_->SignalCloseEvent.connect(this, &TcpServer::OnClose);
			control_socket_->SignalConnectEvent.connect(this, &TcpServer::OnConnect);
			control_socket_->SignalReadEvent.connect(this, &TcpServer::OnRead);
			control_socket_->SignalWriteEvent.connect(this, &TcpServer::OnWrite);
		}
		
		/**
		 * Socket连接成功回调（监听Socket不会触发）
		 */
		void TcpServer::OnConnect(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		
		/**
		 * Socket关闭回调实现
		 * 
		 * 处理客户端Socket关闭事件：
		 * 1. 从tcp_sessions_中查找Connection
		 * 2. 触发SignalOnDestory信号
		 * 3. 释放Connection对象
		 * 4. 从tcp_sessions_中移除
		 */
		void TcpServer::OnClose(rtc::Socket* socket, int ret)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			auto iter = tcp_sessions_.find(socket);
			if (iter == tcp_sessions_.end())
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << " tcp session not find socket :" << socket->GetRemoteAddress().ToString();

				return;
			}
			SignalOnDestory(iter->second.get());
			iter->second.reset();
			tcp_sessions_.erase(iter);
		}
		
		/**
		 * Socket可读回调实现（接受新连接）
		 * 
		 * 处理流程：
		 * 1. 调用Accept接受新连接
		 * 2. 创建Connection对象
		 * 3. 连接Connection的信号：
		 *    - SignalOnRecv -> OnSessionRecv
		 *    - SignalOnClose -> OnSessionClose
		 * 4. 传递共享资源上下文（kShareResourceContext）
		 * 5. 将Connection添加到tcp_sessions_
		 * 6. 触发SignalOnNewConnection信号
		 */
		void TcpServer::OnRead(rtc::Socket* socket)
		{
			//LIBNETWORK_LOG_T_F(LS_INFO) << "";


			rtc::SocketAddress address;
			rtc::Socket*  client = socket->Accept(&address);
			if (!client)
			{
				LIBNETWORK_LOG_T_F(LS_ERROR) << "accept failed !!!";
				return;
			}
			LIBNETWORK_LOG_T_F(LS_INFO) << "tcp new client accept :  " << address.ToString();
			std::unique_ptr<libnetwork::Connection>  tcp_session = std::make_unique<libnetwork::Connection>(network_thread(),  client );
			//http_session->RegisterDecodeCompleteCallback(callback_);
			tcp_session->SignalOnRecv.connect(this, &TcpServer::OnSessionRecv);
			tcp_session->SignalOnClose.connect(this, &TcpServer::OnSessionClose);
			if (contexts_[kShareResourceContext])
			{
				tcp_session->SetContext(kShareResourceContext, contexts_[kShareResourceContext]);
			}
			tcp_sessions_.emplace(std::make_pair(client, std::move(tcp_session)));
			auto iter = tcp_sessions_.find(client);
			if (iter == tcp_sessions_.end())
			{
				LIBNETWORK_LOG_T_F(LS_WARNING) << "tcp session not find failed !!! socket: " << client->GetLocalAddress().ToString() << ", remote:" << client->GetRemoteAddress().ToString();
				return;
			}
			SignalOnNewConnection(iter->second.get());
		}
		
		/**
		 * Socket可写回调
		 */
		void TcpServer::OnWrite(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";

		}
	}


}

