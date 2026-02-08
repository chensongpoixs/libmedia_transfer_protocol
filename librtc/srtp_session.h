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
created: 		2025-10-16

author:			chensong

purpose:		SRTP会话管理（SRTP Session Management）


SRTP协议说明：
- SRTP（Secure Real-time Transport Protocol）是安全实时传输协议
- SRTP用于加密RTP和RTCP数据包，保证WebRTC媒体流的安全性
- SRTP使用AES加密和HMAC-SHA1或GCM认证
- SRTP密钥通过DTLS协商导出

SRTP数据包格式（SRTP Packet Format）：

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+<+
    |V=2|P|X|  CC   |M|     PT      |       sequence number         | |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
    |                           timestamp                           | |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
    |           synchronization source (SSRC) identifier            | |
    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ |
    |            contributing source (CSRC) identifiers             | |
    |                               ....                            | |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |
    |                   RTP extension (OPTIONAL)                    | |
  +>+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ | |
  | |                          payload  ...                         | | |
  | |                               +-------------------------------+ | |
  | |                               | RTP padding   | RTP pad count | | |
  +>+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+<+ |
  | ~                     SRTP MKI (OPTIONAL)                       ~ | |
  | +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ | |
  | :                 authentication tag (RECOMMENDED)              : | |
  | +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ | |
  |                                                                   |
  +- Encrypted Portion                      Authenticated Portion ---+

加密套件说明：
- AES_CM_128_HMAC_SHA1_80: AES-128 Counter Mode + HMAC-SHA1 (80位认证标签)
- AES_CM_128_HMAC_SHA1_32: AES-128 Counter Mode + HMAC-SHA1 (32位认证标签)
- AEAD_AES_128_GCM: AES-128 GCM（推荐）
- AEAD_AES_256_GCM: AES-256 GCM（最安全）

SRTP密钥生成流程（SRTP Key Derivation）：

    DTLS Handshake
         |
         v
    Extract Master Key & Salt
         |
         v
    +-----------------+
    | Master Key (16) |  +  | Master Salt (14) |  = 30 bytes
    +-----------------+     +-----------------+
         |
         v
    Key Derivation Function (PRF)
         |
         +----> SRTP Encryption Key
         +----> SRTP Authentication Key
         +----> SRTP Salt


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


#ifndef _C_LIBRTC_SRTP_SESSION__H_
#define _C_LIBRTC_SRTP_SESSION__H_


#include "srtp.h"
//#include "usrsctp.h"

#include "libmedia_transfer_protocol/librtc/dtls_certs.h"
struct srtp_event_data_t;
struct srtp_ctx_t_;

namespace libmedia_transfer_protocol {

	namespace libsrtp
	{
		////////////////////////////////
		/**
		*  @brief SRTP加密套件枚举（SRTP Crypto Suite）
		*  
		*  该枚举定义了SRTP支持的加密套件。不同的套件提供不同级别的安全性和性能。
		*  
		*  套件说明：
		*  - NONE: 无加密（不推荐）
		*  - AES_CM_128_HMAC_SHA1_80: AES-128 Counter Mode + HMAC-SHA1-80
		*    - 密钥长度：16字节
		*    - 盐长度：14字节
		*    - 认证标签：10字节（80位）
		*  - AES_CM_128_HMAC_SHA1_32: AES-128 Counter Mode + HMAC-SHA1-32
		*    - 密钥长度：16字节
		*    - 盐长度：14字节
		*    - 认证标签：4字节（32位）
		*  - AEAD_AES_128_GCM: AES-128 GCM（推荐）
		*    - 密钥长度：16字节
		*    - 盐长度：12字节
		*    - 认证标签：16字节
		*  - AEAD_AES_256_GCM: AES-256 GCM（最安全）
		*    - 密钥长度：32字节
		*    - 盐长度：12字节
		*    - 认证标签：16字节
		*  
		*  @note GCM模式性能更好，推荐使用
		*  @note 参考RFC 3711和RFC 7714
		*/
		enum  class  CryptoSuite
		{
			NONE = 0,                      ///< 无加密
			AES_CM_128_HMAC_SHA1_80 = 1,   ///< AES-128 + HMAC-SHA1-80（常用）
			AES_CM_128_HMAC_SHA1_32,       ///< AES-128 + HMAC-SHA1-32
			AEAD_AES_256_GCM,              ///< AES-256 GCM（最安全）
			AEAD_AES_128_GCM               ///< AES-128 GCM（推荐）
		};
		
