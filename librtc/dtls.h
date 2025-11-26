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

purpose:		DTLS协议实现（DTLS Protocol Implementation）


DTLS协议说明：
- DTLS（Datagram Transport Layer Security）是TLS协议的数据报版本
- DTLS在WebRTC中用于密钥交换，建立SRTP加密会话
- DTLS握手完成后，双方可以获得SRTP密钥材料
- DTLS还可以用于传输SCTP数据通道的应用数据

DTLS状态机（DTLS State Machine）：

    NONE ----Run()----> CONNECTING ----Handshake Success----> CONNECTED
                              |                                    |
                              |                                    |
                         Handshake Failed                      Close/Reset
                              |                                    |
                              v                                    v
                           FAILED                               CLOSED

DTLS角色（DTLS Role）：
- CLIENT: DTLS客户端，主动发起握手
- SERVER: DTLS服务端，被动响应握手
- AUTO: 自动选择角色（根据SDP协商）

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
    |  Extract SRTP Keys                      |
    |<===========================(6)===========>|
    |                                          |
    |  Application Data (SCTP)  <----------->  |
    |<===========================(7)===========>|


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


#ifndef _C_LIBRTC_DTLS__H_
#define _C_LIBRTC_DTLS__H_

#include <cstddef>

#include "libmedia_transfer_protocol/librtc/dtls_certs.h"


namespace libmedia_transfer_protocol {

	namespace libssl
	{
		static const int32_t  kDtlsMtu{ 1350 };               ///< DTLS最大传输单元（MTU），默认1350字节
		static const int32_t  kSsslReadBufferSize{ 65536 };  ///< SSL读取缓冲区大小，默认64KB
		
		/**
		*  @brief DTLS状态枚举（DTLS State Enum）
		*  
		*  该枚举定义了DTLS连接的状态。DTLS连接从NONE状态开始，
		*  经过CONNECTING状态，最终到达CONNECTED或FAILED/CLOSED状态。
		*  
		*  状态说明：
		*  - NONE: 初始状态，未开始连接
		*  - CONNECTING: 正在进行DTLS握手
		*  - CONNECTED: DTLS握手成功，连接已建立
		*  - FAILED: DTLS握手失败
		*  - CLOSED: DTLS连接已关闭
		*  
		*  状态转换：
		*  NONE -> CONNECTING -> CONNECTED
		*                    |-> FAILED
		*  CONNECTED -> CLOSED
		*  
		*  @note FAILED和CLOSED状态都表示连接不可用
		*  @note 从FAILED或CLOSED状态恢复需要重新创建DTLS对象
		*/
		enum  class  DtlsState
		{
			NONE = 1,      ///< 初始状态，未开始连接
			CONNECTING,    ///< 正在进行DTLS握手
			CONNECTED,     ///< DTLS握手成功，连接已建立
			FAILED,        ///< DTLS握手失败
			CLOSED         ///< DTLS连接已关闭
		};
		 
		/**
		*  @brief DTLS角色枚举（DTLS Role Enum）
		*  
		*  该枚举定义了DTLS连接中的角色。在DTLS握手中，一方是客户端（CLIENT），
		*  另一方是服务端（SERVER）。角色决定了谁先发送ClientHello消息。
		*  
		*  角色说明：
		*  - NONE: 未指定角色
		*  - AUTO: 自动选择角色（根据SDP中的setup属性协商）
		*  - CLIENT: DTLS客户端，主动发起握手
		*  - SERVER: DTLS服务端，被动响应握手
		*  
		*  SDP setup属性与角色的对应关系：
		*  - setup:active -> CLIENT（主动连接）
		*  - setup:passive -> SERVER（被动等待）
		*  - setup:actpass -> 可以是CLIENT或SERVER（根据对方决定）
		*  
		*  @note WebRTC中通常一方设置为actpass，另一方设置为active
		*  @note 角色在Run()方法中设置
		*/
		enum class  Role
		{
			NONE = 0,     ///< 未指定角色
			AUTO = 1,     ///< 自动选择角色
			CLIENT,       ///< DTLS客户端
			SERVER        ///< DTLS服务端
		};
		


