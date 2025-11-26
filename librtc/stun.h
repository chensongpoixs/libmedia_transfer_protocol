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
/***********************************************************************************************
created: 		2025-10-14

author:			chensong

purpose:		STUN协议实现（STUN Protocol Implementation）


STUN协议说明：
- STUN（Session Traversal Utilities for NAT）是NAT穿透工具协议
- STUN用于在WebRTC中进行ICE连接建立和NAT穿透
- STUN消息用于检测客户端的公网IP和端口
- WebRTC使用STUN消息进行连接性检查（Connectivity Check）

STUN消息格式（STUN Message Format）：

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |0 0|     STUN Message Type     |         Message Length        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                         Magic Cookie                          |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    |                     Transaction ID (96 bits)                  |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

STUN属性格式（STUN Attribute Format）：

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |         Attribute Type        |            Length             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                         Value (variable)                ....
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

ICE连接流程（ICE Connection Flow）：

    Client                                    Server
      |                                          |
      |  STUN Binding Request                   |
      | ------(1)-------------------------------->|
      |  (Username, Priority, ICE-CONTROLLING)  |
      |                                          |
      |                  STUN Binding Response  |
      |<-----------------------------(2)---------|
      |  (XOR-MAPPED-ADDRESS, MESSAGE-INTEGRITY)|
      |                                          |
      |  ICE连接建立                              |
      |<===========================(3)===========>|


输赢不重要，答案对你们有什么意义才重要。

光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
************************************************************************************************/


#ifndef _C_STUN_H_
#define _C_STUN_H_

#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include <string>
#include "rtc_base/buffer.h"

namespace libmedia_transfer_protocol {
	namespace librtc {


		const uint32_t kStunMagicCookie = 0x2112A442;  ///< STUN魔术Cookie，固定值0x2112A442，用于识别STUN消息
		
		/**
		*  @brief STUN消息类型枚举（STUN Message Type）
		*  
		*  该枚举定义了STUN协议支持的消息类型。STUN消息分为请求、响应和错误响应三种。
		*  
		*  类型说明：
		*  - kStunMsgUnknow: 未知类型
		*  - kStunMsgBindingRequest: 绑定请求，客户端发送以获取映射地址
		*  - kStunMsgBindingResponse: 绑定响应，服务器返回映射地址
		*  - kStunMsgBindingErrorResponse: 绑定错误响应
		*  - kStunMsgSharedSecretRequest: 共享密钥请求
		*  - kStunMsgSharedSecretResponse: 共享密钥响应
		*  - kStunMsgSharedSecretErrorResponse: 共享密钥错误响应
		*  
		*  @note 最常用的是BindingRequest和BindingResponse
		*  @note 消息类型编码在STUN头部的前2字节
		*/
		enum StunMessageType
		{
			kStunMsgUnknow = 0x0000,                     ///< 未知消息类型
			kStunMsgBindingRequest = 0x0001,             ///< 绑定请求
			kStunMsgBindingResponse = 0x0101,            ///< 绑定响应
			kStunMsgBindingErrorResponse = 0x0111,       ///< 绑定错误响应
			kStunMsgSharedSecretRequest = 0x0002,        ///< 共享密钥请求
			kStunMsgSharedSecretResponse = 0x0102,       ///< 共享密钥响应
			kStunMsgSharedSecretErrorResponse = 0x0112,  ///< 共享密钥错误响应
		};

