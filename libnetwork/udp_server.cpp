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
				   date:  2025-10-24

 * UDP服务器实现文件
 * 
 * 实现了UdpServer类的所有方法，包括：
 * - 服务器启动
 * - RTP/RTCP数据包发送
 * - Socket事件处理
 * - 异步和同步两种模式
 * 
 * 编译选项：
 * - ASYNC_UDP=1：使用异步模式（推荐）
 * - ASYNC_UDP=0：使用同步模式

 ******************************************************************************/

#include "libmedia_transfer_protocol/libnetwork/udp_server.h"
#include "rtc_base/async_udp_socket.h"
#include "rtc_base/buffer.h"
#include "rtc_base/byte_buffer.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"




#define  ASYNC_UDP  (1)


namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
			/**
		 * 构造函数实现
		 * 
		 * 创建ConnectionContext，初始化三个线程
		 */
		UdpServer::UdpServer()
			: context_(libp2p_peerconnection::ConnectionContext::Create())
		{
		}
		
		/**
		 * 析构函数实现
		 */
		UdpServer::~UdpServer()
		{
		}
			/**
		 * 启动UDP服务器实现
		 * 
		 * 启动流程：
		 * 1. 设置服务器地址（IP + 端口）
		 * 2. 根据编译选项创建Socket：
		 *    - ASYNC_UDP=1：创建AsyncUDPSocket（异步模式）
		 *    - ASYNC_UDP=0：创建普通Socket（同步模式）
		 * 3. 绑定到指定地址
		 * 4. 初始化Socket信号连接
		 * 
		 * 注意：如果不在network_thread中调用，会自动切换到network_thread执行
		 * 
		 * @param ip 监听IP地址
		 * @param port 监听端口号
		 * @return 成功返回true，失败返回false
		 */
		bool UdpServer::Startup(const std::string & ip, uint16_t port)
		{
			server_address_.SetIP(ip);
			server_address_.SetPort(port);
			if (network_thread()->IsCurrent())
			{ 
#if ASYNC_UDP
				udp_control_socket_.reset(rtc::AsyncUDPSocket::Create(network_thread()->socketserver(), server_address_));

				if (!udp_control_socket_)
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << "create rtc udp server  socket failed !!! " << server_address_.ToString();
					return  false;
				}
#else 
				//std::unique_ptr<rtc::Socket> control_socket_;
				control_socket_.reset(network_thread()->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_DGRAM));
				if (!control_socket_)
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << "create rtc udp server  socket failed !!! " << server_address_.ToString();
					return  false;
				}
				int32_t ret = control_socket_->Bind(server_address_);
				if (ret != 0)
				{
					RTC_LOG(LS_WARNING) << "bind socket failed !!! " << server_address_.ToString();
					return false;
				}

				ret = control_socket_->Listen(500);
				if (ret != 0)
				{
					RTC_LOG(LS_WARNING) << "Listen socket failed !!! " << server_address_.ToString();
				//	return false;
				}

#endif 
				
				InitSocketSignals(); 
				LIBNETWORK_LOG(LS_INFO) << " start rtc udp server port:" << server_address_.port() << ", start OK!!!";
				return true;
			}
			 
			 
			return 	network_thread()->Invoke<bool>(RTC_FROM_HERE, [this]() {
#if ASYNC_UDP
				udp_control_socket_.reset(rtc::AsyncUDPSocket::Create(network_thread()->socketserver(), server_address_));

				if (!udp_control_socket_)
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << "create rtc udp server  socket failed !!! " << server_address_.ToString();
					return  false;
				}
#else 
				//std::unique_ptr<rtc::Socket> control_socket_;
				control_socket_.reset(network_thread()->socketserver()->CreateSocket(server_address_.ipaddr().family(), SOCK_DGRAM));
				if (!control_socket_)
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << "create rtc udp server  socket failed !!! " << server_address_.ToString();
					return  false;
				}

				int32_t ret = control_socket_->Bind(server_address_);
				if (ret != 0)
				{
					RTC_LOG(LS_WARNING) << "bind socket failed !!! " << server_address_.ToString();
					return false;
				}

				ret = control_socket_->Listen(500);
				if (ret != 0)
				{
					RTC_LOG(LS_WARNING) << "Listen socket failed !!! " << server_address_.ToString();
				//	return false;
				}
