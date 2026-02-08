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

【HTTP解析器类文件】

本文件定义了HttpParser类，用于解析HTTP消息流。

【核心功能】
1. HTTP消息解析：解析请求行/状态行、头部、消息体
2. 分块传输解析：支持Transfer-Encoding: chunked
3. 流式传输解析：支持无固定长度的流式数据
4. 状态机驱动：使用状态机模式处理解析过程

【解析状态机】
┌──────────────────────────────────────────────────────┐
│  kExpectHeaders → 解析头部                           │
│       ↓                                              │
│  kExpectNormalBody → 解析固定长度消息体              │
│  kExpectChunkLen → 解析分块长度                      │
│  kExpectStreamBody → 解析流式数据                    │
│       ↓                                              │
│  kExpectChunkBody → 解析分块数据                     │
│       ↓                                              │
│  kExpectLastEmptyChunk → 解析最后的空分块            │
│       ↓                                              │
│  kExpectHttpComplete/kExpectChunkComplete → 完成     │
└──────────────────────────────────────────────────────┘

【HTTP消息格式】
普通消息：
  头部\r\n\r\n + 消息体（Content-Length指定长度）

分块消息：
  头部\r\n\r\n + 分块1长度\r\n + 分块1数据\r\n + ... + 0\r\n\r\n

流式消息：
  头部\r\n\r\n + 持续的数据流（无固定长度）

【使用场景】
1. HTTP服务器：解析客户端请求
2. HTTP客户端：解析服务器响应
3. 代理服务器：解析和转发HTTP消息
4. 协议分析工具：分析HTTP流量

【使用示例】
@code
HttpParser parser;
MsgBuffer buffer;

// 接收数据到缓冲区
buffer.Append(tcp_data, tcp_data_len);

// 解析数据
HttpParserState state = parser.Parse(buffer);

if (state == kExpectHttpComplete) {
    // 解析完成，获取请求对象
    auto request = parser.GetHttpRequest();
    auto packet = parser.Chunk();
    
    // 处理请求...
}
else if (state == kExpectChunkComplete) {
    // 分块完成，获取分块数据
    auto chunk = parser.Chunk();
    
    // 处理分块...
}
else if (state == kExpectError) {
    // 解析错误
    HttpStatusCode reason = parser.Reason();
}
@endcode

【性能优化】
1. 增量解析：支持流式解析，无需等待完整消息
2. 零拷贝：直接在缓冲区中解析，避免数据拷贝
3. 状态保持：保存解析状态，支持分段接收数据

【作者的思考】
HTTP解析器的设计是一个经典的状态机应用。每个状态代表解析过程中的一个阶段，
状态转换由接收到的数据驱动。这种设计使得解析器能够处理不完整的数据，
支持流式解析，是网络协议解析的标准模式。

 ******************************************************************************/

#ifndef _C_HTTP_PARSER_H_
#define _C_HTTP_PARSER_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "libmedia_transfer_protocol/libhttp/http_utils.h"
#include "libmedia_transfer_protocol/libhttp/http_request.h"
#include "rtc_base/buffer.h"
#include "libmedia_transfer_protocol/libhttp/msg_buffer.h"