		/**
		*  @brief STUN属性类型枚举（STUN Attribute Type）
		*  
		*  该枚举定义了STUN协议支持的属性类型。STUN消息可以包含多个属性，
		*  每个属性有类型、长度和值三部分。
		*  
		*  常用属性说明：
		*  - MAPPED-ADDRESS: 客户端的映射地址（公网IP和端口）
		*  - XOR-MAPPED-ADDRESS: 异或编码的映射地址（推荐使用）
		*  - USERNAME: ICE用户名，格式为"remote_ufrag:local_ufrag"
		*  - MESSAGE-INTEGRITY: HMAC-SHA1消息完整性校验
		*  - FINGERPRINT: CRC-32指纹校验
		*  - PRIORITY: ICE候选优先级
		*  - USE-CANDIDATE: 标记选中的候选
		*  - ICE-CONTROLLING/ICE-CONTROLLED: ICE角色标识
		*  
		*  @note 参考RFC 3489和RFC 5389
		*/
		enum StunAttributeType
		{
			// https://tools.ietf.org/html/rfc3489#section-11.2
			kStunAttrMappedAddress = 0x0001,        ///< 映射地址（旧版）
			kStunAttrResponseAddress = 0x0002,      ///< 响应地址
			kStunAttrChangeRequest = 0x0003,        ///< 变更请求
			kStunAttrSourceAddress = 0x0004,        ///< 源地址
			kStunAttrChangedAddress = 0x0005,       ///< 变更后的地址
			kStunAttrUsername = 0x0006,             ///< 用户名（ICE用户名）
			kStunAttrPassword = 0x0007,             ///< 密码
			kStunAttrMessageIntegrity = 0x0008,     ///< 消息完整性（HMAC-SHA1）
			kStunAttrErrorCode = 0x0009,            ///< 错误码
			kStunAttrUnknownAttributes = 0x000A,    ///< 未知属性
			kStunAttrReflectedFrom = 0x000B,        ///< 反射来源
			// https://tools.ietf.org/html/rfc5389#section-18.2
			kStunAttrRealm = 0x0014,                ///< 域
			kStunAttrNonce = 0x0015,                ///< 随机数
			kStunAttrXorMappedAddress = 0x0020,     ///< XOR映射地址（推荐）
			kStunAttrSoftware = 0x8022,             ///< 软件标识
			kStunAttrAlternateServer = 0x8023,      ///< 备用服务器
			kStunAttrFingerprint = 0x8028,          ///< 指纹（CRC-32）
			kStunAttrPriority = 0x0024,             ///< 优先级（ICE）
			kStunAttrUseCandidate = 0x0025,         ///< 使用候选（ICE）
			kStunAttrIceControlled = 0x8029,        ///< ICE受控方
			kStunAttrIceControlling = 0x802A,       ///< ICE控制方
		};
		/**
		*  @author chensong
		*  @date 2025-10-14
		*  @brief STUN协议处理类（STUN Protocol Handler）
		*  
		*  Stun类用于处理STUN协议消息的解析和生成。它支持STUN Binding Request/Response
		*  消息的编码和解码，用于WebRTC的ICE连接建立。
		*  
		*  主要功能：
		*  1. 解析STUN消息：解析Binding Request，提取用户名、优先级等属性
		*  2. 生成STUN消息：生成Binding Response，包含XOR-MAPPED-ADDRESS
		*  3. 消息完整性校验：计算和验证MESSAGE-INTEGRITY（HMAC-SHA1）
		*  4. 属性管理：管理各种STUN属性（用户名、密码、映射地址等）
		*  
		*  工作流程：
		*  1. 接收STUN Binding Request
		*  2. 调用Decode()解析请求
		*  3. 提取用户名并验证
		*  4. 设置映射地址和端口
		*  5. 调用Encode()生成Binding Response
		*  6. 发送响应给客户端
		*  
		*  @note STUN用于ICE连接建立和NAT穿透
		*  @note 支持MESSAGE-INTEGRITY和FINGERPRINT校验
		*  @note XOR-MAPPED-ADDRESS使用Magic Cookie进行XOR编码
		*  
		*  使用示例：
		*  @code
		*  Stun stun;
		*  
		*  // 解析STUN请求
		*  if (stun.Decode(data, size)) {
		*      std::string ufrag = stun.LocalUFrag();
		*      
		*      // 生成STUN响应
		*      stun.SetPassword("ice_password");
		*      stun.SetMappedAddr(client_ip);
		*      stun.SetMappedPort(client_port);
		*      stun.SetMessageType(kStunMsgBindingResponse);
		*      
		*      rtc::Buffer response = stun.Encode();
		*      // 发送响应
		*  }
		*  @endcode
		*/
		class Stun
		{
		public:
			/** 默认构造函数 */
			Stun() = default;
			
			/** 默认析构函数 */
			~Stun() = default;
			