#endif 
					InitSocketSignals();
					 
					LIBNETWORK_LOG (LS_INFO) << " start rtc udp server port:" << server_address_.port() << ", start OK!!!";
					return true;
				});
			 
		}
			/**
		 * 初始化Socket信号连接
		 * 
		 * 根据编译选项连接不同的信号：
		 * - ASYNC_UDP=1：连接AsyncPacketSocket的信号
		 * - ASYNC_UDP=0：连接普通Socket的信号
		 */
		void UdpServer::InitSocketSignals()
		{ 
#if ASYNC_UDP
			udp_control_socket_->SignalNewConnection.connect(this, &UdpServer::OnNewConnection);
			udp_control_socket_->SignalConnect.connect(this, &UdpServer::OnConnect);
			udp_control_socket_->SignalAddressReady.connect(this, &UdpServer::OnAddressReady);
			
			udp_control_socket_->SignalReadPacket.connect(this, &UdpServer::OnRecvPacket);
			udp_control_socket_->SignalReadyToSend.connect(this, &UdpServer::OnSend);
			udp_control_socket_->SignalClose.connect(this, &UdpServer::OnClose);
#else

			control_socket_->SignalReadEvent.connect(this, &UdpServer::OnRead);
#endif //
 
		}

			/**
		 * 发送数据包到指定地址实现
		 * 
		 * 根据编译选项选择发送方式
		 */
		int32_t UdpServer::SendPacketTo(const rtc::Buffer& packet,
			const rtc::SocketAddress& addr,
			const rtc::PacketOptions& options)
		{
#if ASYNC_UDP
			return udp_control_socket_->SendTo(packet.data(), packet.size(), addr, options);
#else 
			return control_socket_->SendTo(packet.data(), packet.size(), addr );
#endif //
		}

		/**
		 * 发送原始数据到指定地址实现
		 */
		int32_t UdpServer::SendTo(const uint8_t * pv, size_t cb, const rtc::SocketAddress & addr, const rtc::PacketOptions & options)
		{
			//
#if ASYNC_UDP
			return udp_control_socket_->SendTo(pv, cb, addr, options);
#else 
			return control_socket_->SendTo(pv, cb, addr);
#endif //
		}


		/**
		 * 发送单个RTP数据包实现
		 * 
		 * 用于发送音视频RTP数据包
		 */
		int32_t UdpServer::SendRtpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options)
		{
			
#if ASYNC_UDP
			return udp_control_socket_->SendTo(packet.data(), packet.size(), addr, options);
#else 
			return control_socket_->SendTo(packet.data(), packet.size(), addr);
#endif //
		}
		
		/**
		 * 批量发送RTP数据包实现
		 * 
		 * 遍历数据包列表，逐个发送
		 */
		int32_t UdpServer::SendRtpPacketTo(std::vector< std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>>  packets,
			const rtc::SocketAddress& addr, const rtc::PacketOptions& options)
		{
			for (auto &  p : packets)
			{ 
#if ASYNC_UDP
				return udp_control_socket_->SendTo(p->data(), p->size(), addr, options);
#else 
				return control_socket_->SendTo(p->data(), p->size(), addr);
#endif //
			}
			return 0;
			
		}
		
		/**
		 * 发送RTCP控制包实现
		 * 
		 * 用于发送RTCP反馈包（SR、RR、NACK、PLI、FIR等）
		 */
		int32_t UdpServer::SendRtcpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options)
		{
			 

#if ASYNC_UDP
			return udp_control_socket_->SendTo(packet.data(), packet.size(), addr, options);
#else 
			return control_socket_->SendTo(packet.data(), packet.size(), addr);
#endif //
		}
		
			/**
		 * 新连接建立回调（UDP不使用）
		 */
		void UdpServer::OnNewConnection(rtc::AsyncPacketSocket * socket1, rtc::AsyncPacketSocket * socket2)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}

		/**
		 * Socket连接成功回调（UDP不使用）
		 */
		void  UdpServer::OnConnect(rtc::AsyncPacketSocket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		
		/**
		 * 地址就绪回调
		 */
		void UdpServer::OnAddressReady(rtc::AsyncPacketSocket* socket, const rtc::SocketAddress&addr)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "addr:" << socket->GetRemoteAddress().ToString();
		}
		
		/**
		 * 数据包接收回调实现
		 * 
		 * 转发数据包到SignalReadPacket信号
		 */
		void UdpServer::OnRecvPacket(rtc::AsyncPacketSocket * socket, const char  * data, size_t len,
			const rtc::SocketAddress & addr, const int64_t & ms)
		{
			//LIBNETWORK_LOG_T_F(LS_INFO) << "";
			SignalReadPacket(socket, (const uint8_t *)data, len, addr, ms);
		}
		
		/**
		 * Socket可写回调
		 */
		void UdpServer::OnSend(rtc::AsyncPacketSocket * socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "addr:" << socket->GetRemoteAddress().ToString();
		}
		
		/**
		 * Socket关闭回调
		 */
		void  UdpServer::OnClose(rtc::AsyncPacketSocket* socket, int32_t)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		
		/**
		 * Socket可读回调（同步模式）
		 * 
		 * 处理流程：
		 * 1. 创建临时缓冲区（2000字节）
		 * 2. 循环读取所有可用数据包
		 * 3. 触发SignalSyncReadPacket信号
		 */
		void UdpServer::OnRead(rtc::Socket* socket)
		{
			rtc::Buffer buffer(2000);
			buffer.SetSize(0);
			rtc::SocketAddress out_addr;
			int64_t timestamp = 0;
			do {


				int bytes = socket->RecvFrom(buffer.begin() + buffer.size(), buffer.capacity() - buffer.size(), &out_addr, &timestamp);
				if (bytes <= 0)
				{
					break;
				}
				buffer.SetSize(buffer.size() + bytes);
				if (buffer.size() >= (buffer.capacity()))
				{
					break;
				}
			} while (true);

			//RTC_LOG(LS_INFO) << "recvFrom : " << out_addr.ToString() << ",  data => " << std::string((char*)buffer.data(), buffer.size());
			SignalSyncReadPacket(socket, buffer.data(), buffer.size(), out_addr, timestamp);
		}
	}

}

 