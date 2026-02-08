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

 * TCP服务器实现 - 支持多客户端连接管理
 * 
 * "服务器是网络世界的守护者，它静静地监听着端口，等待着每一个连接的到来。
 *  每个连接都是一个故事的开始，每个数据包都承载着信息的使命。"
 *                                                    —— 陈松
 * 
 * GB28181使用RTP传输音视频，有两种方式：UDP、TCP。UDP和RTSP中的没有区别，但是TCP有区别。
 *
 * 目前RTSP有两个版本1.0和2.0，1.0定义在RFC2326中，2.0定义在RFC7826。2.0是2016年由IETF发布的RTSP新标准，
 * 不过现在基本使用的都是RTSP1.0，就算有使用2.0的，也会兼容1.0。
 * 而GB28181则使用RFC4571中定义的RTP，这里面RTP over TCP方式和以往的不同。
 *
 * RFC2326中RTP over TCP的数据包格式：
 * 
 *   +--------+--------+--------+--------+
 *   | magic  |channel | length |  data  |
 *   | number | number |        |        |
 *   +--------+--------+--------+--------+
 *   | 1 byte | 1 byte | 2 bytes| N bytes|
 *   +--------+--------+--------+--------+
 * 
 * - magic number：   RTP数据标识符，"$" 一个字节
 * - channel number： 信道数字 - 1个字节，用来指示信道
 * - data length：    数据长度 - 2个字节，用来指示插入数据长度
 * - data：           数据 - 比如说RTP包，总长度与上面的数据长度相同
 * 
 * RFC4571中的RTP over TCP的数据包格式：
 * 
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +---------------------------------------------------------------+
 *   |             LENGTH            |  RTP or RTCP packet ...       |
 *   +---------------------------------------------------------------+
 * 
 * RFC2326中用channel number标识消息类型，因为RTSP中信令和音视频都是通过同一个TCP通道传输，
 * 所以必须通过channel number区分。而GB28181中信令和媒体数据是不同的传输通道，所以不用去区分。
 *
 * RFC4571标准格式：长度(2字节) + RTP头 + 数据
 * RFC2326标准格式：$(1字节) + 通道号(1字节) + 长度(2字节) + RTP头 + 数据
 * 
 * 本文件实现的TCP服务器特点：
 * 1. 多客户端连接管理
 * 2. 基于信号槽的事件通知
 * 3. 自动的连接生命周期管理
 * 4. 支持多种协议上下文（HTTP、RTMP、GB28181等）
 * 5. 线程安全的操作

 ******************************************************************************/