#include "libmedia_transfer_protocol/libhttp/packet.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		/**
		 * @enum HttpParserState
		 * @brief HTTP解析器状态枚举
		 * 
		 * 定义了HTTP消息解析过程中的各个状态。
		 * 解析器根据当前状态和接收到的数据进行状态转换。
		 * 
		 * 【状态转换图】
		 * ┌─────────────────────────────────────────────┐
		 * │  kExpectHeaders                             │
		 * │    ↓                                        │
		 * │  ├→ kExpectNormalBody → kExpectHttpComplete │
		 * │  ├→ kExpectChunkLen → kExpectChunkBody →   │
		 * │  │    kExpectChunkComplete                  │
		 * │  └→ kExpectStreamBody → kExpectChunkComplete│
		 * │                                             │
		 * │  kExpectError (任何状态都可能转到错误)      │
		 * └─────────────────────────────────────────────┘
		 */
		enum HttpParserState
		{
			kExpectHeaders,          // 期待解析头部（初始状态）
			kExpectNormalBody,       // 期待解析普通消息体（Content-Length指定长度）
			kExpectStreamBody,       // 期待解析流式消息体（无固定长度）
			kExpectHttpComplete,     // HTTP消息解析完成
			kExpectChunkLen,         // 期待解析分块长度
			kExpectChunkBody,        // 期待解析分块数据
			kExpectChunkComplete,    // 分块解析完成
			kExpectLastEmptyChunk,   // 期待解析最后的空分块（0\r\n\r\n）

			kExpectContinue,         // 继续等待更多数据
			kExpectError,            // 解析错误
		};

		/**
		 * @class HttpParser
		 * @brief HTTP消息解析器
		 * 
		 * 使用状态机模式解析HTTP消息流，支持：
		 * - 普通消息（Content-Length）
		 * - 分块传输（Transfer-Encoding: chunked）
		 * - 流式传输（无固定长度）
		 * 
		 * 【解析流程】
		 * 1. 解析头部：查找\r\n\r\n，提取请求行/状态行和头部字段
		 * 2. 确定消息体类型：根据Content-Length或Transfer-Encoding
		 * 3. 解析消息体：根据类型采用不同的解析策略
		 * 4. 返回解析状态：完成、继续或错误
		 * 
		 * 【分块传输格式】
		 * 分块长度（十六进制）\r\n
		 * 分块数据\r\n
		 * ...
		 * 0\r\n
		 * \r\n
		 * 
		 * 【线程安全】
		 * 本类不是线程安全的，每个连接应使用独立的解析器实例。
		 */
		class HttpParser
		{
		public:
			/**
			 * @brief 默认构造函数
			 * 
			 * 初始状态为kExpectHeaders
			 */
			HttpParser() = default;
			~HttpParser() = default;

			/**
			 * @brief 解析HTTP消息
			 * @param buf 消息缓冲区
			 * @return 解析状态
			 * 
			 * 【返回值说明】
			 * - kExpectHttpComplete: 完整HTTP消息解析完成
			 * - kExpectChunkComplete: 一个分块解析完成
			 * - kExpectContinue: 需要更多数据继续解析
			 * - kExpectError: 解析错误（可通过Reason()获取错误原因）
			 * 
			 * 【使用方式】
			 * 1. 接收TCP数据到缓冲区
			 * 2. 调用Parse()解析
			 * 3. 根据返回状态处理：
			 *    - 完成：获取请求对象和数据包
			 *    - 继续：等待更多数据
			 *    - 错误：关闭连接
			 * 
			 * 【注意事项】
			 * - 可以多次调用Parse()，支持增量解析
			 * - 解析完成后会自动清理状态，准备解析下一个消息
			 * - 缓冲区中已解析的数据会被自动移除
			 */
			HttpParserState Parse(MsgBuffer &buf);
			
			/**
			 * @brief 获取解析出的数据包
			 * @return 数据包指针（包含消息体或分块数据）
			 * 
			 * - 普通消息：返回完整消息体
			 * - 分块消息：返回当前分块的数据
			 * - 流式消息：返回当前接收到的数据
			 */
			const std::shared_ptr<Packet> &Chunk() const;
			
			/**
			 * @brief 获取解析错误原因
			 * @return HTTP状态码（表示错误类型）
			 * 
			 * 当Parse()返回kExpectError时调用，获取具体错误原因
			 * 常见错误：
			 * - k400BadRequest: 请求格式错误
			 * - k413RequestEntityTooLarge: 消息体过大
			 */
			HttpStatusCode Reason() const;
			
			/**
			 * @brief 清理状态，准备解析下一个HTTP消息
			 * 
			 * 重置所有状态变量，释放资源
			 */
			void ClearForNextHttp();
			
			/**
			 * @brief 清理状态，准备解析下一个分块
			 * 
			 * 保持分块传输状态，只清理当前分块的数据
			 */
			void ClearForNextChunk();
			
			/**
			 * @brief 获取解析出的HTTP请求/响应对象
			 * @return HTTP请求/响应对象指针
			 * 
			 * 在解析完成后调用，获取解析结果
			 */
			std::shared_ptr<HttpRequest> GetHttpRequest() const
			{
				return req_;
			}
			
		private:
			/**
			 * @brief 解析流式消息体
			 * @param buf 消息缓冲区
			 * 
			 * 从缓冲区读取数据到chunk_，直到缓冲区为空或chunk_满
			 */
			void ParseStream(MsgBuffer &buf);
			
			/**
			 * @brief 解析普通消息体
			 * @param buf 消息缓冲区
			 * 
			 * 根据Content-Length读取固定长度的数据
			 */
			void ParseNormalBody(MsgBuffer &buf);
			
			/**
			 * @brief 解析分块数据
			 * @param buf 消息缓冲区
			 * 
			 * 读取当前分块的数据（不包括结尾的\r\n）
			 */
			void ParseChunk(MsgBuffer &buf);
			
			/**
			 * @brief 解析HTTP头部
			 * 
			 * 解析header_字符串，提取：
			 * 1. 请求行/状态行
			 * 2. 所有头部字段
			 * 3. 确定消息体类型（普通/分块/流式）
			 */
			void ParseHeaders();
			
			/**
			 * @brief 处理请求行/状态行
			 * @param line 第一行内容
			 * 
			 * 请求行格式：方法 路径 版本
			 * 状态行格式：版本 状态码 状态消息
			 */
			void ProcessMethodLine(const std::string &line);

			HttpParserState state_{ kExpectHeaders };           // 当前解析状态
			int32_t current_chunk_length_{ 0 };                 // 当前分块长度
			int32_t current_content_length_{ 0 };               // 当前消息体长度
			bool is_stream_{ false };                           // 是否流式传输
			bool is_chunked_{ false };                          // 是否分块传输
			bool is_request_{ true };                           // true=请求，false=响应
			HttpStatusCode reason_{ kUnknown };                 // 错误原因
			std::string header_;                                // 头部字符串缓存
			std::shared_ptr<Packet> chunk_;                     // 当前数据包
			std::shared_ptr<HttpRequest> req_;                  // 解析出的请求/响应对象
		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_