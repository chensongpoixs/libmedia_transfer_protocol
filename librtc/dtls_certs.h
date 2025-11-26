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

purpose:		DTLS证书管理（DTLS Certificate Management）


DTLS证书管理说明：
- DTLS（Datagram Transport Layer Security）是TLS协议的数据报版本
- DTLS用于在WebRTC中进行密钥交换和建立SRTP加密会话
- 每个DTLS连接需要一个证书和私钥对
- 证书指纹（Fingerprint）用于在SDP中验证证书的真实性
- 支持多种指纹算法：SHA-1、SHA-224、SHA-256、SHA-384、SHA-512

DTLS握手流程（DTLS Handshake Flow）：

  Client                                    Server
    |                                          |
    |  ClientHello                            |
    | ------(1)-------------------------------->|
    |                                          |
    |                      ServerHello, Certificate, ServerHelloDone |
    |<-----------------------------(2)---------|
    |                                          |
    |  Certificate, ClientKeyExchange, CertificateVerify |
    | ------(3)-------------------------------->|
    |                                          |
    |  [ChangeCipherSpec]                     |
    |  Finished                               |
    | ------(4)-------------------------------->|
    |                                          |
    |                  [ChangeCipherSpec] Finished |
    |<-----------------------------(5)---------|
    |                                          |
    |  Application Data (SRTP)  <----------->  |
    |<===========================(6)===========>|

证书指纹格式（Certificate Fingerprint Format）：

  算法:指纹值
  例如：sha-256 AB:CD:EF:12:34:56:78:90:AB:CD:EF:12:34:56:78:90:AB:CD:EF:12:34:56:78:90:AB:CD:EF:12:34:56:78:90

SDP中的DTLS指纹示例（DTLS Fingerprint in SDP）：

  a=fingerprint:sha-256 AB:CD:EF:12:34:56:78:90:AB:CD:EF:12:34:56:78:90:AB:CD:EF:12:34:56:78:90:AB:CD:EF:12:34:56:78:90
  a=setup:actpass


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


#ifndef _C_DTLS_CERTS_H_
#define _C_DTLS_CERTS_H_

#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include <map>
#include <vector>
#include <string>
#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>
#include "libmedia_transfer_protocol/librtc/dtls_certs.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "rtc_base/task_queue.h"
#include "libmedia_transfer_protocol/librtc/srtp_session.h"
#include <cstddef>

#include "absl/types/optional.h"
#include <cstdint>
#include <string>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <random>

#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/buffer.h"
namespace libmedia_transfer_protocol {
	namespace libssl {
		
	//	static const int32_t  
		
		/**
		*  @brief 证书指纹算法枚举（Fingerprint Algorithm Enum）
		*  
		*  该枚举定义了DTLS证书指纹支持的哈希算法。指纹算法用于计算证书的哈希值，
		*  并在SDP中进行证书验证。
		*  
		*  算法说明：
		*  - NONE: 无算法（默认值）
		*  - SHA1: SHA-1算法（160位），不推荐使用，安全性较低
		*  - SHA224: SHA-224算法（224位）
		*  - SHA256: SHA-256算法（256位），推荐使用
		*  - SHA384: SHA-384算法（384位）
		*  - SHA512: SHA-512算法（512位）
		*  
		*  @note WebRTC推荐使用SHA-256算法
		*  @note SHA-1算法已被废弃，但为了兼容性仍然支持
		*/
		enum  class  FingerprintAlgorithm
		{
			NONE = 0,        ///< 无算法
			SHA1 = 1,        ///< SHA-1算法（不推荐）
			SHA224,          ///< SHA-224算法
			SHA256,          ///< SHA-256算法（推荐）
			SHA384,          ///< SHA-384算法
			SHA512           ///< SHA-512算法
		};
		
		/**
		*  @brief 证书指纹结构体（Certificate Fingerprint Structure）
		*  
		*  该结构体表示一个证书指纹，包含哈希算法和指纹值。
		*  指纹用于在SDP中验证DTLS证书的真实性。
		*  
		*  指纹格式：
		*  - 算法：FingerprintAlgorithm枚举值
		*  - 值：十六进制字符串，字节之间用冒号分隔
		*  
		*  示例：
		*  - algorithm: SHA256
		*  - value: "AB:CD:EF:12:34:56:78:90:..."
		*/
		struct Fingerprint
		{
			FingerprintAlgorithm algorithm{ FingerprintAlgorithm::NONE };  ///< 哈希算法
			std::string value;                                             ///< 指纹值（十六进制字符串）
		};
		// clang-format off
		//extern   std::map<  std::string, FingerprintAlgorithm> kString2FingerprintAlgorithm;
		//extern   std::map<FingerprintAlgorithm,    std::string> kFingerprintAlgorithm2String;
		
