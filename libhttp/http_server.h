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

【HTTP服务器类文件】

本文件定义了HttpServer类，用于实现HTTP服务器功能。

【核心功能】
1. TCP服务器管理：监听端口，接受客户端连接
2. HTTP连接管理：为每个连接创建HttpContext
3. 请求分发：通过信号机制将HTTP请求分发给上层处理
4. 响应发送：管理HTTP响应的发送过程
5. 多线程支持：使用独立的网络线程处理IO

【架构设计】
┌─────────────────────────────────────────────┐
│  HttpServer                                 │
│    ├─ TcpServer (TCP连接管理)              │
│    │    ├─ Connection 1 → HttpContext 1    │
│    │    ├─ Connection 2 → HttpContext 2    │
│    │    └─ Connection N → HttpContext N    │
│    │                                        │
│    └─ 信号处理                              │
│         ├─ OnNewConnection                 │
│         ├─ OnRecv → Parse → OnRequest      │
│         ├─ OnSent                           │
│         └─ OnDestory                        │
└─────────────────────────────────────────────┘

【线程模型】
- signaling_thread: 信令线程（用于信令处理）
- worker_thread: 工作线程（用于业务逻辑）
- network_thread: 网络线程（用于IO操作）

【使用场景】
1. HTTP API服务器：提供RESTful API接口
2. 流媒体服务器：提供HLS、FLV等流媒体服务
3. 文件服务器：提供静态文件下载服务
4. WebSocket服务器：升级HTTP连接为WebSocket

【使用示例】
@code
// 创建HTTP服务器
HttpServer server;

// 连接信号处理
server.SignalOnRequest.connect([](Connection* conn, 
                                   const HttpRequestPtr& req, 
                                   const PacketPtr& packet) {
    // 处理HTTP请求
    if (req->Path() == "/api/test") {
        auto response = std::make_shared<HttpRequest>(false);
        response->SetStatusCode(200);
        response->SetBody("{\"status\":\"ok\"}");
        
        auto context = conn->GetContext<HttpContext>(kHttpContext);
        context->PostRequest(response);
    }
});

// 启动服务器
server.Startup("0.0.0.0", 8080);
@endcode

【CORS支持】
服务器默认支持跨域资源共享（CORS），自动添加以下头部：
- Access-Control-Allow-Origin: *
- Access-Control-Allow-Methods: POST, GET, OPTIONS
- Access-Control-Allow-Headers: content-type

【作者的思考】
HTTP服务器的设计体现了"分层架构"的思想。底层的TcpServer处理TCP连接，
中间的HttpContext处理HTTP协议，上层的业务逻辑通过信号机制与HTTP层解耦。
这种设计使得每一层都可以独立测试和替换，是网络服务器设计的经典模式。

 ******************************************************************************/

