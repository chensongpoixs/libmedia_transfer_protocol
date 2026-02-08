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

【HTTP工具类文件】

本文件提供HTTP协议相关的工具函数，包括：
1. HTTP方法解析（ParseMethod）
2. HTTP状态码解析和转换（ParseStatusCode、ParseStatusMessage）
3. Content-Type解析和转换（ParseContentType、ContentTypeToString）
4. URL编解码（UrlEncode、UrlDecode）
5. 字符串处理（Trim、ltrim、rtrim）

这些工具函数是HTTP协议处理的基础设施，被HTTP解析器、请求构造器等模块广泛使用。

【作者的思考】
工具类的设计体现了"单一职责"和"高内聚低耦合"的原则。每个函数都专注于一个特定的转换或处理任务，
函数之间相互独立，可以自由组合使用。这种设计使得代码易于测试、维护和扩展。

【URL编解码说明】
URL编码（也称为百分号编码）是将特殊字符转换为%XX格式的过程：
- 空格 → %20 或 +
- 中文字符 → UTF-8字节序列的百分号编码
- 特殊字符（如&、=、?）→ %26、%3D、%3F

URL解码是编码的逆过程，将%XX格式还原为原始字符。

 ******************************************************************************/

#ifndef _C_LIBHTTP_UTILS_H_
#define _C_LIBHTTP_UTILS_H_

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
#include "libmedia_transfer_protocol/libgb28181/gb28181_session.h"
#include "libmedia_transfer_protocol/libhttp/http_type.h"
#include <string>
#include <unordered_map>
#include <iostream>
//#include <ctype.h>
#include <cstdint>
#include <vector>
#include <sstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <algorithm>
namespace  libmedia_transfer_protocol {
	namespace libhttp
	{
		/**
		 * @class HttpUtils
		 * @brief HTTP协议工具类
		 * 
		 * 提供HTTP协议相关的静态工具函数，包括：
		 * - 协议元素解析（方法、状态码、内容类型）
		 * - 字符串编解码（URL编解码）
		 * - 字符串处理（去除空白字符）
		 * 
		 * 【使用场景】
		 * 1. HTTP请求解析：将字符串形式的方法名转换为枚举
		 * 2. HTTP响应构造：将状态码转换为状态消息
		 * 3. URL处理：对URL参数进行编解码
		 * 4. 文件服务：根据文件扩展名确定Content-Type
		 * 
		 * 【使用示例】
		 * @code
		 * // 解析HTTP方法
		 * HttpMethod method = HttpUtils::ParseMethod("GET");
		 * 
		 * // 获取状态码消息
		 * std::string msg = HttpUtils::ParseStatusMessage(404); // "Not Found"
		 * 
		 * // URL编解码
		 * std::string encoded = HttpUtils::UrlEncode("hello world"); // "hello+world"
		 * std::string decoded = HttpUtils::UrlDecode("hello+world"); // "hello world"
		 * 
		 * // 字符串去空白
		 * std::string str = "  hello  ";
		 * HttpUtils::Trim(str); // "hello"
		 * @endcode
		 * 
		 * 【注意事项】
		 * 1. 所有函数都是静态函数，无需实例化即可使用
		 * 2. URL编解码支持UTF-8字符
		 * 3. Trim函数会修改原字符串
		 */
		class HttpUtils
		{
		public:
			/**
			 * @brief 解析HTTP方法字符串
			 * @param method HTTP方法字符串（如"GET"、"POST"）
			 * @return HttpMethod枚举值，无效方法返回kInvalid
			 * 
			 * 支持的方法：GET、POST、PUT、DELETE、HEAD、OPTIONS、PATCH
			 */
			static HttpMethod ParseMethod(const std::string &method);
			
			/**
			 * @brief 将整数状态码转换为HttpStatusCode枚举
			 * @param code HTTP状态码（如200、404）
			 * @return HttpStatusCode枚举值，未知状态码返回kUnknown
			 */
			static HttpStatusCode ParseStatusCode(int32_t code);
			
			/**
			 * @brief 获取状态码对应的状态消息
			 * @param code HTTP状态码
			 * @return 状态消息字符串（如"OK"、"Not Found"）
			 * 
			 * 示例：
			 * - 200 → "OK"
			 * - 404 → "Not Found"
			 * - 500 → "Internal Server Error"
			 */
			static std::string ParseStatusMessage(int32_t code);
			