		/**
		*  @brief SRTP流向类型（SRTP Direction Type）
		*  
		*  该枚举定义了SRTP会话的流向。每个方向需要独立的SRTP会话和密钥。
		*  
		*  类型说明：
		*  - INBOUND: 入站流（接收），用于解密接收到的SRTP数据包
		*  - OUTBOUND: 出站流（发送），用于加密发送的RTP数据包
		*  
		*  @note 发送和接收使用不同的密钥，通过DTLS协商
		*/
		enum   SrtpType
		{
			INBOUND = 1,    ///< 入站流（接收，解密）
			OUTBOUND        ///< 出站流（发送，加密）
		};
		// AES-HMAC: http://tools.ietf.org/html/rfc3711
		static const  size_t kSrtpMasterKeyLength{ 16 };    ///< SRTP主密钥长度（AES-128）：16字节
		static const  size_t kSrtpMasterSaltLength{ 14 };   ///< SRTP主盐长度：14字节
		static const  size_t kSrtpMasterLength{ kSrtpMasterKeyLength + kSrtpMasterSaltLength };  ///< SRTP主密钥材料总长度：30字节
		// AES-GCM: http://tools.ietf.org/html/rfc7714
		static const size_t kSrtpAesGcm256MasterKeyLength{ 32 };  ///< SRTP AES-256 GCM主密钥长度：32字节
		static const size_t kSrtpAesGcm256MasterSaltLength{ 12 }; ///< SRTP AES-256 GCM主盐长度：12字节
		static const size_t kSrtpAesGcm256MasterLength{ kSrtpAesGcm256MasterKeyLength + kSrtpAesGcm256MasterSaltLength };  ///< SRTP AES-256 GCM主密钥材料总长度：44字节
		static const size_t kSrtpAesGcm128MasterKeyLength{ 16 };  ///< SRTP AES-128 GCM主密钥长度：16字节
		static const size_t kSrtpAesGcm128MasterSaltLength{ 12 }; ///< SRTP AES-128 GCM主盐长度：12字节
		static const size_t kSrtpAesGcm128MasterLength{ kSrtpAesGcm128MasterKeyLength + kSrtpAesGcm128MasterSaltLength };  ///< SRTP AES-128 GCM主密钥材料总长度：28字节
		// clang-format on
		
		/**
		*  @brief SRTP加密套件映射条目（SRTP Crypto Suite Map Entry）
		*  
		*  该结构用于将加密套件枚举映射到字符串名称。
		*  用于SDP协商和配置。
		*/
		struct SrtpCryptoSuiteMapEntry
		{
			CryptoSuite   crypto_suite;  ///< 加密套件枚举值
			const char *  name;          ///< 加密套件字符串名称
		};
		