		/**
		*  @author chensong
		*  @date 2025-10-14
		*  @brief DTLS证书管理类（DTLS Certificate Manager）
		*  
		*  DtlsCerts类用于管理DTLS证书和私钥，提供证书生成、加载、指纹计算等功能。
		*  它采用单例模式设计，确保整个应用程序中只有一个证书实例。
		*  
		*  主要功能：
		*  1. 证书和私钥管理：生成或从文件加载证书和私钥
		*  2. 指纹计算：计算证书的多种哈希指纹
		*  3. SSL上下文管理：创建和管理OpenSSL的SSL_CTX
		*  4. DTLS检测：检测数据包是否为DTLS包
		*  
		*  证书生成说明：
		*  - 使用RSA算法生成2048位密钥对
		*  - 证书有效期默认为365天
		*  - 证书包含随机的通用名称（CN）
		*  - 证书为自签名证书
		*  
		*  @note 该类采用单例模式，通过GetInstance()获取实例
		*  @note 证书和私钥在Init()时加载或生成
		*  @note 证书指纹用于SDP交换和DTLS握手验证
		*  
		*  使用示例：
		*  @code
		*  // 获取单例实例
		*  auto& dtls_certs = DtlsCerts::GetInstance();
		*  
		*  // 初始化（自动生成证书）
		*  dtls_certs.Init();
		*  
		*  // 或从文件加载
		*  dtls_certs.Init("cert.pem", "key.pem");
		*  
		*  // 获取指纹
		*  auto fingerprints = dtls_certs.Fingerprints();
		*  for (const auto& fp : fingerprints) {
		*      std::cout << "Fingerprint: " << fp.value << std::endl;
		*  }
		*  @endcode
		*/
		class DtlsCerts
		{
		public:
			/**
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于初始化DtlsCerts实例。由于采用单例模式，
			*  构造函数为私有，只能通过GetInstance()获取实例。
			*  
			*  @note 构造函数不会加载或生成证书，需要调用Init()
			*/
			DtlsCerts();
			
			/**
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理DtlsCerts实例。它会释放证书、私钥和SSL上下文。
			*  
			*  清理流程：
			*  1. 释放SSL_CTX对象
			*  2. 释放X509证书对象
			*  3. 释放EVP_PKEY私钥对象
			*  
			*  @note 析构函数会自动调用，也可以手动调用Destroy()
			*/
			virtual ~DtlsCerts();


			/**
			*  @brief 获取单例实例（Get Singleton Instance）
			*  
			*  该静态方法用于获取DtlsCerts的单例实例。采用懒汉式单例模式，
			*  在第一次调用时创建实例，后续调用返回同一个实例。
			*  
			*  @return 返回DtlsCerts实例的引用
			*  @note 该方法是线程安全的，C++11保证静态局部变量的初始化是线程安全的
			*  @note 返回的是引用，不需要手动释放内存
			*  
			*  使用示例：
			*  @code
			*  auto& dtls_certs = DtlsCerts::GetInstance();
			*  dtls_certs.Init();
			*  @endcode
			*/
			static DtlsCerts &GetInstance()
			{
				static DtlsCerts  instanace;
				return instanace;
			}
		public:
			/**
			*  @brief 获取指纹算法（Get Fingerprint Algorithm）
			*  
			*  该静态方法用于将指纹算法字符串转换为枚举值。
			*  
			*  支持的算法字符串：
			*  - "sha-1" -> SHA1
			*  - "sha-224" -> SHA224
			*  - "sha-256" -> SHA256
			*  - "sha-384" -> SHA384
			*  - "sha-512" -> SHA512
			*  
			*  @param fingerprint 指纹算法字符串（不区分大小写）
			*  @return 返回对应的FingerprintAlgorithm枚举值，如果不支持返回NONE
			*  
			*  使用示例：
			*  @code
			*  auto algo = DtlsCerts::GetFingerprintAlgorithm("sha-256");
			*  if (algo == FingerprintAlgorithm::SHA256) {
			*      // 使用SHA-256算法
			*  }
			*  @endcode
			*/
			static FingerprintAlgorithm GetFingerprintAlgorithm(const std::string& fingerprint);
			
			/**
			*  @brief 获取指纹算法字符串（Get Fingerprint Algorithm String）
			*  
			*  该静态方法用于将指纹算法枚举值转换为字符串。
			*  
			*  @param fingerprint 指纹算法枚举值
			*  @return 返回对应的算法字符串，如"sha-256"
			*  
			*  使用示例：
			*  @code
			*  std::string algo_str = DtlsCerts::GetFingerprintAlgorithmString(FingerprintAlgorithm::SHA256);
			*  // algo_str = "sha-256"
			*  @endcode
			*/
			static std::string  GetFingerprintAlgorithmString(FingerprintAlgorithm fingerprint);
			