		/**
		*  @author chensong
		*  @date 2025-10-14
		*  @brief DTLS协议实现类（DTLS Protocol Implementation）
		*  
		*  Dtls类实现了DTLS协议，用于在WebRTC中进行密钥交换和建立SRTP加密会话。
		*  它使用OpenSSL库进行DTLS握手，并通过信号槽机制通知上层应用握手状态和数据。
		*  
		*  主要功能：
		*  1. DTLS握手：进行DTLS客户端或服务端握手
		*  2. 密钥提取：从DTLS握手中提取SRTP密钥材料
		*  3. 证书验证：验证远程证书指纹是否匹配
		*  4. 应用数据传输：在DTLS连接上传输SCTP数据通道数据
		*  5. 状态管理：管理DTLS连接状态
		*  
		*  工作流程：
		*  1. 创建Dtls对象并设置远程证书指纹
		*  2. 调用Run()开始DTLS握手（指定CLIENT或SERVER角色）
		*  3. 通过OnRecv()接收DTLS数据包
		*  4. 通过SignalDtlsSendPakcet信号发送DTLS数据包
		*  5. 握手成功后通过SignalDtlsConnected信号通知，并提供SRTP密钥
		*  6. 使用SendApplicationData()发送应用数据（SCTP）
		*  
		*  @note 该类使用信号槽模式进行事件通知
		*  @note DTLS握手在独立的任务队列中执行
		*  @note 支持超时重传机制
		*  
		*  使用示例：
		*  @code
		*  // 创建DTLS对象
		*  Dtls dtls(task_queue_factory);
		*  
		*  // 连接信号
		*  dtls.SignalDtlsConnected.connect([](Dtls* d, ...) {
		*      // 处理连接成功，获取SRTP密钥
		*  });
		*  
		*  // 设置远程证书指纹
		*  Fingerprint fp;
		*  fp.algorithm = FingerprintAlgorithm::SHA256;
		*  fp.value = "AB:CD:EF:...";
		*  dtls.SetRemoteFingerprint(fp);
		*  
		*  // 开始握手
		*  dtls.Run(Role::CLIENT);
		*  
		*  // 接收数据
		*  dtls.OnRecv(data, size);
		*  @endcode
		*/
		class Dtls : public sigslot::has_slots<>
		{
		public:
			/**
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建Dtls实例。它会初始化OpenSSL的SSL对象和BIO对象。
			*  
			*  初始化流程：
			*  1. 创建任务队列用于DTLS操作
			*  2. 从DtlsCerts获取SSL上下文
			*  3. 创建SSL对象
			*  4. 创建内存BIO对象用于数据读写
			*  5. 设置SSL为DTLS模式
			*  6. 设置MTU大小
			*  
			*  @param task_queue_factory 任务队列工厂，用于创建DTLS任务队列
			*  @note SSL对象和BIO对象在析构时自动释放
			*/
			Dtls(webrtc::TaskQueueFactory* task_queue_factory)  ;
			
			/**
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理Dtls实例。它会释放SSL对象和BIO对象。
			*  
			*  清理流程：
			*  1. 释放SSL对象
			*  2. 释放BIO对象
			*  3. 清理任务队列
			*  
			*  @note 析构函数会自动调用，也可以手动调用Reset()
			*/
			~Dtls();

			/**
			*  @brief 运行DTLS握手（Run DTLS Handshake）
			*  
			*  该方法用于开始DTLS握手。根据指定的角色（CLIENT或SERVER）
			*  启动DTLS握手流程。
			*  
			*  握手流程：
			*  1. 设置本地角色（CLIENT或SERVER）
			*  2. 设置SSL为客户端或服务端模式
			*  3. 启动握手定时器
			*  4. 发送第一个握手消息（CLIENT发送ClientHello）
			*  5. 等待对方响应
			*  
			*  @param local_role 本地角色，CLIENT或SERVER
			*  @note 该方法会触发SignalDtlsConnecting信号
			*  @note 握手成功后会触发SignalDtlsConnected信号
			*  @note 握手失败后会触发SignalDtlsFailed信号
			*  
			*  使用示例：
			*  @code
			*  dtls.Run(Role::CLIENT);  // 作为客户端发起握手
			*  dtls.Run(Role::SERVER);  // 作为服务端等待握手
			*  @endcode
			*/
			void Run(Role local_role);