			/**
			 * @brief 解析Content-Type字符串
			 * @param contentType Content-Type字符串（如"application/json"）
			 * @return ContentType枚举值，未知类型返回kContentTypeNONE
			 */
			static ContentType ParseContentType(const std::string &contentType);
			
			/**
			 * @brief 将ContentType枚举转换为完整的Content-Type头字符串
			 * @param contenttype ContentType枚举值
			 * @return 完整的Content-Type头字符串（包含charset）
			 * 
			 * 示例：kContentTypeAppJson → "Content-Type: application/json; charset=utf-8\r\n"
			 */
			static const std::string &ContentTypeToString(ContentType contenttype);
			
			/**
			 * @brief 将状态码转换为状态消息字符串
			 * @param code HTTP状态码
			 * @return 状态消息字符串
			 * 
			 * 与ParseStatusMessage功能相同，返回const引用以提高性能
			 */
			static const std::string &StatusCodeToString(int code);
			
			/**
			 * @brief 根据文件名获取Content-Type
			 * @param fileName 文件名（包含扩展名）
			 * @return ContentType枚举值
			 * 
			 * 支持的扩展名：
			 * - .html → kContentTypeTextHTML
			 * - .ts → kContentTypeVideoMP2T
			 * - .flv → kContentTypeVideoXFlv
			 * - 其他 → kContentTypeTextPlain
			 */
			static ContentType GetContentType(const std::string &fileName);
			
			/**
			 * @brief 将字符转换为十六进制字符串
			 * @param c 要转换的字符
			 * @return 两位十六进制字符串（大写）
			 * 
			 * 示例：' ' → "20"，'A' → "41"
			 */
			static std::string CharToHex(char c);
			
			/**
			 * @brief 检查URL是否需要解码
			 * @param url URL字符串
			 * @return true表示包含编码字符（%或+），需要解码
			 */
			static bool NeedUrlDecoding(const std::string &url);
			
			/**
			 * @brief URL解码
			 * @param url 编码后的URL字符串
			 * @return 解码后的字符串
			 * 
			 * 解码规则：
			 * - %XX → 对应的字符（XX为十六进制）
			 * - + → 空格
			 * - 其他字符保持不变
			 * 
			 * 示例：
			 * - "hello+world" → "hello world"
			 * - "hello%20world" → "hello world"
			 * - "%E4%B8%AD%E6%96%87" → "中文"
			 */
			static std::string UrlDecode(const std::string &url);
			
			/**
			 * @brief URL编码
			 * @param src 原始字符串
			 * @return 编码后的URL字符串
			 * 
			 * 编码规则：
			 * - 空格 → +
			 * - 字母数字和部分符号（-_.!~*'()&=/\?）保持不变
			 * - 其他字符 → %XX（XX为十六进制）
			 * 
			 * 示例：
			 * - "hello world" → "hello+world"
			 * - "中文" → "%E4%B8%AD%E6%96%87"
			 */
			static std::string UrlEncode(const std::string &src);

			/**
			 * @brief 去除字符串左侧空白字符
			 * @param str 要处理的字符串（会被修改）
			 * @return 处理后的字符串引用
			 * 
			 * 空白字符包括：空格、制表符、换行符等
			 */
			static std::string& ltrim(std::string &str)
			{
				auto p = std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>( ::isspace)));
				str.erase(str.begin(), p);
				return str;
			}

			/**
			 * @brief 去除字符串右侧空白字符
			 * @param str 要处理的字符串（会被修改）
			 * @return 处理后的字符串引用
			 */
			static std::string& rtrim(std::string &str)
			{
				auto p = std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>( ::isspace)));
				str.erase(p.base(), str.end());
				return str;
			}

			/**
			 * @brief 去除字符串两侧空白字符
			 * @param str 要处理的字符串（会被修改）
			 * @return 处理后的字符串引用
			 * 
			 * 示例："  hello  " → "hello"
			 */
			static std::string& Trim(std::string &str)
			{
				ltrim(rtrim(str));
				return str;
			}
		};
	}

}


#endif // _C_LIBGB28181_SERVER_H_