#ifndef _C_LIBHTTP_TCP_SERVER_H_
#define _C_LIBHTTP_TCP_SERVER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h" 
#include "rtc_base/thread.h"
#include "rtc_base/physical_socket_server.h"
#include "libp2p_peerconnection/connection_context.h"
//#include "libmedia_transfer_protocol/libnetwork/tcp_session.h"
//#include "libmedia_transfer_protocol/libnetwork/connection.h"
namespace  libmedia_transfer_protocol {
	namespace libnetwork
	{
			/**
		 * @class TcpServer
		 * @brief TCP服务器类 - 管理多个TCP客户端连接
		 * 
		 * 功能说明：
		 * TcpServer提供了完整的TCP服务器功能，包括监听端口、接受连接、管理会话等。
		 * 它使用Connection类封装每个客户端连接，并通过信号槽机制通知上层应用。
		 * 
		 * 工作原理：
		 * 1. 服务器启动：创建监听Socket并绑定到指定端口
		 * 2. 接受连接：当有客户端连接时，创建Connection对象
		 * 3. 会话管理：使用map管理所有活跃的连接
		 * 4. 事件通知：通过信号槽通知新连接、数据接收、连接关闭等事件
		 * 5. 资源清理：连接关闭时自动清理资源
		 * 
		 * 服务器生命周期：
		 * 
		 *   Startup()
		 *      ↓
		 *   创建监听Socket
		 *      ↓
		 *   Bind + Listen
		 *      ↓
		 *   ┌──────────────┐
		 *   │  等待连接    │
		 *   └──────────────┘
		 *      ↓
		 *   OnRead() - Accept新连接
		 *      ↓
		 *   创建Connection对象
		 *      ↓
		 *   触发SignalOnNewConnection
		 *      ↓
		 *   ┌──────────────┐
		 *   │  数据收发    │ ←──┐
		 *   └──────────────┘    │
		 *      ↓               │
		 *   OnSessionRecv()    │
		 *      ↓               │
		 *   触发SignalOnRecv ──┘
		 *      ↓
		 *   OnSessionClose()
		 *      ↓
		 *   触发SignalOnDestory
		 *      ↓
		 *   清理Connection
		 * 
		 * 使用场景：
		 * - HTTP服务器（Web服务、API服务）
		 * - RTMP推流服务器
		 * - GB28181视频监控服务器
		 * - FLV直播服务器
		 * - WebSocket服务器
		 * 
		 * 使用示例：
		 * @code
		 * // 创建TCP服务器
		 * TcpServer* server = new TcpServer();
		 * 
		 * // 监听新连接
		 * server->SignalOnNewConnection.connect([](Connection* conn) {
		 *     std::cout << "New client connected" << std::endl;
		 *     
		 *     // 设置HTTP上下文
		 *     auto http_ctx = std::make_shared<HttpContext>();
		 *     conn->SetContext(kHttpContext, http_ctx);
		 * });
		 * 
		 * // 监听数据接收
		 * server->SignalOnRecv.connect([](Connection* conn, const rtc::CopyOnWriteBuffer& data) {
		 *     // 处理接收到的数据
		 *     auto http_ctx = conn->GetContext<HttpContext>(kHttpContext);
		 *     http_ctx->ProcessData(data);
		 * });
		 * 
		 * // 监听连接关闭
		 * server->SignalOnDestory.connect([](Connection* conn) {
		 *     std::cout << "Client disconnected" << std::endl;
		 * });
		 * 
		 * // 启动服务器
		 * if (server->Startup("0.0.0.0", 8080)) {
		 *     std::cout << "Server started on port 8080" << std::endl;
		 * }
		 * 
		 * // 关闭指定连接
		 * server->CloseSession(conn);
		 * @endcode
		 * 
		 * 注意事项：
		 * - 所有网络操作在network_thread中执行
		 * - Connection对象由TcpServer管理生命周期
		 * - 连接关闭时会自动清理资源
		 * - 支持共享资源上下文（kShareResourceContext）
		 * - 使用unique_ptr管理Connection，避免内存泄漏
		 */
		class TcpServer : public sigslot::has_slots<>  //: public   TcpHandler
		{
		public:
			/**
			 * @brief 构造函数
			 * 
			 * 创建ConnectionContext，初始化三个线程：
			 * - signaling_thread：信令线程
			 * - worker_thread：工作线程
			 * - network_thread：网络线程
			 */
			explicit TcpServer();
			
			/**
			 * @brief 析构函数
			 * 
			 * 断开Socket信号连接，释放资源
			 */
			virtual ~TcpServer();

			public:
			/**
			 * @brief 启动TCP服务器
			 * @param ip 监听IP地址（如"0.0.0.0"表示监听所有网卡）
			 * @param port 监听端口号
			 * @return 成功返回true，失败返回false
			 * 
			 * 启动流程：
			 * 1. 创建TCP Socket
			 * 2. 绑定到指定IP和端口
			 * 3. 开始监听（队列长度500）
			 * 4. 初始化Socket信号连接
			 */
			bool Startup(const std::string &ip, uint16_t port); 

			/**
			 * @brief 关闭指定连接
			 * @param conn 要关闭的连接对象
			 * 
			 * 调用Connection的Close方法，异步关闭连接
			 */
			void CloseSession(Connection *conn);
			
			/**
			 * @brief 关闭指定Socket
			 * @param socket 要关闭的Socket对象
			 * 
			 * 直接关闭Socket（不推荐使用，建议使用CloseSession）
			 */
			void Close(rtc::Socket *socket);
			public: 
			// ========== 信号定义 ==========
			
			/**
			 * @brief 新连接建立信号
			 * @param Connection* 新建立的连接对象
			 * 
			 * 当有新客户端连接时触发，此时Connection已创建并初始化
			 */
			sigslot::signal1<Connection*> SignalOnNewConnection;
			
			/**
			 * @brief 数据接收信号
			 * @param Connection* 接收数据的连接对象
			 * @param rtc::CopyOnWriteBuffer& 接收到的数据
			 * 
			 * 当连接接收到数据时触发
			 */
			sigslot::signal2<Connection*, const rtc::CopyOnWriteBuffer&> SignalOnRecv;
			
			/**
			 * @brief 数据发送完成信号
			 * @param Connection* 发送数据的连接对象
			 * 
			 * 当数据发送完成时触发
			 */
			sigslot::signal1<Connection*> SignalOnSent;
			
