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

【HTTP类型定义文件】

本文件定义了HTTP协议相关的基础类型和枚举，包括：
- HTTP状态码（HttpStatusCode）
- HTTP版本（Version）
- HTTP内容类型（ContentType）
- HTTP请求方法（HttpMethod）

这些类型是HTTP协议实现的基础，被HTTP解析器、请求处理器等模块广泛使用。

【作者的思考】
HTTP协议是互联网的基石，它的设计简洁而优雅。状态码用三位数字表达了丰富的语义，
方法名用简单的动词描述了操作意图。这种设计哲学值得我们在系统设计中学习：
用最简单的方式表达最丰富的含义。

 ******************************************************************************/

#ifndef _C_LIBHTTP_TYPE_H_
#define _C_LIBHTTP_TYPE_H_

#include <algorithm>

namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		/**
		 * @enum HttpStatusCode
		 * @brief HTTP状态码枚举
		 * 
		 * 定义了HTTP协议中的所有标准状态码，按照RFC 7231规范分为五类：
		 * - 1xx: 信息性状态码（Informational）
		 * - 2xx: 成功状态码（Successful）
		 * - 3xx: 重定向状态码（Redirection）
		 * - 4xx: 客户端错误状态码（Client Error）
		 * - 5xx: 服务器错误状态码（Server Error）
		 * 
		 * 状态码分类说明：
		 * ┌─────────────────────────────────────────┐
		 * │  1xx: 信息响应 - 请求已接收，继续处理   │
		 * │  2xx: 成功 - 请求已成功接收、理解和处理 │
		 * │  3xx: 重定向 - 需要进一步操作完成请求   │
		 * │  4xx: 客户端错误 - 请求包含语法错误     │
		 * │  5xx: 服务器错误 - 服务器处理请求失败   │
		 * └─────────────────────────────────────────┘
		 */
		enum HttpStatusCode
		{
			kUnknown = 0,                              // 未知状态码
			
			// 1xx: 信息性状态码
			k100Continue = 100,                        // 继续，客户端应继续其请求
			k101SwitchingProtocols = 101,              // 切换协议，服务器根据客户端请求切换协议
			
			// 2xx: 成功状态码
			k200OK = 200,                              // 请求成功
			k201Created = 201,                         // 已创建，请求成功并创建了新资源
			k202Accepted = 202,                        // 已接受，请求已接受但尚未处理
			k203NonAuthoritativeInformation = 203,     // 非授权信息，返回的信息可能来自另一来源
			k204NoContent = 204,                       // 无内容，请求成功但无返回内容
			k205ResetContent = 205,                    // 重置内容，要求客户端重置文档视图
			k206PartialContent = 206,                  // 部分内容，服务器成功处理了部分GET请求
			
			// 3xx: 重定向状态码
			k300MultipleChoices = 300,                 // 多种选择，请求的资源有多个可选位置
			k301MovedPermanently = 301,                // 永久移动，资源已被永久移动到新位置
			k302Found = 302,                           // 临时移动，资源临时从不同URI响应请求
			k303SeeOther = 303,                        // 查看其他位置，应使用GET方法获取资源
			k304NotModified = 304,                     // 未修改，资源未修改可使用缓存版本
			k305UseProxy = 305,                        // 使用代理，必须通过代理访问资源
			k307TemporaryRedirect = 307,               // 临时重定向，使用原有方法重定向
			k308PermanentRedirect = 308,               // 永久重定向，使用原有方法重定向
			
			// 4xx: 客户端错误状态码
			k400BadRequest = 400,                      // 错误请求，请求语法错误
			k401Unauthorized = 401,                    // 未授权，需要身份验证
			k402PaymentRequired = 402,                 // 需要付款，保留状态码
			k403Forbidden = 403,                       // 禁止访问，服务器拒绝请求
			k404NotFound = 404,                        // 未找到，请求的资源不存在
			k405MethodNotAllowed = 405,                // 方法不允许，请求方法不被允许
			k406NotAcceptable = 406,                   // 不可接受，无法根据请求的内容特性响应
			k407ProxyAuthenticationRequired = 407,     // 需要代理身份验证
			k408RequestTimeout = 408,                  // 请求超时，服务器等待请求超时
			k409Conflict = 409,                        // 冲突，请求与当前资源状态冲突
			k410Gone = 410,                            // 已删除，资源已被永久删除
			k411LengthRequired = 411,                  // 需要Content-Length头
			k412PreconditionFailed = 412,              // 前提条件失败
			k413RequestEntityTooLarge = 413,           // 请求实体过大
			k414RequestURITooLarge = 414,              // 请求URI过长
			k415UnsupportedMediaType = 415,            // 不支持的媒体类型
			k416RequestedRangeNotSatisfiable = 416,    // 请求范围无法满足
			k417ExpectationFailed = 417,               // 期望失败
			k418ImATeapot = 418,                       // 我是茶壶（愚人节彩蛋）
			k421MisdirectedRequest = 421,              // 请求被误导
			k425TooEarly = 425,                        // 太早，服务器不愿意处理可能被重放的请求
			k426UpgradeRequired = 426,                 // 需要升级协议
			k428PreconditionRequired = 428,            // 需要前提条件
			k429TooManyRequests = 429,                 // 请求过多，客户端发送请求过于频繁
			k431RequestHeaderFieldsTooLarge = 431,     // 请求头字段过大
			k451UnavailableForLegalReasons = 451,      // 因法律原因不可用
			
			// 5xx: 服务器错误状态码
			k500InternalServerError = 500,             // 服务器内部错误
			k501NotImplemented = 501,                  // 未实现，服务器不支持请求的功能
			k502BadGateway = 502,                      // 错误网关，网关或代理服务器收到无效响应
			k503ServiceUnavailable = 503,              // 服务不可用，服务器暂时过载或维护
			k504GatewayTimeout = 504,                  // 网关超时，网关或代理服务器超时
			k505HTTPVersionNotSupported = 505,         // HTTP版本不支持
			k510NotExtended = 510,                     // 未扩展，需要进一步扩展请求
		};

		/**
		 * @enum Version
		 * @brief HTTP协议版本枚举
		 * 
		 * 支持的HTTP版本：
		 * - HTTP/1.0: 早期版本，每个请求都需要建立新连接
		 * - HTTP/1.1: 支持持久连接、管道化、分块传输等特性
		 * 
		 * 注意：HTTP/2和HTTP/3使用二进制帧格式，需要单独实现
		 */
		enum class Version
		{
			kUnknown = 0,    // 未知版本
			kHttp10,         // HTTP/1.0版本
			kHttp11          // HTTP/1.1版本（默认）
		};

		/**
		 * @enum ContentType
		 * @brief HTTP内容类型枚举
		 * 
		 * 定义了常用的MIME类型，用于指示HTTP消息体的数据格式。
		 * Content-Type头字段告诉接收方如何解析消息体内容。
		 * 
		 * 常用类型说明：
		 * - JSON: 用于API数据交换
		 * - HTML: 用于网页内容
		 * - Plain: 用于纯文本
		 * - Form: 用于表单提交
		 * - M3U8: 用于HLS流媒体播放列表
		 * - TS: 用于HLS流媒体切片
		 * - FLV: 用于Flash视频流
		 */
		enum ContentType
		{
			kContentTypeNONE = 0,         // 无内容类型
			kContentTypeAppJson,          // application/json - JSON数据
			kContentTypeTextPlain,        // text/plain - 纯文本
			kContentTypeTextHTML,         // text/html - HTML文档
			kContentTypeAppXForm,         // application/x-www-form-urlencoded - 表单数据
			kContentTypeAppMpegUrl,       // application/vnd.apple.mpegurl - HLS播放列表(m3u8)
			kContentTypeVideoMP2T,        // video/MP2T - MPEG-TS视频流(ts切片)
			kContentTypeVideoXFlv,        // video/x-flv - FLV视频流
			kContentTypeTextXML,          // text/xml - XML文档
			kContentTypeAppXML,           // application/xml - XML数据
		};

		/**
		 * @enum HttpMethod
		 * @brief HTTP请求方法枚举
		 * 
		 * 定义了HTTP协议支持的请求方法，每种方法表示对资源的不同操作：
		 * 
		 * 方法语义：
		 * ┌──────────┬────────────────────────────────┐
		 * │  GET     │ 获取资源（幂等、安全）         │
		 * │  POST    │ 创建资源或提交数据             │
		 * │  PUT     │ 更新资源（幂等）               │
		 * │  DELETE  │ 删除资源（幂等）               │
		 * │  HEAD    │ 获取资源头信息（幂等、安全）   │
		 * │  OPTIONS │ 查询支持的方法（幂等、安全）   │
		 * │  PATCH   │ 部分更新资源                   │
		 * └──────────┴────────────────────────────────┘
		 * 
		 * 幂等性：多次执行相同操作结果一致
		 * 安全性：不会修改服务器资源状态
		 */
		enum HttpMethod
		{
			kGet = 0,      // GET方法 - 请求获取资源
			kPost,         // POST方法 - 提交数据或创建资源
			kHead,         // HEAD方法 - 获取资源头信息（不返回body）
			kPut,          // PUT方法 - 更新或创建资源
			kDelete,       // DELETE方法 - 删除资源
			kOptions,      // OPTIONS方法 - 查询服务器支持的方法
			kPatch,        // PATCH方法 - 部分更新资源
			kInvalid       // 无效方法
		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_