#ifndef _C_LIBHTTP_SERVER_H_
#define _C_LIBHTTP_SERVER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 ////////////////////
#include "libcross_platform_collection_render/video_render/cvideo_render_factory.h"
#include "libcross_platform_collection_render/video_render/cvideo_render.h"
#include "libcross_platform_collection_render/track_capture/ctrack_capture.h"
#include "libmedia_transfer_protocol/rtp_packet_sink_interface.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_received.h"
#include "libmedia_codec/video_codec_type.h"
#include "libmedia_codec/video_codecs/h264_decoder.h"
#include "libmedia_codec/video_codecs/nal_parse_factory.h"
#include "libmedia_transfer_protocol/rtp_stream_receiver_controller.h"
#include "libmedia_transfer_protocol/librtsp/rtsp_session.h"
#include "libmedia_transfer_protocol/video_receive_stream.h"
#include "libp2p_peerconnection/connection_context.h"
#include "libcross_platform_collection_render/audio_capture/audio_capture.h"
#include "libmedia_transfer_protocol/libhttp/http_session.h"
#include "libmedia_transfer_protocol/libhttp/msg_buffer.h"
#include "libmedia_transfer_protocol/libhttp/packet.h"
#include "libmedia_transfer_protocol/libhttp/http_request.h"
#include "libmedia_transfer_protocol/libnetwork/tcp_server.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		/**
		 * @class HttpServer
		 * @brief HTTP服务器类
		 * 
		 * 基于TcpServer实现的HTTP服务器，支持：
		 * - 多连接管理
		 * - HTTP/1.1协议
		 * - 分块传输编码
		 * - 流式传输
		 * - CORS跨域支持
		 * 
		 * 【工作流程】
		 * 1. 启动服务器，监听指定端口
		 * 2. 接受客户端连接，创建HttpContext
		 * 3. 接收数据，解析HTTP请求
		 * 4. 触发SignalOnRequest信号，通知上层
		 * 5. 上层处理请求，通过HttpContext发送响应
		 * 6. 连接关闭，清理资源
		 * 
		 * 【信号机制】
		 * 使用sigslot库实现观察者模式：
		 * - 服务器触发信号
		 * - 上层连接信号槽处理事件
		 * - 实现松耦合的事件驱动架构
		 * 
		 * 【线程安全】
		 * 所有网络操作在network_thread中执行
		 * 信号回调在相应的线程中执行
		 */
		class HttpServer : public sigslot::has_slots<>
		{
		public:
			/**
			 * @brief 构造函数
			 * 
			 * 创建HTTP服务器实例，初始化TCP服务器和信号连接
			 */
			explicit HttpServer();
			
			/**
			 * @brief 析构函数
			 * 
			 * 断开所有信号连接，释放资源
			 */
			virtual ~HttpServer();

		public:
			/**
			 * @brief 启动HTTP服务器
			 * @param ip 监听IP地址（如"0.0.0.0"表示所有接口）
			 * @param port 监听端口号
			 * @return true=成功，false=失败
			 * 
			 * 【注意事项】
			 * - 在network_thread中执行
			 * - 端口被占用会返回false
			 * - 启动成功后开始接受连接
			 */
			bool Startup(const std::string &ip, uint16_t port);

			
		public:
			/**
			 * @brief 新连接建立信号
			 * @param conn 新建立的连接对象
			 * 
			 * 当客户端连接成功时触发
			 */
			sigslot::signal1< libnetwork::Connection*> SignalOnNewConnection;
			
			/**
			 * @brief 连接销毁信号
			 * @param conn 被销毁的连接对象
			 * 
			 * 当连接关闭时触发，用于清理资源
			 */
			sigslot::signal1<libnetwork::Connection*> SignalOnDestory;
			
			/**
			 * @brief 数据发送完成信号
			 * @param conn 连接对象
			 * 
			 * 当HTTP响应发送完成时触发
			 */
			sigslot::signal1< libnetwork::Connection*> SignalOnSent;
			
			/**
			 * @brief 准备发送下一个分块信号
			 * @param conn 连接对象
			 * 
			 * 当一个分块发送完成，准备发送下一个时触发
			 */
			sigslot::signal1< libnetwork::Connection *> SignalOnSentNextChunk;
			
			/**
			 * @brief 接收到HTTP请求信号
			 * @param conn 连接对象
			 * @param http_request HTTP请求对象
			 * @param packet 消息体数据包
			 * 
			 * 当解析出完整的HTTP请求时触发
			 * 上层应在此信号的槽函数中处理请求并发送响应
			 */
			sigslot::signal3< libnetwork::Connection *, const  std::shared_ptr<HttpRequest>, const std::shared_ptr<Packet>> SignalOnRequest;
			
			/**
			 * @brief 准备发送下一个分块（内部回调）
			 * @param conn 连接对象
			 * 
			 * 【内部方法】
			 * 从HttpContext转发到上层
			 */
			void OnSentNextChunk(libnetwork::Connection *conn);
			
			/**
			 * @brief 接收到HTTP请求（内部回调）
			 * @param conn 连接对象
			 * @param http_request HTTP请求对象
			 * @param packet 消息体数据包
			 * 
			 * 【内部方法】
			 * 从HttpContext转发到上层
			 */
			void OnRequest(libnetwork::Connection *conn, const  std::shared_ptr<HttpRequest> http_request, const std::shared_ptr<Packet> packet);

		public:
			/**
			 * @brief 获取信令线程
			 * @return 信令线程指针
			 */
			rtc::Thread* signaling_thread() { return tcp_server_->signaling_thread(); }
			const rtc::Thread* signaling_thread() const { return tcp_server_->signaling_thread(); }
			
			/**
			 * @brief 获取工作线程
			 * @return 工作线程指针
			 */
			rtc::Thread* worker_thread() { return tcp_server_->worker_thread(); }
			const rtc::Thread* worker_thread() const { return tcp_server_->worker_thread(); }
			
			/**
			 * @brief 获取网络线程
			 * @return 网络线程指针
			 * 
			 * 所有网络IO操作在此线程中执行
			 */
			rtc::Thread* network_thread() { return tcp_server_->network_thread(); }
			const rtc::Thread* network_thread() const { return tcp_server_->network_thread(); }
			
		public:
			/**
			 * @brief 初始化信号连接
			 * 
			 * 【内部方法】
			 * 连接TcpServer的信号到HttpServer的槽函数
			 */
			void InitSocketSignals();


			/**
			 * @brief 新连接建立回调
			 * @param conn 新建立的连接对象
			 * 
			 * 【内部方法】
			 * 1. 触发SignalOnNewConnection信号
			 * 2. 创建HttpContext并绑定到连接
			 * 3. 连接HttpContext的信号
			 */
			void OnNewConnection(libnetwork::Connection* conn);
			
			/**
			 * @brief 连接销毁回调
			 * @param conn 被销毁的连接对象
			 * 
			 * 【内部方法】
			 * 1. 触发SignalOnDestory信号
			 * 2. 断开HttpContext的信号连接
			 * 3. 清理HttpContext
			 */
			void OnDestory(libnetwork::Connection* conn);
			
			/**
			 * @brief 接收数据回调
			 * @param conn 连接对象
			 * @param data 接收到的数据
			 * 
			 * 【内部方法】
			 * 1. 将数据放入MsgBuffer
			 * 2. 调用HttpContext::Parse解析
			 * 3. 解析错误则关闭连接
			 */
			void OnRecv(libnetwork::Connection* conn, const rtc::CopyOnWriteBuffer& data);
			
			/**
			 * @brief 数据发送完成回调
			 * @param conn 连接对象
			 * 
			 * 【内部方法】
			 * 调用HttpContext::WriteComplete处理发送完成事件
			 */
			void OnSent(libnetwork::Connection* conn);

		private:
			std::unique_ptr<libnetwork::TcpServer>	tcp_server_;  // TCP服务器对象
		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_