			/**
			*  @brief 检测是否为DTLS数据包（Is DTLS Packet）
			*  
			*  该静态方法用于检测数据包是否为DTLS数据包。通过检查数据包的
			*  长度和第一个字节的值来判断。
			*  
			*  DTLS数据包特征：
			*  - 最小长度为13字节
			*  - 第一个字节的值在20-63之间（DTLS记录类型）
			*  
			*  DTLS记录类型：
			*  - 20: ChangeCipherSpec
			*  - 21: Alert
			*  - 22: Handshake
			*  - 23: Application Data
			*  
			*  @param data 数据包指针
			*  @param len 数据包长度（字节）
			*  @return 如果是DTLS数据包返回true，否则返回false
			*  @note 该方法用于区分DTLS、STUN、RTP等不同类型的数据包
			*  @note 参考：https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
			*  
			*  使用示例：
			*  @code
			*  uint8_t data[100];
			*  size_t len = 100;
			*  if (DtlsCerts::IsDtls(data, len)) {
			*      // 处理DTLS数据包
			*  }
			*  @endcode
			*/
			static bool IsDtls(const uint8_t* data, size_t len)
			{
				// clang-format off
				return (
					// Minimum DTLS record length is 13 bytes.
					(len >= 13) &&
					// DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
					(data[0] > 19 && data[0] < 64)
					);
				// clang-format on
			}

		public:
			/**
			*  @brief 初始化DTLS证书（Initialize DTLS Certificate）
			*  
			*  该方法用于初始化DTLS证书和私钥。如果提供了证书文件路径，
			*  则从文件加载；否则自动生成新的证书和私钥。
			*  
			*  初始化流程：
			*  1. 检查是否提供证书和私钥文件路径
			*  2. 如果提供，则从文件加载证书和私钥
			*  3. 如果未提供，则自动生成新的证书和私钥
			*  4. 创建SSL上下文（SSL_CTX）
			*  5. 计算证书指纹
			*  
			*  @param dtls_certificate_file DTLS证书文件路径（PEM格式），为nullptr则自动生成
			*  @param dtls_private_key_file DTLS私钥文件路径（PEM格式），为nullptr则自动生成
			*  @return 如果初始化成功返回true，否则返回false
			*  @note 证书文件必须为PEM格式
			*  @note 如果从文件加载失败，会回退到自动生成
			*  
			*  使用示例：
			*  @code
			*  // 自动生成证书
			*  dtls_certs.Init();
			*  
			*  // 从文件加载证书
			*  dtls_certs.Init("cert.pem", "key.pem");
			*  @endcode
			*/
			bool Init(const char * dtls_certificate_file = nullptr,
				const char * dtls_private_key_file = nullptr);
			
			/**
			*  @brief 销毁DTLS证书（Destroy DTLS Certificate）
			*  
			*  该方法用于销毁DTLS证书和私钥，释放所有相关资源。
			*  
			*  清理流程：
			*  1. 释放SSL_CTX对象
			*  2. 释放X509证书对象
			*  3. 释放EVP_PKEY私钥对象
			*  4. 清空指纹列表
			*  
			*  @note 该方法在析构函数中自动调用
			*  @note 销毁后需要重新调用Init()才能使用
			*/
			void Destroy();
			
			/**
			*  @brief 获取证书指纹列表（Get Certificate Fingerprints）
			*  
			*  该方法用于获取证书的指纹列表。每个指纹使用不同的哈希算法计算。
			*  
			*  @return 返回包含多个指纹的vector，每个指纹包含算法和值
			*  @note 指纹在Init()时计算
			*  @note 返回的指纹包括SHA-1、SHA-224、SHA-256、SHA-384、SHA-512
			*  
			*  使用示例：
			*  @code
			*  auto fingerprints = dtls_certs.Fingerprints();
			*  for (const auto& fp : fingerprints) {
			*      std::cout << "Algorithm: " << static_cast<int>(fp.algorithm) << std::endl;
			*      std::cout << "Value: " << fp.value << std::endl;
			*  }
			*  @endcode
			*/
			 std::vector<libssl::Fingerprint> Fingerprints();
			
			/**
			*  @brief 获取私钥（Get Private Key）
			*  
			*  该方法用于获取DTLS私钥对象。
			*  
			*  @return 返回EVP_PKEY私钥指针，如果未初始化返回nullptr
			*  @note 返回的指针由DtlsCerts管理，不要手动释放
			*  
			*  使用示例：
			*  @code
			*  EVP_PKEY* pkey = dtls_certs.GetPrivateKey();
			*  if (pkey) {
			*      // 使用私钥
			*  }
			*  @endcode
			*/
			EVP_PKEY *GetPrivateKey()const;
			
