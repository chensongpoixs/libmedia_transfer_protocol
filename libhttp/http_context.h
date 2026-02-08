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

【HTTP上下文类文件】

本文件定义了HttpContext类，用于管理HTTP连接的上下文信息。

【核心功能】
1. HTTP消息解析：使用HttpParser解析接收到的数据
2. HTTP消息发送：管理HTTP响应的发送过程
3. 分块传输管理：支持chunked编码的发送
4. 流式传输管理：支持流式数据的发送
5. 状态管理：跟踪HTTP消息的发送状态

【HTTP发送状态机】
┌──────────────────────────────────────────────────┐
│  kHttpContextPostInit → 初始状态                 │
│       ↓                                          │
│  kHttpContextPostHttp → 发送完整HTTP消息         │
│  kHttpContextPostHttpHeader → 发送HTTP头部       │
│       ↓                                          │
│  kHttpContextPostHttpBody → 发送HTTP消息体       │
│       ↓                                          │
│  kHttpContextPostChunkHeader → 发送分块头部      │
│  kHttpContextPostChunkLen → 发送分块长度         │
│       ↓                                          │
│  kHttpContextPostChunkBody → 发送分块数据        │
│       ↓                                          │
│  kHttpContextPostChunkEOF → 发送分块结束标记     │
│       ↓                                          │
│  kHttpContextPostHttpStreamHeader → 发送流头部   │
│  kHttpContextPostHttpStreamChunk → 发送流数据    │
└──────────────────────────────────────────────────┘

【使用场景】
1. HTTP服务器：每个连接对应一个HttpContext
2. 请求解析：接收TCP数据，解析HTTP请求
3. 响应发送：构造并发送HTTP响应
4. 流媒体传输：发送视频、音频流数据

【使用示例】
@code
// 创建HTTP上下文
auto context = std::make_shared<HttpContext>(connection);

// 接收数据并解析
MsgBuffer buffer;
buffer.Append(tcp_data, tcp_data_len);
context->Parse(buffer);

// 发送普通响应
auto response = std::make_shared<HttpRequest>(false);
response->SetStatusCode(200);
response->SetBody("Hello World");
context->PostRequest(response);

// 发送分块响应
context->PostChunkHeader(response->MakeHeaders());
context->PostChunk(chunk1);
context->PostChunk(chunk2);
context->PostEofChunk();
@endcode

【分块传输格式】
HTTP/1.1 200 OK\r\n
Transfer-Encoding: chunked\r\n
\r\n
5\r\n
Hello\r\n
6\r\n
 World\r\n
0\r\n
\r\n

【作者的思考】
HTTP上下文的设计体现了"关注点分离"的原则。它将HTTP协议处理与底层TCP连接分离，
使得HTTP逻辑可以独立于网络层实现。这种设计使代码更易于测试和维护。

 ******************************************************************************/

#ifndef _C_HTTP_CONTEXT_H_
#define _C_HTTP_CONTEXT_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
 ////////////////////
