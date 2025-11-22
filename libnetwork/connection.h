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
#include <queue>
#include <mutex>
#include "rtc_base/copy_on_write_buffer.h"



namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		enum
		{
			kNormalContext = 0,
			kRtmpContext,
			kHttpContext,
			kShareResourceContext,
			kFlvContext,
			kRtcContext,
			kGb28181Context,
		};


		enum class  ProtocolType{
			ProtocolUdp= 1,
			ProtocolTcp
		};
		//typedef    UdpServer    UdpSession;
		class Connection : public   sigslot::has_slots<>
		{
		public:
			//explicit Connection();
			 Connection(rtc::Thread* network_thread, rtc::AsyncPacketSocket * session, const rtc::SocketAddress& addr);
			  Connection(rtc::Thread* network_thread, rtc::Socket * session);
			virtual ~Connection();
		public:

			void  Close();

			void AyncSend(const uint8_t *data, int32_t  size);
		
			void AsyncSend(rtc::CopyOnWriteBuffer &&data);
			
			// 同步发送方法（TCP/UDP 通用）
			void Send(const rtc::CopyOnWriteBuffer& data);
			void Send(const uint8_t* data, int32_t size);
			
			rtc::Socket*   GetSocket() const { return socket_; }
			const rtc::SocketAddress& GetRemoteAddress() const { return remote_address_; }


			sigslot::signal1<Connection*> SignalOnClose;
			sigslot::signal2<Connection*, const rtc::CopyOnWriteBuffer&> SignalOnRecv;
			sigslot::signal1<Connection*> SignalOnSent;
		public:

			
		public:
		private:
			
		public:
			 
			 

			void SetContext(int type, const std::shared_ptr<void> &context);
			void SetContext(int type, std::shared_ptr<void> &&context);
			template <typename T> std::shared_ptr<T> GetContext(int type) const
			{
				auto iter = contexts_.find(type);
				if (iter != contexts_.end())
				{
					return std::static_pointer_cast<T>(iter->second);
				}
				return std::shared_ptr<T>();
			}
			void ClearContext(int type);
			void ClearContext();

		private:
			void InitSocketSignals();
			void OnConnect(rtc::Socket* socket);
			void OnClose(rtc::Socket* socket, int ret);
			void OnRead(rtc::Socket* socket);
			void OnWrite(rtc::Socket* socket);
			
			// TCP 发送缓冲区管理
			void TrySendPendingData();
			void AddToSendQueue(const rtc::CopyOnWriteBuffer& data);
			
		private:
			rtc::Thread        *   network_thread_;
			//UdpSession *        udp_session_;
			rtc::AsyncPacketSocket  * udp_session_;
			//TcpSession*        tcp_session_;
			rtc::Socket*  socket_;
			rtc::SocketAddress  remote_address_;
			rtc::Buffer  recv_buffer_;
			int32_t  recv_buffer_size_ = 0;
			std::atomic_bool         available_write;
			ProtocolType        protocol_type_ = ProtocolType::ProtocolUdp;

			// TCP 发送队列（当 socket 不可写时缓冲数据）
			std::queue<rtc::CopyOnWriteBuffer> send_queue_;
			std::mutex send_queue_lock_;
			static constexpr size_t kMaxSendQueueSize = 1024 * 1024 * 10;  // 最大发送队列大小 10MB
			size_t send_queue_total_size_ = 0;

			std::unordered_map<uint32_t, std::shared_ptr<void>>     contexts_;
		};
	}

}


#endif // _C_LIBHTTP_TCP_SESSION_H_