			/**
			*  @brief 接收DTLS数据（On Receive DTLS Data）
			*  
			*  该方法用于接收DTLS数据包。数据包可能是握手消息或应用数据。
			*  
			*  处理流程：
			*  1. 将数据写入BIO读缓冲区
			*  2. 调用SSL_read()读取数据
			*  3. 如果是握手数据，继续握手流程
			*  4. 如果是应用数据，触发SignalDtlsApplicationDataReceived信号
			*  5. 发送待发送的DTLS数据
			*  
			*  @param data 接收到的数据指针
			*  @param size 数据大小（字节）
			*  @note 该方法在网络线程中调用
			*  @note 数据可能触发握手状态变化
			*  
			*  使用示例：
			*  @code
			*  // 接收到UDP数据包后
			*  if (DtlsCerts::IsDtls(data, size)) {
			*      dtls.OnRecv(data, size);
			*  }
			*  @endcode
			*/
			void OnRecv(const uint8_t *data, int32_t size);
			
			/**
			*  @brief 发送应用数据（Send Application Data）
			*  
			*  该方法用于在DTLS连接上发送应用数据（通常是SCTP数据）。
			*  
			*  发送流程：
			*  1. 检查DTLS连接是否已建立
			*  2. 使用SSL_write()写入数据
			*  3. 从BIO写缓冲区读取加密后的数据
			*  4. 通过SignalDtlsSendPakcet信号发送数据
			*  
			*  @param data 应用数据指针
			*  @param len 数据长度（字节）
			*  @note DTLS连接必须处于CONNECTED状态
			*  @note 发送的数据会被DTLS加密
			*  
			*  使用示例：
			*  @code
			*  // 发送SCTP数据
			*  uint8_t sctp_data[1000];
			*  size_t sctp_len = 1000;
			*  dtls.SendApplicationData(sctp_data, sctp_len);
			*  @endcode
			*/
			void SendApplicationData(const uint8_t* data, size_t len);
			
			/**
			*  @brief 设置远程证书指纹（Set Remote Certificate Fingerprint）
			*  
			*  该方法用于设置远程证书指纹，用于在握手完成后验证远程证书。
			*  
			*  @param fingerprint 远程证书指纹，包含算法和指纹值
			*  @return 如果设置成功返回true，否则返回false
			*  @note 必须在Run()之前调用
			*  @note 指纹从SDP中的a=fingerprint行获取
			*  
			*  使用示例：
			*  @code
			*  Fingerprint fp;
			*  fp.algorithm = FingerprintAlgorithm::SHA256;
			*  fp.value = "AB:CD:EF:12:34:56:...";
			*  if (dtls.SetRemoteFingerprint(fp)) {
			*      dtls.Run(Role::CLIENT);
			*  }
			*  @endcode
			*/
			bool SetRemoteFingerprint(Fingerprint fingerprint);
			 
			

		public:
			/**
			*  @brief DTLS信号（DTLS Signals）
			*  
			*  这些信号用于通知DTLS事件给上层应用。
			*/
			
			/**
			*  @brief DTLS连接中信号
			*  
			*  当DTLS开始连接时触发此信号。
			*  
			*  参数：
			*  - Dtls*: DTLS对象指针
			*/
			sigslot::signal1<Dtls*>									SignalDtlsConnecting;
			
			/**
			*  @brief DTLS连接成功信号
			*  
			*  当DTLS握手成功完成时触发此信号，并提供SRTP密钥材料。
			*  
			*  参数：
			*  - Dtls*: DTLS对象指针
			*  - libsrtp::CryptoSuite: SRTP加密套件
			*  - uint8_t*: 本地SRTP密钥指针
			*  - size_t: 本地SRTP密钥长度
			*  - uint8_t*: 远程SRTP密钥指针
			*  - size_t: 远程SRTP密钥长度
			*  - std::string&: 远程证书指纹
			*/
			sigslot::signal7<Dtls*, libsrtp::CryptoSuite  ,
				uint8_t*  , size_t  ,
				uint8_t*  , size_t  , std::string&  >				SignalDtlsConnected; 
			
			/**
			*  @brief DTLS连接关闭信号
			*  
			*  当DTLS连接正常关闭时触发此信号。
			*  
			*  参数：
			*  - Dtls*: DTLS对象指针
			*/
			sigslot::signal1<Dtls*>									SignalDtlsClose;
			