#include "libmedia_transfer_protocol/libhttp/http_context.h"
#include "libmedia_transfer_protocol/libhttp/http_session.h"
#include "libmedia_transfer_protocol/libhttp/packet.h"
#include "libmedia_transfer_protocol/libhttp/msg_buffer.h"
#include "libmedia_transfer_protocol/libhttp/http_parser.h"
#include "libmedia_transfer_protocol/libnetwork/tcp_server.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		/**
		 * @enum HttpContextPostState
		 * @brief HTTP发送状态枚举
		 * 
		 * 定义了HTTP响应发送过程中的各个状态。
		 * 用于跟踪当前正在发送的内容类型和发送进度。
		 */
		enum HttpContextPostState
		{
			kHttpContextPostInit,              // 初始状态，未开始发送
			kHttpContextPostHttp,              // 发送完整HTTP消息（头部+消息体）
			kHttpContextPostHttpHeader,        // 发送HTTP头部
			kHttpContextPostHttpBody,          // 发送HTTP消息体
			kHttpContextPostHttpStreamHeader,  // 发送流式传输的头部
			kHttpContextPostHttpStreamChunk,   // 发送流式传输的数据块
			kHttpContextPostChunkHeader,       // 发送分块传输的头部
			kHttpContextPostChunkLen,          // 发送分块长度
			kHttpContextPostChunkBody,         // 发送分块数据
			kHttpContextPostChunkEOF           // 发送分块结束标记（0\r\n\r\n）
		};
		
		/**
		 * @class HttpContext
		 * @brief HTTP连接上下文类
		 * 
		 * 管理单个HTTP连接的完整生命周期，包括：
		 * - 接收和解析HTTP请求
		 * - 构造和发送HTTP响应
		 * - 支持普通、分块、流式三种传输模式
		 * 
		 * 【传输模式】
		 * 1. 普通模式：一次性发送完整的HTTP消息
		 * 2. 分块模式：使用chunked编码分块发送数据
		 * 3. 流式模式：持续发送数据流，无固定长度
		 * 
		 * 【信号机制】
		 * 使用sigslot信号槽机制通知上层：
		 * - SignalOnSent: 消息发送完成
		 * - SignalOnSentNextChunk: 准备发送下一个分块
		 * - SignalOnRequest: 接收到完整的HTTP请求
		 * 
		 * 【线程安全】
		 * 本类不是线程安全的，应在网络线程中使用。
		 */
		class HttpContext
		{
		public:
			/**
			 * @brief 构造函数
			 * @param conn TCP连接对象指针
			 * 
			 * 创建HTTP上下文，绑定到指定的TCP连接
			 */
			HttpContext(libnetwork::Connection*conn );
			~HttpContext() = default;

			/**
			 * @brief 解析HTTP消息
			 * @param buf 接收缓冲区
			 * @return 解析结果（1=成功，-1=错误）
			 * 
			 * 【处理流程】
			 * 1. 调用HttpParser解析缓冲区数据
			 * 2. 如果解析完成，触发SignalOnRequest信号
			 * 3. 如果解析错误，返回-1（上层应关闭连接）
			 * 
			 * 【使用方式】
			 * 在TCP连接的OnRecv回调中调用此方法
			 */
			int32_t Parse(MsgBuffer &buf);
			
			/**
			 * @brief 发送完整的HTTP消息
			 * @param header_and_body 包含头部和消息体的完整字符串
			 * @return true=成功，false=失败（状态不正确）
			 * 
			 * 适用于小消息的一次性发送
			 * 要求当前状态为kHttpContextPostInit
			 */
			bool PostRequest(const std::string &header_and_body);
			
			/**
			 * @brief 发送HTTP消息（头部+数据包）
			 * @param header HTTP头部字符串
			 * @param packet 消息体数据包
			 * @return true=成功，false=失败
			 * 
			 * 先发送头部，发送完成后自动发送消息体
			 */
			bool PostRequest(const std::string &header, std::shared_ptr<Packet> &packet);
			
			/**
			 * @brief 发送HTTP请求/响应对象
			 * @param request HTTP请求/响应对象
			 * @return true=成功，false=失败
			 * 
			 * 根据对象的传输模式自动选择发送方式：
			 * - 普通模式：调用PostRequest
			 * - 分块模式：调用PostChunkHeader
			 * - 流式模式：调用PostStreamHeader
			 */
			bool PostRequest(std::shared_ptr<HttpRequest> &request);
			
			/**
			 * @brief 发送分块传输的头部
			 * @param header HTTP头部字符串（包含Transfer-Encoding: chunked）
			 * @return true=成功，false=失败
			 * 
			 * 开始分块传输，发送头部后等待PostChunk调用
			 */
			bool PostChunkHeader(const std::string &header);
			
			/**
			 * @brief 发送一个分块
			 * @param chunk 分块数据包
			 * 
			 * 【发送格式】
			 * 分块长度（十六进制）\r\n
			 * 分块数据\r\n
			 * 
			 * 发送完成后触发SignalOnSentNextChunk信号
			 */
			void PostChunk(std::shared_ptr<Packet> &chunk);
			
			/**
			 * @brief 发送分块结束标记
			 * 
			 * 发送：0\r\n\r\n
			 * 表示分块传输结束
			 * 发送完成后触发SignalOnSent信号
			 */
			void PostEofChunk();
			
			/**
			 * @brief 发送流式传输的头部
			 * @param header HTTP头部字符串
			 * @return true=成功，false=失败
			 * 
			 * 开始流式传输，发送头部后等待PostStreamChunk调用
			 */
			bool PostStreamHeader(const std::string &header);
			
			/**
			 * @brief 发送流式数据块
			 * @param packet 数据包
			 * @return true=成功，false=失败
			 * 
			 * 持续发送数据流，每次发送完成触发SignalOnSentNextChunk信号
			 */
			bool PostStreamChunk(std::shared_ptr<Packet> &packet);
			
			/**
			 * @brief 写入完成回调
			 * @param conn TCP连接对象
			 * 
			 * 【内部方法】
			 * 在TCP连接的OnSent回调中调用
			 * 根据当前状态决定下一步操作：
			 * - 继续发送剩余数据
			 * - 触发相应的信号通知上层
			 */
			void WriteComplete(libnetwork::Connection *);


		public:
			/**
			 * @brief 发送完成信号
			 * @param conn TCP连接对象
			 * 
			 * 当完整的HTTP消息或分块传输结束时触发
			 */
			sigslot::signal1<libnetwork::Connection *> SignalOnSent;
			
			/**
			 * @brief 准备发送下一个分块信号
			 * @param conn TCP连接对象
			 * 
			 * 当一个分块或流数据块发送完成，准备发送下一个时触发
			 */
			sigslot::signal1<libnetwork::Connection *> SignalOnSentNextChunk;
			
			/**
			 * @brief 接收到HTTP请求信号
			 * @param conn TCP连接对象
			 * @param http_request HTTP请求对象
			 * @param packet 消息体数据包
			 * 
			 * 当解析出完整的HTTP请求时触发
			 */
			sigslot::signal3<libnetwork::Connection *,  const  std::shared_ptr<HttpRequest> , const std::shared_ptr<Packet>> SignalOnRequest;
			
		private:
			libnetwork::Connection* connection_;                // TCP连接对象
			HttpParser http_parser_;                            // HTTP解析器
			std::string header_;                                // 待发送的头部字符串
			std::shared_ptr<Packet> out_pakcet_;                // 待发送的数据包
			HttpContextPostState post_state_{ kHttpContextPostInit };  // 发送状态
			bool header_sent_;                                  // 头部是否已发送
		};



	}

}


#endif // _C_HTTP_CONTEXT_H_