		public:
			/**
			*  @brief 解码STUN消息（Decode STUN Message）
			*  
			*  该方法用于解析STUN消息，提取消息类型、事务ID、属性等。
			*  
			*  解析流程：
			*  1. 验证STUN头部（Magic Cookie）
			*  2. 提取消息类型和长度
			*  3. 提取事务ID
			*  4. 解析属性列表（USERNAME、PRIORITY等）
			*  5. 验证MESSAGE-INTEGRITY（如果有）
			*  
			*  @param data STUN消息数据指针
			*  @param size STUN消息大小（字节）
			*  @return 如果解析成功返回true，否则返回false
			*  @note 解析失败可能是格式错误或校验失败
			*/
			bool Decode(const uint8_t* data, uint32_t size);
			
			/**
			*  @brief 编码STUN消息（Encode STUN Message）
			*  
			*  该方法用于生成STUN响应消息，包含XOR-MAPPED-ADDRESS等属性。
			*  
			*  生成流程：
			*  1. 构建STUN头部
			*  2. 添加XOR-MAPPED-ADDRESS属性
			*  3. 添加USERNAME属性（回显请求中的用户名）
			*  4. 计算MESSAGE-INTEGRITY（HMAC-SHA1）
			*  5. 添加FINGERPRINT（CRC-32）
			*  
			*  @return 返回编码后的STUN消息缓冲区
			*  @note 必须先设置密码、映射地址和端口
			*/
			rtc::Buffer Encode();
			
			/**
			*  @brief 获取本地用户名片段（Get Local UFrag）
			*  
			*  该方法用于从USERNAME属性中提取本地用户名片段。
			*  USERNAME格式为"remote_ufrag:local_ufrag"。
			*  
			*  @return 返回本地用户名片段
			*  @note 用于ICE连接验证
			*/
			std::string LocalUFrag();
			
			/**
			*  @brief 设置密码（Set Password）
			*  
			*  该方法用于设置ICE密码，用于计算MESSAGE-INTEGRITY。
			*  
			*  @param pwd ICE密码
			*  @note 密码用于HMAC-SHA1计算
			*/
			void SetPassword(const std::string &pwd);
			
			/**
			*  @brief 设置映射地址（Set Mapped Address）
			*  
			*  该方法用于设置客户端的映射地址（公网IP）。
			*  
			*  @param addr 映射地址（网络字节序）
			*  @note 地址将被编码到XOR-MAPPED-ADDRESS属性
			*/
			void SetMappedAddr(uint32_t addr);
			
			/**
			*  @brief 设置映射端口（Set Mapped Port）
			*  
			*  该方法用于设置客户端的映射端口（公网端口）。
			*  
			*  @param port 映射端口
			*  @note 端口将被编码到XOR-MAPPED-ADDRESS属性
			*/
			void SetMappedPort(uint16_t port);
			
			/**
			*  @brief 设置消息类型（Set Message Type）
			*  
			*  该方法用于设置STUN消息类型。
			*  
			*  @param type STUN消息类型
			*  @note 通常设置为kStunMsgBindingResponse
			*/
			void SetMessageType(StunMessageType type);
			
			/**
			*  @brief 计算HMAC-SHA1（Calculate HMAC）
			*  
			*  该方法用于计算MESSAGE-INTEGRITY的HMAC-SHA1值。
			*  
			*  @param buf 输出缓冲区，存储HMAC结果（20字节）
			*  @param data 输入数据指针
			*  @param bytes 输入数据长度
			*  @return 返回HMAC长度（20字节）
			*  @note 使用SHA1算法计算HMAC
			*/
			size_t CalcHmac(char *buf, const char *data, size_t bytes);
			
		private:
			StunMessageType type_{ kStunMsgUnknow };   ///< STUN消息类型
			int32_t stun_length_{ 0 };                 ///< STUN消息长度（不包括头部20字节）
			std::string transcation_id_;               ///< 事务ID（96位，12字节）
			std::string user_name_{""};                ///< ICE用户名（格式：remote_ufrag:local_ufrag）
			std::string password_{ "" };               ///< ICE密码，用于HMAC计算
			uint32_t mapped_addr_{ 0 };                ///< 映射地址（客户端公网IP）
			uint16_t mapped_port_{ 0 };                ///< 映射端口（客户端公网端口）
		};
	}
}


#endif//