			/**
			*  @brief DTLS连接失败信号
			*  
			*  当DTLS握手失败时触发此信号。
			*  
			*  参数：
			*  - Dtls*: DTLS对象指针
			*/
			sigslot::signal1<Dtls*>									SignalDtlsFailed; 
			
			/**
			*  @brief DTLS发送数据包信号
			*  
			*  当DTLS需要发送数据包时触发此信号。
			*  
			*  参数：
			*  - Dtls*: DTLS对象指针
			*  - const uint8_t*: 数据指针
			*  - size_t: 数据长度
			*/
			sigslot::signal3< Dtls*,const uint8_t *, size_t>		SignalDtlsSendPakcet; 
			
			/**
			*  @brief DTLS应用数据接收信号
			*  
			*  当DTLS接收到应用数据时触发此信号（通常是SCTP数据）。
			*  
			*  参数：
			*  - Dtls*: DTLS对象指针
			*  - const uint8_t*: 数据指针
			*  - size_t: 数据长度
			*/
			sigslot::signal3<  Dtls*, const uint8_t *, size_t>		SignalDtlsApplicationDataReceived;
		private:
			 
			 
			 


		public:
			/**
			*  @brief SSL信息回调（SSL Info Callback）
			*  
			*  该方法在OpenSSL事件发生时被调用，用于跟踪SSL状态变化。
			*  
			*  @param where SSL事件类型标志
			*  @param ret SSL事件返回值
			*  @note 该方法由OpenSSL内部调用
			*  @note 用于调试和监控DTLS握手过程
			*/
			void OnSslInfo(int32_t where, int32_t ret);

			/**
			*  @brief 定时器回调（Timer Callback）
			*  
			*  该方法用于处理DTLS超时重传。DTLS使用超时重传机制确保
			*  握手消息的可靠传输。
			*  
			*  处理流程：
			*  1. 检查握手是否超时
			*  2. 如果超时，重传上一个握手消息
			*  3. 更新超时时间
			*  
			*  @note 该方法在任务队列中定期调用
			*  @note DTLS使用指数退避算法计算超时时间
			*/
			void OnTimer();

		private:
			/**
			*  @brief 检查DTLS是否正在运行（Is Running）
			*  
			*  该方法用于检查DTLS是否处于运行状态（CONNECTING或CONNECTED）。
			*  
			*  @return 如果DTLS正在运行返回true，否则返回false
			*  @note CONNECTING和CONNECTED状态表示DTLS正在运行
			*  @note NONE、FAILED和CLOSED状态表示DTLS未运行
			*/
			bool IsRunning() const
			{
				switch (this->state_)
				{
				case DtlsState::NONE:
					return false;
				case DtlsState::CONNECTING:
				case DtlsState::CONNECTED:
					return true;
				case DtlsState::FAILED:
				case DtlsState::CLOSED:
					return false;
				}

				// Make GCC 4.9 happy.
				return false;
			}

			/**
			*  @brief 重置DTLS连接（Reset DTLS）
			*  
			*  该方法用于重置DTLS连接，清理所有状态和资源。
			*  
			*  重置流程：
			*  1. 释放SSL对象
			*  2. 释放BIO对象
			*  3. 重新创建SSL和BIO对象
			*  4. 重置状态为NONE
			*  5. 清空握手完成标志
			*  
			*  @note 重置后需要重新调用Run()开始握手
			*/
			void Reset();
			
			/**
			*  @brief 检查SSL操作状态（Check SSL Status）
			*  
			*  该方法用于检查SSL操作的返回值，并处理各种错误情况。
			*  
			*  @param returnCode SSL操作返回值
			*  @return 如果操作成功返回true，否则返回false
			*  @note 处理SSL_ERROR_WANT_READ、SSL_ERROR_WANT_WRITE等错误
			*/
			bool CheckStatus(int returnCode);
			
			/**
			*  @brief 发送待发送的DTLS数据（Send Pending Outgoing DTLS Data）
			*  
			*  该方法用于从BIO写缓冲区读取数据并通过SignalDtlsSendPakcet信号发送。
			*  
			*  发送流程：
			*  1. 从BIO写缓冲区读取数据
			*  2. 如果有数据，触发SignalDtlsSendPakcet信号
			*  3. 重复直到没有数据
			*  
			*  @note 该方法在握手和应用数据发送后调用
			*/
			void SendPendingOutgoingDtlsData();
			