		extern   std::vector< SrtpCryptoSuiteMapEntry>   kSrtpCryptoSuites;  ///< SRTP加密套件映射表
		//const int32_t kSrtpMaxBufferSize = 65535;
		static const size_t kEncryptBufferSize{ 65536 };  ///< 加密缓冲区大小：64KB
		/**
		*  @author chensong
		*  @date 2025-10-16
		*  @brief SRTP会话管理类（SRTP Session Manager）
		*  
		*  SrtpSession类用于管理SRTP会话，提供RTP/RTCP数据包的加密和解密功能。
		*  它封装了libsrtp库，为WebRTC提供媒体流安全传输。
		*  
		*  主要功能：
		*  1. SRTP会话初始化：根据加密套件和密钥创建SRTP会话
		*  2. RTP加密：加密发送的RTP数据包
		*  3. SRTP解密：解密接收的SRTP数据包
		*  4. RTCP加密：加密发送的RTCP数据包
		*  5. SRTCP解密：解密接收的SRTCP数据包
		*  6. 流管理：管理多个SSRC的加密/解密状态
		*  
		*  加密流程：
		*  1. DTLS握手完成，导出SRTP密钥材料
		*  2. 创建入站和出站SrtpSession
		*  3. 发送RTP时，调用EncryptRtp()加密
		*  4. 接收SRTP时，调用DecryptSrtp()解密
		*  
		*  密钥导出流程（DTLS-SRTP Key Derivation）：
		*  1. DTLS握手完成
		*  2. 调用SSL_export_keying_material导出密钥材料
		*  3. 分割密钥材料：client_write_key + server_write_key + client_salt + server_salt
		*  4. 根据DTLS角色选择发送和接收密钥
		*  5. 创建入站和出站SrtpSession
		*  
		*  @note 每个方向（入站/出站）需要独立的SrtpSession
		*  @note 发送和接收使用不同的密钥
		*  @note 使用libsrtp库进行实际加密/解密操作
		*  
		*  使用示例：
		*  @code
		*  // 导出SRTP密钥
		*  uint8_t key_material[60];
		*  SSL_export_keying_material(ssl, key_material, 60, "EXTRACTOR-dtls_srtp", 19, nullptr, 0, 0);
		*  
		*  // 创建出站会话（发送）
		*  SrtpSession* send_session = new SrtpSession(
		*      OUTBOUND,
		*      CryptoSuite::AEAD_AES_128_GCM,
		*      client_key,
		*      28  // AES-128-GCM key + salt
		*  );
		*  
		*  // 加密RTP数据包
		*  const uint8_t* rtp_data = ...;
		*  size_t rtp_len = ...;
		*  if (send_session->EncryptRtp(&rtp_data, &rtp_len)) {
		*      // 发送加密后的数据
		*  }
		*  
		*  // 创建入站会话（接收）
		*  SrtpSession* recv_session = new SrtpSession(
		*      INBOUND,
		*      CryptoSuite::AEAD_AES_128_GCM,
		*      server_key,
		*      28
		*  );
		*  
		*  // 解密SRTP数据包
		*  uint8_t* srtp_data = ...;
		*  size_t srtp_len = ...;
		*  if (recv_session->DecryptSrtp(srtp_data, &srtp_len)) {
		*      // 处理解密后的RTP数据
		*  }
		*  @endcode
		*/
		class SrtpSession
		{
		public:
			/**
			*  @brief SRTP会话构造函数
			*  
			*  该构造函数用于创建SRTP会话，初始化libsrtp上下文。
			*  
			*  初始化流程：
			*  1. 根据加密套件选择SRTP策略
			*  2. 设置密钥和盐
			*  3. 创建libsrtp会话上下文
			*  4. 注册事件回调
			*  
			*  @param type SRTP流向类型（INBOUND或OUTBOUND）
			*  @param cryptoSuite 加密套件（推荐AEAD_AES_128_GCM）
			*  @param key 密钥材料指针（包含key和salt）
			*  @param keyLen 密钥材料长度（根据加密套件而定）
			*  @note 密钥长度必须与加密套件匹配
			*  @note 构造失败会抛出异常
			*/
			SrtpSession(SrtpType type, CryptoSuite cryptoSuite, uint8_t* key, size_t keyLen)  ;
			
			/**
			*  @brief SRTP会话析构函数
			*  
			*  该析构函数用于销毁SRTP会话，释放libsrtp资源。
			*/
			~SrtpSession()  ;
			
		private:
			/**
			*  @brief SRTP事件回调（SRTP Event Callback）
			*  
			*  该静态方法用于处理libsrtp库的事件通知（如密钥过期）。
			*  
			*  @param data SRTP事件数据
			*/
			static void OnSrtpEvent(srtp_event_data_t* data);
			
		public:
			/**
			*  @brief 初始化SRTP库（Initialize SRTP Library）
			*  
			*  该静态方法用于初始化libsrtp库。必须在使用SRTP功能前调用一次。
			*  
			*  @return 如果初始化成功返回true，否则返回false
			*  @note 应用程序启动时调用一次
			*/
			static bool InitSrtpLibrary();
			
