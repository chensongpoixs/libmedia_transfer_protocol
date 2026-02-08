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

【HTTP请求/响应类文件】

本文件定义了HttpRequest类，用于封装HTTP请求和响应消息。

【核心功能】
1. HTTP请求封装：支持GET、POST等各种HTTP方法
2. HTTP响应封装：支持各种状态码的响应
3. 头部管理：添加、删除、查询HTTP头部字段
4. 参数解析：解析URL查询参数
5. 消息构造：生成完整的HTTP消息字符串

【HTTP消息格式】
┌─────────────────────────────────────────┐
│  请求行/状态行                          │
│  GET /path?key=value HTTP/1.1           │
├─────────────────────────────────────────┤
│  头部字段                               │
│  Host: example.com                      │
│  Content-Type: application/json         │
│  Content-Length: 123                    │
├─────────────────────────────────────────┤
│  空行（\r\n）                           │
├─────────────────────────────────────────┤
│  消息体（可选）                         │
│  {"key": "value"}                       │
└─────────────────────────────────────────┘

【请求行格式】
方法 路径?参数 版本
GET /api/users?id=123 HTTP/1.1

【状态行格式】
版本 状态码 状态消息
HTTP/1.1 200 OK

【使用场景】
1. HTTP客户端：构造请求发送给服务器
2. HTTP服务器：解析请求、构造响应
3. 代理服务器：转发和修改HTTP消息
4. API测试：构造各种HTTP请求进行测试

【使用示例】
@code
// 构造GET请求
auto request = std::make_shared<HttpRequest>(true);
request->SetMethod(kGet);
request->SetPath("/api/users");
request->SetParameter("id", "123");
request->AddHeader("Host", "example.com");
std::string msg = request->AppendToBuffer();

// 构造200响应
auto response = std::make_shared<HttpRequest>(false);
response->SetStatusCode(200);
response->AddHeader("Content-Type", "application/json");
response->SetBody("{\"status\":\"ok\"}");
std::string resp = response->AppendToBuffer();

// 构造404响应
auto notFound = HttpRequest::NewHttp404Response();
@endcode

【流式传输支持】
1. 普通模式：Content-Length指定消息体长度
2. 分块模式：Transfer-Encoding: chunked
3. 流式模式：持续发送数据，无固定长度

【作者的思考】
HTTP协议的设计体现了"简单即美"的哲学。文本格式的消息易于调试，
头部字段的扩展性使协议能够不断演进。虽然HTTP/2采用了二进制格式，
但HTTP/1.1的文本格式仍然是理解HTTP协议的最佳起点。

 ******************************************************************************/

#ifndef _C_HTTP_REQUEST_H_
#define _C_HTTP_REQUEST_H_