			/**
			*  @brief 获取证书（Get Certificate）
			*  
			*  该方法用于获取DTLS证书对象。
			*  
			*  @return 返回X509证书指针，如果未初始化返回nullptr
			*  @note 返回的指针由DtlsCerts管理，不要手动释放
			*  
			*  使用示例：
			*  @code
			*  X509* cert = dtls_certs.GetCertificate();
			*  if (cert) {
			*      // 使用证书
			*  }
			*  @endcode
			*/
			X509 * GetCertificate() const ;
			
			/**
			*  @brief 获取SSL上下文（Get SSL Context）
			*  
			*  该方法用于获取OpenSSL的SSL_CTX对象。
			*  
			*  @return 返回SSL_CTX指针，如果未初始化返回nullptr
			*  @note 返回的指针由DtlsCerts管理，不要手动释放
			*  @note SSL_CTX用于创建SSL连接
			*  
			*  使用示例：
			*  @code
			*  SSL_CTX* ctx = dtls_certs.GetSslCtx();
			*  if (ctx) {
			*      SSL* ssl = SSL_new(ctx);
			*  }
			*  @endcode
			*/
			SSL_CTX * GetSslCtx() const;
		public:
			/**
			*  @brief 生成证书和私钥（Generate Certificate and Private Key）
			*  
			*  该方法用于自动生成DTLS证书和私钥。
			*  
			*  生成流程：
			*  1. 生成RSA 2048位密钥对
			*  2. 创建X509证书结构
			*  3. 设置证书版本为V3
			*  4. 设置证书序列号为随机数
			*  5. 设置证书有效期（默认365天）
			*  6. 设置证书主题和颁发者（CN=随机字符串）
			*  7. 设置证书公钥
			*  8. 使用私钥签名证书
			*  
			*  @note 生成的证书为自签名证书
			*  @note 证书有效期从当前时间开始
			*/
			void GenerateCertificateAndPrivateKey();
			
			/**
			*  @brief 从文件读取证书和私钥（Read Certificate and Private Key from Files）
			*  
			*  该方法用于从PEM格式的文件中读取证书和私钥。
			*  
			*  @param dtls_certificate_file 证书文件路径（PEM格式）
			*  @param dtls_private_key_file 私钥文件路径（PEM格式）
			*  @note 如果读取失败，会回退到自动生成
			*  @note 文件格式必须为PEM
			*/
			void ReadCertificateAndPrivateKeyFromFiles(const char * dtls_certificate_file = nullptr,
				const char * dtls_private_key_file = nullptr);
			
			/**
			*  @brief 创建SSL上下文（Create SSL Context）
			*  
			*  该方法用于创建OpenSSL的SSL_CTX对象。
			*  
			*  创建流程：
			*  1. 创建DTLS方法（DTLS_method）
			*  2. 创建SSL_CTX对象
			*  3. 设置SSL_CTX选项
			*  4. 加载证书和私钥到SSL_CTX
			*  5. 设置密码套件和验证模式
			*  
			*  @note 该方法在Init()时自动调用
			*/
			void CreateSslCtx();
			
			/**
			*  @brief 生成证书指纹（Generate Certificate Fingerprints）
			*  
			*  该方法用于计算证书的多种哈希指纹。
			*  
			*  生成流程：
			*  1. 获取证书的DER编码
			*  2. 使用多种哈希算法（SHA-1、SHA-224、SHA-256、SHA-384、SHA-512）计算哈希值
			*  3. 将哈希值转换为十六进制字符串（字节间用冒号分隔）
			*  4. 存储到指纹列表
			*  
			*  @note 该方法在Init()时自动调用
			*  @note 生成的指纹用于SDP交换
			*/
			void GenerateFingerprints();

			/**
			*  @brief 生成随机数（Generate Random Number）
			*  
			*  该方法用于生成32位无符号随机数。
			*  
			*  @return 返回随机生成的uint32_t值
			*  @note 使用C++11标准库的随机数生成器
			*  @note 用于生成证书序列号等随机值
			*/
			uint32_t GenRandom();
		private:
 
			////////////////////////////////////////////
			// 私有成员变量

			X509*      certificate_{ nullptr };                          ///< X509证书对象，用于DTLS握手
			EVP_PKEY*  private_key_{ nullptr };                          ///< EVP_PKEY私钥对象，用于证书签名和DTLS握手
			SSL_CTX*   ssl_ctx_{ nullptr };                              ///< SSL上下文对象，用于创建SSL连接
			
			std::vector<libssl::Fingerprint> local_fingerprints_;        ///< 本地证书指纹列表，包含多种哈希算法的指纹
		};
	}


}


#endif // 