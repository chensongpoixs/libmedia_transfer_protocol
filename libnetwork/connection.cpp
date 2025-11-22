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

 
#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 
#include "libp2p_peerconnection/connection_context.h"
#include <atomic>
#include "libmedia_transfer_protocol/libnetwork/connection.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
#include <mutex>


namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		Connection::Connection(rtc::Thread* network_thread, rtc::AsyncPacketSocket * session, const rtc::SocketAddress& addr)
			: network_thread_(network_thread)
			, udp_session_(session) 
			, socket_(nullptr)
			, remote_address_(addr)
			, recv_buffer_(1024 * 1024 * 8)
			, recv_buffer_size_(0)
			, available_write(true)
			, protocol_type_(ProtocolType::ProtocolUdp)
			, send_queue_total_size_(0)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "remote:" << remote_address_.ToString();
			InitSocketSignals();
		}
		Connection::Connection(rtc::Thread* network_thread, rtc::Socket * session)
			: network_thread_(network_thread)
			, udp_session_(nullptr)
			, socket_(session)
			, remote_address_(session->GetRemoteAddress())
			, recv_buffer_(1024 * 1024 * 8)
			, recv_buffer_size_(0)
			, available_write(true)
			, protocol_type_(ProtocolType::ProtocolTcp)
			, send_queue_total_size_(0)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "remote:" << remote_address_.ToString();
			InitSocketSignals();
		}
		 
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
		void Connection::Close()
		{
			network_thread_->PostTask(RTC_FROM_HERE, [this ]() {
				available_write = false;
				
				// 清空发送队列
				{
					std::lock_guard<std::mutex> lock(send_queue_lock_);
					while (!send_queue_.empty())
					{
						send_queue_.pop();
					}
					send_queue_total_size_ = 0;
				}
				
				if (socket_)
				{
					socket_->Close();
				}
			});
		}
		void Connection::AyncSend(const uint8_t * data, int32_t size)
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
		void Connection::AsyncSend(rtc::CopyOnWriteBuffer&&  data)
		{
			if (data.size() == 0)
			{
				return;
			}
			
			network_thread_->PostTask(RTC_FROM_HERE, [this,  new_data = std::move(data)]() {
				if (protocol_type_ == ProtocolType::ProtocolUdp)
				{
					if (udp_session_)
					{
						udp_session_->SendTo(new_data.data(), new_data.size(), remote_address_, rtc::PacketOptions());
						SignalOnSent(this);
					}
					else
					{
						LIBNETWORK_LOG_T_F(LS_WARNING) << "UDP session is null, send failed";
					}
				}
				else if (protocol_type_ == ProtocolType::ProtocolTcp)
				{
					if (!socket_)
					{
						LIBNETWORK_LOG_T_F(LS_WARNING) << "TCP socket is null, send failed";
						return;
					}
					
					// 如果 socket 可写，直接发送
					if (available_write)
					{
						int sent = socket_->Send(new_data.data(), new_data.size());
						
						if (sent > 0)
						{
							if (sent == static_cast<int>(new_data.size()))
							{
								// 完整发送
								SignalOnSent(this);
							}
							else
							{
								// 部分发送，将剩余数据加入队列
								LIBNETWORK_LOG(LS_VERBOSE) << "partial send, sent:" << sent << ", total:" << new_data.size();
								rtc::CopyOnWriteBuffer remaining(new_data.data() + sent, new_data.size() - sent);
								AddToSendQueue(remaining);
								available_write = false;
							}
						}
						else if (sent == 0)
						{
							// socket 缓冲区满，加入队列
							AddToSendQueue(new_data);
							available_write = false;
						}
						else
						{
							// 发送错误
							LIBNETWORK_LOG(LS_ERROR) << "TCP send error, closing connection";
							Close();
						}
					}
					else
					{
						// socket 不可写，加入队列
						AddToSendQueue(new_data);
					}
				}
				else
				{
					LIBNETWORK_LOG_T_F(LS_WARNING) << "Unknown protocol type, send failed";
				}
			});
		}
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
		void Connection::OnConnect(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
		}
		void Connection::OnClose(rtc::Socket* socket, int ret)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			available_write = false;
			SignalOnClose(this);
		}
		void Connection::OnRead(rtc::Socket* socket)
		{
			//LIBTCP_LOG_T_F(LS_INFO) << "";

			// TCP 模式下，使用固定大小的缓冲区读取数据
			// 如果数据量大，可以多次读取
			rtc::Buffer buffer(1024 * 1024 * 8);  // 8MB 缓冲区
			buffer.SetSize(0);

			do {
				size_t available = buffer.capacity() - buffer.size();
				if (available == 0)
				{
					// 缓冲区已满，发送当前数据
					if (buffer.size() > 0)
					{
						SignalOnRecv(this, rtc::CopyOnWriteBuffer(buffer));
						buffer.SetSize(0);
					}
					break;
				}
				
				int bytes = socket->Recv(buffer.begin() + buffer.size(), available, nullptr);
				if (bytes <= 0)
				{
					if (bytes < 0)
					{
						// 读取错误
						LIBNETWORK_LOG(LS_WARNING) << "TCP recv error, closing connection";
						Close();
					}
					break;
				}
				
				buffer.SetSize(buffer.size() + bytes);
				
			} while (true);

			if (buffer.size() > 0)
			{
				SignalOnRecv(this, rtc::CopyOnWriteBuffer(buffer));
			}

		}
		void Connection::OnWrite(rtc::Socket* socket)
		{
			LIBNETWORK_LOG_T_F(LS_INFO) << "";
			available_write = true;
			// TCP 模式下，当 socket 可写时，尝试发送队列中的数据
			if (protocol_type_ == ProtocolType::ProtocolTcp)
			{
				TrySendPendingData();
			}
		}
		
		void Connection::Send(const rtc::CopyOnWriteBuffer& data)
		{
			if (data.size() == 0)
			{
				return;
			}
			
			if (protocol_type_ == ProtocolType::ProtocolUdp)
			{
				// UDP 直接发送
				if (udp_session_)
				{
					udp_session_->SendTo(data.data(), data.size(), remote_address_, rtc::PacketOptions());
				}
			}
			else
			{
				// TCP 发送
				AsyncSend(rtc::CopyOnWriteBuffer(data));
			}
		}
		
		void Connection::Send(const uint8_t* data, int32_t size)
		{
			if (!data || size <= 0)
			{
				return;
			}
			Send(rtc::CopyOnWriteBuffer(data, size));
		}
		
		void Connection::TrySendPendingData()
		{
			if (protocol_type_ != ProtocolType::ProtocolTcp || !socket_ || !available_write)
			{
				return;
			}
			
			std::lock_guard<std::mutex> lock(send_queue_lock_);
			
			while (!send_queue_.empty() && available_write)
			{
				auto& data = send_queue_.front();
				int sent = socket_->Send(data.data(), data.size());
				
				if (sent > 0)
				{
					if (sent == static_cast<int>(data.size()))
					{
						// 完整发送
						send_queue_total_size_ -= data.size();
						send_queue_.pop();
						SignalOnSent(this);
					}
					else
					{
						// 部分发送，保留剩余数据
						LIBNETWORK_LOG(LS_VERBOSE) << "partial send, sent:" << sent << ", total:" << data.size();
						rtc::CopyOnWriteBuffer remaining(data.data() + sent, data.size() - sent);
						send_queue_total_size_ -= sent;
						send_queue_.pop();
						send_queue_.push(remaining);
						send_queue_total_size_ += remaining.size();
						available_write = false;  // socket 缓冲区可能已满
						break;
					}
				}
				else if (sent == 0)
				{
					// socket 缓冲区满，等待下次可写事件
					available_write = false;
					break;
				}
				else
				{
					// 发送错误
					LIBNETWORK_LOG(LS_ERROR) << "send error, closing connection";
					Close();
					break;
				}
			}
		}
		
		void Connection::AddToSendQueue(const rtc::CopyOnWriteBuffer& data)
		{
			std::lock_guard<std::mutex> lock(send_queue_lock_);
			
			// 检查队列大小限制
			if (send_queue_total_size_ + data.size() > kMaxSendQueueSize)
			{
				LIBNETWORK_LOG(LS_WARNING) << "send queue overflow, dropping data, size:" << data.size();
				return;
			}
			
			send_queue_.push(data);
			send_queue_total_size_ += data.size();
		}
		void Connection::SetContext(int type, const std::shared_ptr<void> &context)
		{
			contexts_[type] = context;
		}
		void Connection::SetContext(int type, std::shared_ptr<void> &&context)
		{
			contexts_[type] = std::move(context);
		}
		void Connection::ClearContext(int type)
		{
			contexts_[type].reset();
		}
		void Connection::ClearContext()
		{
			contexts_.clear();
		}
	}

}
 