			/**
			*  @brief 销毁SRTP库（Destroy SRTP Library）
			*  
			*  该静态方法用于销毁libsrtp库，释放全局资源。
			*  
			*  @note 应用程序退出时调用一次
			*/
			static void DestroySrtpLibrary();
			
			/**
			*  @brief 获取错误字符串（Get Error String）
			*  
			*  该静态方法用于将libsrtp错误码转换为可读字符串。
			*  
			*  @param code libsrtp错误码
			*  @return 返回错误描述字符串
			*/
			static const char* GetErrorString(srtp_err_status_t code);
			
		public:
			/**
			*  @brief 加密RTP数据包（Encrypt RTP Packet）
			*  
			*  该方法用于加密RTP数据包，生成SRTP数据包。
			*  
			*  加密流程：
			*  1. 将RTP数据复制到内部缓冲区
			*  2. 调用libsrtp进行加密
			*  3. 添加认证标签
			*  4. 更新数据指针和长度
			*  
			*  @param data RTP数据指针的指针（输入RTP，输出SRTP）
			*  @param len RTP长度的指针（输入RTP长度，输出SRTP长度）
			*  @return 如果加密成功返回true，否则返回false
			*  @note 加密后的数据存储在内部缓冲区
			*  @note 调用者必须在下次调用前使用返回的数据
			*/
			bool EncryptRtp(const uint8_t** data, size_t* len);
			
			/**
			*  @brief 解密SRTP数据包（Decrypt SRTP Packet）
			*  
			*  该方法用于解密SRTP数据包，恢复RTP数据包。
			*  
			*  解密流程：
			*  1. 验证认证标签
			*  2. 调用libsrtp进行解密
			*  3. 移除认证标签
			*  4. 更新数据长度
			*  
			*  @param data SRTP数据指针（就地解密）
			*  @param len SRTP长度的指针（输入SRTP长度，输出RTP长度）
			*  @return 如果解密成功返回true，否则返回false
			*  @note 解密操作是就地进行的
			*  @note 解密失败可能是密钥错误或数据被篡改
			*/
			bool DecryptSrtp(uint8_t* data, size_t* len);
			
			/**
			*  @brief 加密RTCP数据包（Encrypt RTCP Packet）
			*  
			*  该方法用于加密RTCP数据包，生成SRTCP数据包。
			*  
			*  @param data RTCP数据指针的指针（输入RTCP，输出SRTCP）
			*  @param len RTCP长度的指针（输入RTCP长度，输出SRTCP长度）
			*  @return 如果加密成功返回true，否则返回false
			*  @note RTCP加密类似于RTP，但使用独立的密钥派生
			*/
			bool EncryptRtcp(const uint8_t** data, size_t* len);
			
			/**
			*  @brief 解密SRTCP数据包（Decrypt SRTCP Packet）
			*  
			*  该方法用于解密SRTCP数据包，恢复RTCP数据包。
			*  
			*  @param data SRTCP数据指针（就地解密）
			*  @param len SRTCP长度的指针（输入SRTCP长度，输出RTCP长度）
			*  @return 如果解密成功返回true，否则返回false
			*/
			bool DecryptSrtcp(uint8_t* data, size_t* len);
			
			/**
			*  @brief 移除流（Remove Stream）
			*  
			*  该方法用于从SRTP会话中移除指定SSRC的流。
			*  
			*  @param ssrc 要移除的流的SSRC
			*  @note 移除流会清除该SSRC的加密/解密状态
			*/
			void RemoveStream(uint32_t ssrc);
			
		public:
			// Allocated by this.
			srtp_ctx_t_* session_{ nullptr };          ///< libsrtp会话上下文
			uint8_t EncryptBuffer[kEncryptBufferSize]; ///< 加密缓冲区（64KB），用于存储加密后的数据
			
		};
	}
}

#endif // _C_LIBRTC_SRTP_SESSION__H_