			/**
			 * @brief 连接销毁信号
			 * @param Connection* 即将销毁的连接对象
			 * 
			 * 当连接关闭并即将销毁时触发，此时Connection仍然有效
			 */
			sigslot::signal1<Connection*> SignalOnDestory;
			

			public:
			// ========== 上下文管理接口 ==========
			
			/**
			 * @brief 设置服务器级别的上下文对象（拷贝版本）
			 * @param type 上下文类型
			 * @param context 上下文对象的shared_ptr
			 * 
			 * 服务器级别的上下文会自动传递给新建立的连接
			 */
			void SetContext(int type, const std::shared_ptr<void> &context);
			
			/**
			 * @brief 设置服务器级别的上下文对象（移动版本）
			 * @param type 上下文类型
			 * @param context 上下文对象的shared_ptr（移动语义）
			 */
			void SetContext(int type, std::shared_ptr<void> &&context);
			
			/**
			 * @brief 获取服务器级别的上下文对象
			 * @tparam T 上下文对象的实际类型
			 * @param type 上下文类型
			 * @return 类型安全的上下文对象指针
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
			 */
			void ClearContext();
			public:
			// ========== 线程访问接口 ==========
			
			/**
			 * @brief 获取信令线程
			 * @return 信令线程指针
			 */
			rtc::Thread* signaling_thread() { return context_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return context_->signaling_thread(); }
			
			/**
			 * @brief 获取工作线程
			 * @return 工作线程指针
			 */
			rtc::Thread* worker_thread() { return context_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return context_->worker_thread(); }
			
			/**
			 * @brief 获取网络线程
			 * @return 网络线程指针
			 * 
			 * 所有网络操作必须在此线程执行
			 */
			rtc::Thread* network_thread() { return context_->network_thread(); }
			const rtc::Thread* network_thread() const { return context_->network_thread(); }


	
			public:
			// ========== Connection事件处理 ==========
			
			/**
			 * @brief Connection数据接收回调
			 * @param conn 接收数据的连接对象
			 * @param data 接收到的数据
			 * 
			 * 转发Connection的数据接收事件到SignalOnRecv信号
			 */
			void OnSessionRecv(Connection*  conn, const rtc::CopyOnWriteBuffer & data);
			
			/**
			 * @brief Connection关闭回调
			 * @param conn 关闭的连接对象
			 * 
			 * 处理流程：
			 * 1. 在网络线程中执行清理
			 * 2. 触发SignalOnDestory信号
			 * 3. 断开Connection的所有信号连接
			 * 4. 从tcp_sessions_中移除
			 * 5. 释放Connection对象
			 */
			void OnSessionClose(Connection*  conn);
			
		public:
			// ========== Socket事件处理 ==========
			
			/**
			 * @brief 初始化监听Socket的信号连接
			 */
			void InitSocketSignals();
			
			/**
			 * @brief Socket连接成功回调（监听Socket不会触发）
			 * @param socket Socket对象
			 */
			void OnConnect(rtc::Socket* socket);
			
			/**
			 * @brief Socket关闭回调
			 * @param socket 关闭的Socket对象
			 * @param ret 关闭原因代码
			 * 
			 * 处理客户端Socket关闭事件
			 */
			void OnClose(rtc::Socket* socket, int ret);
			
			/**
			 * @brief Socket可读回调
			 * @param socket 可读的Socket对象
			 * 
			 * 处理流程：
			 * 1. 调用Accept接受新连接
			 * 2. 创建Connection对象
			 * 3. 连接Connection的信号
			 * 4. 传递共享资源上下文
			 * 5. 触发SignalOnNewConnection信号
			 */
			void OnRead(rtc::Socket* socket);
			
			/**
			 * @brief Socket可写回调
			 * @param socket 可写的Socket对象
			 */
			void OnWrite(rtc::Socket* socket);

			private:
			// ========== 成员变量 ==========
			
			rtc::scoped_refptr<libp2p_peerconnection::ConnectionContext>	context_;   ///< 连接上下文（管理三个线程）
			rtc::SocketAddress server_address_;                                         ///< 服务器监听地址
			std::unique_ptr<rtc::Socket> control_socket_;                               ///< 监听Socket
			rtc::AsyncResolver* resolver_;                                              ///< 异步DNS解析器（未使用）
			std::map<rtc::Socket*, std::unique_ptr<libnetwork::Connection>> tcp_sessions_;  ///< TCP会话映射表（Socket -> Connection）
			std::unordered_map<int, std::shared_ptr<void>> contexts_;                   ///< 服务器级别的上下文映射表
		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_