			/**
			*  @brief 设置超时（Set Timeout）
			*  
			*  该方法用于设置DTLS握手超时时间并启动定时器。
			*  
			*  @return 如果设置成功返回true，否则返回false
			*  @note 使用DTLSv1_get_timeout()获取超时时间
			*  @note 超时时间使用指数退避算法
			*/
			bool SetTimeout();
			
			/**
			*  @brief 处理握手（Process Handshake）
			*  
			*  该方法用于处理DTLS握手流程。
			*  
			*  处理流程：
			*  1. 调用SSL_do_handshake()进行握手
			*  2. 检查握手是否完成
			*  3. 如果完成，验证远程证书指纹
			*  4. 提取SRTP密钥
			*  5. 触发SignalDtlsConnected信号
			*  
			*  @return 如果握手成功返回true，否则返回false
			*  @note 握手可能需要多次调用才能完成
			*/
			bool ProcessHandshake();
			
			/**
			*  @brief 检查远程证书指纹（Check Remote Certificate Fingerprint）
			*  
			*  该方法用于验证远程证书指纹是否与预期的指纹匹配。
			*  
			*  验证流程：
			*  1. 获取远程证书
			*  2. 计算证书指纹
			*  3. 与SetRemoteFingerprint()设置的指纹比较
			*  4. 如果匹配返回true，否则返回false
			*  
			*  @return 如果指纹匹配返回true，否则返回false
			*  @note 指纹不匹配会导致握手失败
			*  @note 这是防止中间人攻击的重要措施
			*/
			bool CheckRemoteFingerprint();
			
			/**
			*  @brief 提取SRTP密钥（Extract SRTP Keys）
			*  
			*  该方法用于从DTLS握手中提取SRTP密钥材料。
			*  
			*  提取流程：
			*  1. 使用SSL_export_keying_material()导出密钥材料
			*  2. 根据加密套件分割密钥材料
			*  3. 得到本地和远程的SRTP主密钥和盐值
			*  
			*  @param srtpCryptoSuite SRTP加密套件
			*  @note 密钥材料用于创建SRTP会话
			*  @note 不同的加密套件需要不同长度的密钥
			*/
			void ExtractSrtpKeys(libsrtp::CryptoSuite srtpCryptoSuite);
			
			/**
			*  @brief 获取协商的SRTP加密套件（Get Negotiated SRTP Crypto Suite）
			*  
			*  该方法用于获取DTLS握手协商的SRTP加密套件。
			*  
			*  @return 返回协商的SRTP加密套件
			*  @note 加密套件在DTLS握手中通过use_srtp扩展协商
			*  @note 常用套件：SRTP_AES128_CM_HMAC_SHA1_80、SRTP_AEAD_AES_128_GCM
			*/
			libmedia_transfer_protocol::libsrtp::CryptoSuite GetNegotiatedSrtpCryptoSuite();

			
		private: 
			///
			/// 私有成员变量
			///
			
			// OpenSSL相关对象
			SSL * ssl_{ nullptr };                                     ///< OpenSSL SSL对象，用于DTLS连接
			BIO * bio_read_{ nullptr };                                ///< 内存BIO对象，用于读取数据
			BIO * bio_write_{ nullptr };                               ///< 内存BIO对象，用于写入数据
			uint8_t   ssl_read_buffer_[kSsslReadBufferSize];          ///< SSL读取缓冲区，大小为64KB
			
			// DTLS状态
			DtlsState  state_{ DtlsState::NONE };                     ///< DTLS连接状态
			Role    local_role_{ Role::NONE };                         ///< 本地角色（CLIENT或SERVER）

			// 证书指纹
			Fingerprint remote_fingerprint_;                           ///< 远程证书指纹，用于验证远程证书
			
			// 握手状态
			bool handshake_done_{ false };                             ///< 握手完成标志（持久）
			bool handshake_done_now_{ false };                         ///< 握手刚刚完成标志（临时）
			std::string remote_cert_;                                  ///< 远程证书字符串
			
			// 任务队列
			rtc::TaskQueue   dtls_queue_;                              ///< DTLS任务队列，用于异步执行DTLS操作
			 
		};
	}

}

#endif //