#include <algorithm>
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "libmedia_transfer_protocol/libhttp/http_utils.h"
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		/**
		 * @class HttpRequest
		 * @brief HTTP请求/响应封装类
		 * 
		 * 统一封装HTTP请求和响应消息，通过is_request_标志区分。
		 * 支持完整的HTTP/1.1协议特性，包括分块传输、流式传输等。
		 * 
		 * 【双重身份】
		 * - 请求模式（is_request_=true）：封装HTTP请求
		 * - 响应模式（is_request_=false）：封装HTTP响应
		 * 
		 * 【消息组成】
		 * 1. 起始行：请求行或状态行
		 * 2. 头部字段：键值对形式的元数据
		 * 3. 空行：分隔头部和消息体
		 * 4. 消息体：实际传输的数据（可选）
		 * 
		 * 【传输模式】
		 * - 普通模式：使用Content-Length指定长度
		 * - 分块模式：使用Transfer-Encoding: chunked
		 * - 流式模式：持续传输，无固定长度
		 * 
		 * 【线程安全】
		 * 本类不是线程安全的，多线程访问需要外部同步。
		 */
		class HttpRequest
		{
		public:
			/**
			 * @brief 构造函数
			 * @param is_request true表示请求，false表示响应
			 */
			explicit HttpRequest(bool is_request = true);
			
			/**
			 * @brief 获取所有头部字段
			 * @return 头部字段的map（键已转换为小写）
			 */
			const std::unordered_map<std::string, std::string> &Headers() const;
			
			/**
			 * @brief 添加头部字段
			 * @param field 字段名（会自动转换为小写）
			 * @param value 字段值
			 * 
			 * 如果字段已存在，会覆盖原值
			 */
			void AddHeader(const std::string &field, const std::string &value);
			
			/**
			 * @brief 删除头部字段
			 * @param key 字段名（会自动转换为小写）
			 */
			void RemoveHeader(const std::string &key);
			
			/**
			 * @brief 获取头部字段值
			 * @param field 字段名（会自动转换为小写）
			 * @return 字段值，不存在返回空字符串
			 * 
			 * 示例：
			 * @code
			 * std::string contentType = request->GetHeader("content-type");
			 * @endcode
			 */
			const std::string &GetHeader(const std::string &field) const;
			
			/**
			 * @brief 添加头部字段（移动语义版本）
			 * @param field 字段名（会自动转换为小写）
			 * @param value 字段值
			 * 
			 * 使用移动语义避免字符串拷贝
			 */
			void AddHeader(std::string &&field, std::string &&value);
			
			/**
			 * @brief 生成HTTP头部字符串
			 * @return 包含起始行和所有头部字段的字符串
			 * 
			 * 格式：
			 * 起始行\r\n
			 * 字段1: 值1\r\n
			 * 字段2: 值2\r\n
			 * \r\n
			 */
			std::string MakeHeaders();
			
			/**
			 * @brief 设置查询字符串
			 * @param query 查询字符串（不包含?）
			 * 
			 * 会自动解析参数到parameters_
			 * 示例："key1=value1&key2=value2"
			 */
			void SetQuery(const std::string &query);
			
			/**
			 * @brief 设置查询字符串（移动语义版本）
			 * @param query 查询字符串
			 */
			void SetQuery(std::string &&query);
			
			/**
			 * @brief 设置URL参数
			 * @param key 参数名
			 * @param value 参数值
			 */
			void SetParameter(const std::string &key, const std::string &value);
			
			/**
			 * @brief 设置URL参数（移动语义版本）
			 * @param key 参数名
			 * @param value 参数值
			 */
			void SetParameter(std::string &&key, std::string &&value);
			
			/**
			 * @brief 获取URL参数值
			 * @param key 参数名
			 * @return 参数值，不存在返回空字符串
			 */
			const std::string &GetParameter(const std::string &key) const;
			
			/**
			 * @brief 获取查询参数值（GetParameter的别名）
			 * @param key 参数名
			 * @return 参数值
			 */
			const std::string &GetQueryParam(const std::string &key) const;
			
			/**
			 * @brief 获取查询字符串
			 * @return 查询字符串
			 */
			const std::string &Query() const;

			/**
			 * @brief 设置HTTP方法（字符串版本）
			 * @param method 方法字符串（如"GET"、"POST"）
			 */
			void SetMethod(const std::string &method);
			
			/**
			 * @brief 设置HTTP方法（枚举版本）
			 * @param method 方法枚举值
			 */
			void SetMethod(HttpMethod method);
			
			/**
			 * @brief 获取HTTP方法
			 * @return 方法枚举值
			 */
			HttpMethod Method() const;
			
			/**
			 * @brief 设置HTTP版本（枚举版本）
			 * @param v 版本枚举值
			 */
			void SetVersion(Version v);
			
			/**
			 * @brief 设置HTTP版本（字符串版本）
			 * @param version 版本字符串（如"HTTP/1.1"）
			 */
			void SetVersion(const std::string &version);
			
			/**
			 * @brief 获取HTTP版本
			 * @return 版本枚举值
			 */
			Version GetVersion() const;
			
			/**
			 * @brief 设置请求路径
			 * @param path 路径字符串（如"/api/users"）
			 * 
			 * 如果路径包含URL编码字符，会自动解码
			 */
			void SetPath(const std::string &path);
			
			/**
			 * @brief 获取请求路径
			 * @return 路径字符串
			 */
			const std::string &Path() const;
			
			/**
			 * @brief 设置响应状态码
			 * @param code 状态码（如200、404）
			 */
			void SetStatusCode(int32_t code);
			
			/**
			 * @brief 获取响应状态码
			 * @return 状态码
			 */
			uint32_t GetStatusCode() const;
			
			/**
			 * @brief 设置消息体
			 * @param body 消息体内容
			 */
			void SetBody(const std::string &body);
			
			/**
			 * @brief 设置消息体（移动语义版本）
			 * @param body 消息体内容
			 */
			void SetBody(std::string &&body);
			
			/**
			 * @brief 获取消息体
			 * @return 消息体内容
			 */
			const std::string &Body() const;
			
			/**
			 * @brief 生成完整的HTTP消息
			 * @return 包含头部和消息体的完整消息字符串
			 * 
			 * 格式：头部 + 空行 + 消息体
			 */
			std::string AppendToBuffer();
			
			/**
			 * @brief 判断是否为请求消息
			 * @return true表示请求，false表示响应
			 */
			bool IsRequest() const;
			
			/**
			 * @brief 判断是否为流式传输
			 * @return true表示流式传输
			 */
			bool IsStream() const;
			
			/**
			 * @brief 判断是否为分块传输
			 * @return true表示分块传输
			 */
			bool IsChunked() const;
			
			/**
			 * @brief 设置流式传输标志
			 * @param s true表示流式传输
			 */
			void SetIsStream(bool s);
			
			/**
			 * @brief 设置分块传输标志
			 * @param c true表示分块传输
			 */
			void SetIsChunked(bool c);

			/**
			 * @brief 创建400错误响应
			 * @return 400 Bad Request响应对象
			 * 
			 * 包含CORS头部，允许跨域访问
			 */
			static std::shared_ptr< HttpRequest> NewHttp400Response();
			
			/**
			 * @brief 创建404错误响应
			 * @return 404 Not Found响应对象
			 * 
			 * 包含CORS头部，允许跨域访问
			 */
			static std::shared_ptr< HttpRequest>  NewHttp404Response();
			
			/**
			 * @brief 创建OPTIONS方法响应
			 * @return 200 OK响应对象
			 * 
			 * 用于处理CORS预检请求
			 */
			static std::shared_ptr< HttpRequest>  NewHttpOptionsResponse();
			
		private:
			/**
			 * @brief 追加请求行到字符串流
			 * @param ss 字符串流
			 * 
			 * 格式：方法 路径?参数 版本\r\n
			 */
			void AppendRequestFirstLine(std::stringstream &ss);
			
			/**
			 * @brief 追加状态行到字符串流
			 * @param ss 字符串流
			 * 
			 * 格式：版本 状态码 状态消息\r\n
			 */
			void AppendResponseFirstLine(std::stringstream &ss);

			/**
			 * @brief 解析查询字符串中的参数
			 * 
			 * 解析query_字符串，提取键值对到parameters_
			 * 支持URL解码和空值参数
			 */
			void ParseParameters();

			HttpMethod method_{ kInvalid };                                    // HTTP方法
			Version version_{ Version::kUnknown };                             // HTTP版本
			std::string path_;                                                 // 请求路径
			std::string query_;                                                // 查询字符串
			std::unordered_map<std::string, std::string> headers_;             // 头部字段（键为小写）
			std::unordered_map<std::string, std::string> parameters_;          // URL参数
			std::string body_;                                                 // 消息体
			uint32_t code_{ 0 };                                               // 响应状态码
			bool is_request_{ true };                                          // true=请求，false=响应
			bool is_stream_{ false };                                          // 是否流式传输
			bool is_chunked_{ false };                                         // 是否分块传输

		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_