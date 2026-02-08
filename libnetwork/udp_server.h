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

 * UDP服务器 - 支持RTP/RTCP媒体传输
 * 
 * "UDP如同风中的信使，快速而轻盈。它不保证送达，却能承载实时的音视频流。
 *  在WebRTC的世界里，UDP是媒体传输的首选，因为实时性比可靠性更重要。"
 *                                                    —— 陈松
 * 
 * 本文件实现了UDP服务器功能，主要用于：
 * 
 * 1. WebRTC媒体传输：
 *    - RTP数据包发送和接收
 *    - RTCP控制包处理
 *    - STUN/TURN协议支持
 * 
 * 2. GB28181视频监控：
 *    - PS流接收
 *    - RTP over UDP
 * 
 * 3. 实时音视频：
 *    - 低延迟传输
 *    - 支持丢包容忍
 * 
 * UDP vs TCP：
 * - UDP：无连接、不可靠、快速、适合实时媒体
 * - TCP：面向连接、可靠、慢、适合文件传输
 * 
 * 支持两种模式：
 * - 异步模式（AsyncUDPSocket）：推荐，性能更好
 * - 同步模式（Socket）：兼容模式

 ******************************************************************************/


#ifndef _C_LIBNETWORK_UDP_SERVER_H_
#define _C_LIBNETWORK_UDP_SERVER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 
#include "libp2p_peerconnection/connection_context.h"
#include <atomic>
#include "rtc_base/buffer.h"
#include "rtc_base/thread.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"


namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
		/**
		 * @class UdpServer
		 * @brief UDP服务器类 - 用于RTP/RTCP媒体传输
		 * 
		 * 功能说明：
		 * UdpServer提供了完整的UDP服务器功能，专门用于实时媒体传输。
		 * 它支持RTP/RTCP协议，可以同时处理多个客户端的数据包。
		 * 
		 * 工作原理：
		 * 1. 服务器启动：创建UDP Socket并绑定到指定端口
		 * 2. 数据接收：通过SignalReadPacket信号通知上层
		 * 3. 数据发送：支持RTP、RTCP、普通数据包发送
		 * 4. 无连接：每个数据包独立处理，不维护连接状态
		 * 
		 * UDP数据包格式：
		 * 
		 *   +--------+--------+--------+--------+
		 *   |  IP头  | UDP头  |   数据负载      |
		 *   +--------+--------+--------+--------+
		 *   | 20字节 | 8字节  |   N字节         |
		 *   +--------+--------+--------+--------+
		 * 
		 * RTP数据包格式：
		 * 
		 *    0                   1                   2                   3
		 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |                           timestamp                           |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *   |           synchronization source (SSRC) identifier            |
		 *   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
		 *   |            contributing source (CSRC) identifiers             |
		 *   |                             ....                              |
		 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * 
		 * 使用场景：
		 * - WebRTC媒体服务器
		 * - GB28181视频监控服务器
		 * - 实时音视频传输
		 * - STUN/TURN服务器
		 * 
		 * 使用示例：
		 * @code
		 * // 创建UDP服务器
		 * UdpServer* server = new UdpServer();
		 * 
		 * // 监听数据接收
		 * server->SignalReadPacket.connect([](rtc::AsyncPacketSocket* socket,
		 *                                      const uint8_t* data,
		 *                                      size_t len,
		 *                                      const rtc::SocketAddress& addr,
		 *                                      const int64_t& timestamp) {
		 *     // 处理接收到的数据包
		 *     if (IsRtpPacket(data, len)) {
		 *         ProcessRtpPacket(data, len, addr);
		 *     } else if (IsRtcpPacket(data, len)) {
		 *         ProcessRtcpPacket(data, len, addr);
		 *     }
		 * });
		 * 
		 * // 启动服务器
		 * if (server->Startup("0.0.0.0", 5000)) {
		 *     std::cout << "UDP server started on port 5000" << std::endl;
		 * }
		 * 
		 * // 发送RTP数据包
		 * rtc::CopyOnWriteBuffer rtp_packet = ...;
		 * rtc::SocketAddress client_addr("192.168.1.100", 6000);
		 * server->SendRtpPacketTo(std::move(rtp_packet), client_addr, rtc::PacketOptions());
		 * 
		 * // 发送RTCP数据包
		 * rtc::CopyOnWriteBuffer rtcp_packet = ...;
		 * server->SendRtcpPacketTo(std::move(rtcp_packet), client_addr, rtc::PacketOptions());
		 * @endcode
		 * 
		 * 注意事项：
		 * - UDP不保证数据包送达，需要上层处理丢包
		 * - UDP不保证数据包顺序，需要使用序列号排序
		 * - 单个UDP数据包最大1472字节（MTU 1500 - IP头20 - UDP头8）
		 * - 使用异步模式（ASYNC_UDP=1）性能更好
		 * - 所有网络操作在network_thread中执行
		 */
		class UdpServer : public   sigslot::has_slots<>
		{
		public:
			/**
			 * @brief 构造函数
			 * 
			 * 创建ConnectionContext，初始化三个线程
			 */
			explicit UdpServer();

			/**
			 * @brief 析构函数
			 * 
			 * 释放资源
			 */
			virtual ~UdpServer();
			
		public:
			/**
			 * @brief 启动UDP服务器
			 * @param ip 监听IP地址（如"0.0.0.0"表示监听所有网卡）
			 * @param port 监听端口号
			 * @return 成功返回true，失败返回false
			 * 
			 * 启动流程：
			 * 1. 创建UDP Socket
			 * 2. 绑定到指定IP和端口
			 * 3. 初始化Socket信号连接
			 * 
			 * 注意：如果不在network_thread中调用，会自动切换到network_thread执行
			 */
			bool Startup(const std::string &ip, uint16_t port);
		public:
			// ========== 数据发送接口 ==========
			
			/**
			 * @brief 发送数据包到指定地址
			 * @param packet 数据包缓冲区
			 * @param addr 目标地址
			 * @param options 数据包选项
			 * @return 发送的字节数，失败返回负数
			 */
			int32_t SendPacketTo(const rtc::Buffer& packet,
				const rtc::SocketAddress& addr,
				const rtc::PacketOptions& options);
			
			/**
			 * @brief 发送原始数据到指定地址
			 * @param pv 数据指针
			 * @param cb 数据长度
			 * @param addr 目标地址
			 * @param options 数据包选项
			 * @return 发送的字节数，失败返回负数
			 */
			int32_t SendTo(const uint8_t* pv,
				size_t cb,
				const rtc::SocketAddress& addr,
				const rtc::PacketOptions& options);

			// ========== RTP数据包发送 ==========
			
			/**
			 * @brief 发送单个RTP数据包
			 * @param packet RTP数据包（移动语义）
			 * @param addr 目标地址
			 * @param options 数据包选项
			 * @return 发送的字节数，失败返回负数
			 * 
			 * 用于发送音视频RTP数据包
			 */
			int32_t SendRtpPacketTo(rtc::CopyOnWriteBuffer packet, 
				const rtc::SocketAddress& addr, 
				const rtc::PacketOptions& options);
			
			/**
			 * @brief 批量发送RTP数据包
			 * @param packets RTP数据包列表
			 * @param addr 目标地址
			 * @param options 数据包选项
			 * @return 发送的字节数，失败返回负数
			 * 
			 * 用于批量发送多个RTP数据包，提高效率
			 */
			int32_t SendRtpPacketTo(std::vector<std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>> packets,
				const rtc::SocketAddress& addr, 
				const rtc::PacketOptions& options);
			
			// ========== RTCP数据包发送 ==========
			
			/**
			 * @brief 发送RTCP控制包
			 * @param packet RTCP数据包（移动语义）
			 * @param addr 目标地址
			 * @param options 数据包选项
			 * @return 发送的字节数，失败返回负数
			 * 
			 * 用于发送RTCP反馈包（SR、RR、NACK、PLI、FIR等）
			 */
			int32_t SendRtcpPacketTo(rtc::CopyOnWriteBuffer packet, 
				const rtc::SocketAddress& addr, 
				const rtc::PacketOptions& options);
		public:
			// ========== 信号定义 ==========
			
			/**
			 * @brief 数据包接收信号（异步模式）
			 * @param rtc::AsyncPacketSocket* 接收数据的Socket
			 * @param const uint8_t* 数据指针
			 * @param size_t 数据长度
			 * @param rtc::SocketAddress& 发送方地址
			 * @param const int64_t& 接收时间戳（微秒）
			 * 
			 * 当接收到UDP数据包时触发（异步模式）
			 * 用于处理RTP、RTCP、STUN等协议数据包
			 */
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				const int64_t&>
				SignalReadPacket;

			/**
			 * @brief 数据包接收信号（同步模式）
			 * @param rtc::Socket* 接收数据的Socket
			 * @param const uint8_t* 数据指针
			 * @param size_t 数据长度
			 * @param rtc::SocketAddress& 发送方地址
			 * @param const int64_t 接收时间戳（微秒）
			 * 
			 * 当接收到UDP数据包时触发（同步模式）
			 */
			sigslot::signal5<rtc::Socket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				const int64_t>
				SignalSyncReadPacket;

		public:
			// ========== 线程访问接口 ==========
			
			/**
			 * @brief 获取信令线程
			 */
			rtc::Thread* signaling_thread() { return context_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return context_->signaling_thread(); }
			
			/**
			 * @brief 获取工作线程
			 */
			rtc::Thread* worker_thread() { return context_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return context_->worker_thread(); }
			
			/**
			 * @brief 获取网络线程
			 * 
			 * 所有网络操作必须在此线程执行
			 */
			rtc::Thread* network_thread() { return context_->network_thread(); }
			const rtc::Thread* network_thread() const { return context_->network_thread(); }
			
		public:
			// ========== Socket事件处理（异步模式） ==========
			
			/**
			 * @brief 初始化Socket信号连接
			 */
			void InitSocketSignals();
			
			/**
			 * @brief 新连接建立回调（UDP不使用）
			 * @param socket1 第一个Socket
			 * @param socket2 第二个Socket
			 */
			void OnNewConnection(rtc::AsyncPacketSocket* socket1, rtc::AsyncPacketSocket* socket2);
			
			/**
			 * @brief Socket连接成功回调（UDP不使用）
			 * @param socket Socket对象
			 */
			void OnConnect(rtc::AsyncPacketSocket* socket);
			
			/**
			 * @brief 数据包接收回调
			 * @param socket 接收数据的Socket
			 * @param data 数据指针
			 * @param len 数据长度
			 * @param addr 发送方地址
			 * @param ms 接收时间戳（微秒）
			 * 
			 * 转发数据包到SignalReadPacket信号
			 */
			void OnRecvPacket(rtc::AsyncPacketSocket* socket, 
				const char* data, 
				size_t len,
				const rtc::SocketAddress& addr, 
				const int64_t& ms);
			
			/**
			 * @brief 地址就绪回调
			 * @param socket Socket对象
			 * @param addr 本地地址
			 */
			void OnAddressReady(rtc::AsyncPacketSocket* socket, const rtc::SocketAddress& addr);
			
			/**
			 * @brief Socket可写回调
			 * @param socket Socket对象
			 */
			void OnSend(rtc::AsyncPacketSocket* socket);
			
			/**
			 * @brief Socket关闭回调
			 * @param socket Socket对象
			 * @param error 错误代码
			 */
			void OnClose(rtc::AsyncPacketSocket* socket, int32_t error);

			// ========== Socket事件处理（同步模式） ==========
			
			/**
			 * @brief Socket可读回调（同步模式）
			 * @param socket Socket对象
			 * 
			 * 处理流程：
			 * 1. 循环读取所有可用数据包
			 * 2. 触发SignalSyncReadPacket信号
			 */
			void OnRead(rtc::Socket* socket);
		private:
			// ========== 成员变量 ==========
			
			rtc::scoped_refptr<libp2p_peerconnection::ConnectionContext> context_;  ///< 连接上下文（管理三个线程）
			rtc::SocketAddress server_address_;                                     ///< 服务器监听地址
			std::unique_ptr<rtc::AsyncPacketSocket> udp_control_socket_;            ///< UDP Socket（异步模式）
			std::unique_ptr<rtc::Socket> control_socket_;                           ///< UDP Socket（同步模式）
		};
	}

}


#endif // _C_LIBHTTP_TCP_SESSION_H_