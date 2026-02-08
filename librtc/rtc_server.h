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

purpose:		RTC服务器（RTC Server）


RTC服务器说明：
- RtcServer是WebRTC传输层的核心组件
- RtcServer负责UDP数据包的收发和分发
- RtcServer识别和分类不同类型的数据包（STUN、DTLS、RTP、RTCP）
- RtcServer通过信号槽机制将数据包分发给对应的处理器

WebRTC数据包类型识别（Packet Type Detection）：

    UDP数据包
         |
         v
    RtcServer::OnRecvPacket()
         |
         +---> IsStun()  -----> SignalStunPacket  -----> StunHandler
         |
         +---> IsDtls()  -----> SignalDtlsPacket  -----> DtlsHandler
         |
         +---> IsRtp()   -----> SignalRtpPacket   -----> RtpHandler
         |
         +---> IsRtcp()  -----> SignalRtcpPacket  -----> RtcpHandler

数据包类型识别规则：
1. STUN数据包：
   - 第一字节 < 2（STUN头部标志）
   - Magic Cookie = 0x2112A442

2. DTLS数据包：
   - 第一字节范围 20-64（DTLS Content Type）
   - 支持的Content Type：
     * 20: ChangeCipherSpec
     * 21: Alert
     * 22: Handshake
     * 23: ApplicationData

3. RTP数据包：
   - 版本号 = 2（V=2）
   - Payload Type < 64（非RTCP范围）
   - 第二字节 < 200

4. RTCP数据包：
   - 版本号 = 2（V=2）
   - Payload Type >= 200（RTCP范围）
   - 常见RTCP类型：200(SR), 201(RR), 202(SDES), 203(BYE), 204(APP)

线程模型（Thread Model）：

    Signaling Thread
         |
         +---> 信令处理
         
    Worker Thread
         |
         +---> 媒体处理
         |
         +---> SCTP处理
         
    Network Thread
         |
         +---> UDP收发
         |
         +---> 数据包分发


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


#ifndef _C_RTC_SERVER_H_
#define _C_RTC_SERVER_H_

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
#include "rtc_base/thread.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "absl/types/optional.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#ifdef WIN32
#include "rtc_base/win32_socket_server.h"
#include <vcruntime.h>
#endif
#include "rtc_base/async_udp_socket.h"
#include "libmedia_transfer_protocol/transport.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "libmedia_transfer_protocol/rtp_rtcp/rtp_packet_to_send.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "libmedia_transfer_protocol/libnetwork/udp_server.h"
namespace libmedia_transfer_protocol {
	namespace librtc {
		
		/**
		*  @author chensong
		*  @date 2025-10-14
		*  @brief RTC服务器类（RTC Server）
		*  
		*  RtcServer类是WebRTC传输层的核心组件，负责UDP数据包的收发和分发。
		*  它能够识别不同类型的数据包（STUN、DTLS、RTP、RTCP），并通过信号槽
		*  机制将数据包分发给对应的处理器。
		*  
		*  主要功能：
		*  1. UDP服务器：监听UDP端口，接收和发送数据包
		*  2. 数据包识别：识别STUN、DTLS、RTP、RTCP数据包
		*  3. 数据包分发：通过信号槽机制将数据包分发给对应处理器
		*  4. 线程管理：管理信令线程、工作线程、网络线程
		*  5. 数据包发送：支持RTP、RTCP数据包的批量发送
		*  
		*  工作流程：
		*  1. 启动UDP服务器，监听指定端口
		*  2. 接收UDP数据包
		*  3. 识别数据包类型（STUN/DTLS/RTP/RTCP）
		*  4. 触发对应的信号（SignalStunPacket/SignalDtlsPacket等）
		*  5. 数据包处理器接收信号并处理数据包
		*  
		*  数据包识别流程：
		*  1. IsStun()：检查是否为STUN数据包
		*  2. IsDtls()：检查是否为DTLS数据包
		*  3. IsRtp()：检查是否为RTP数据包
		*  4. IsRtcp()：检查是否为RTCP数据包
		*  
		*  @note RtcServer使用信号槽机制进行数据包分发
		*  @note 支持异步和同步两种接收模式
		*  @note 所有数据包收发在网络线程执行
		*  
		*  使用示例：
		*  @code
		*  RtcServer rtc_server;
		*  
		*  // 连接STUN数据包信号
		*  rtc_server.SignalStunPacket.connect(this, &MyClass::OnStunPacket);
		*  
		*  // 连接DTLS数据包信号
		*  rtc_server.SignalDtlsPacket.connect(this, &MyClass::OnDtlsPacket);
		*  
		*  // 连接RTP数据包信号
		*  rtc_server.SignalRtpPacket.connect(this, &MyClass::OnRtpPacket);
		*  
		*  // 启动RTC服务器
		*  rtc_server.Start("0.0.0.0", 10000);
		*  
		*  // 发送RTP数据包
		*  rtc::CopyOnWriteBuffer rtp_packet = ...;
		*  rtc::SocketAddress dest_addr("192.168.1.100", 20000);
		*  rtc::PacketOptions options;
		*  rtc_server.SendRtpPacketTo(rtp_packet, dest_addr, options);
		*  @endcode
		*/
		class RtcServer : public sigslot::has_slots<>
		{
		public:
			/** 默认构造函数 */
			explicit RtcServer();
			
			/** 虚拟析构函数 */
			virtual ~RtcServer();



		public:
			/**
			*  @brief 启动RTC服务器（Start RTC Server）
			*  
			*  该方法用于启动RTC服务器，监听指定的IP地址和端口。
			*  
			*  启动流程：
			*  1. 创建UDP服务器
			*  2. 绑定IP地址和端口
			*  3. 注册数据包接收回调
			*  4. 启动网络线程
			*  
			*  @param ip 监听的IP地址（如"0.0.0.0"表示所有网卡）
			*  @param port 监听的UDP端口
			*  @return 如果启动成功返回true，否则返回false
			*  @note 通常使用"0.0.0.0"监听所有网卡
			*/
			bool Start(const char * ip, uint16_t port);

		public:
			/**
			*  @brief 发送数据包（Send Packet）
			*  
			*  该方法用于发送数据包到默认目标地址。
			*  
			*  @param packet 数据包缓冲区
			*  @param options 数据包选项
			*  @return 返回发送的字节数，失败返回负数
			*/
			int SendPacket(const rtc::Buffer& packet, const rtc::PacketOptions& options);
			
			/**
			*  @brief 发送数据包到指定地址（Send Packet To）
			*  
			*  该方法用于发送数据包到指定的目标地址。
			*  
			*  @param packet 数据包缓冲区
			*  @param addr 目标地址（IP和端口）
			*  @param options 数据包选项
			*  @return 返回发送的字节数，失败返回负数
			*/
			int SendPacketTo(const rtc::Buffer& packet,
				const rtc::SocketAddress& addr,
				const rtc::PacketOptions& options);


			/**
			*  @brief 发送RTP数据包到指定地址（Send RTP Packet To）
			*  
			*  该方法用于发送单个RTP数据包到指定的目标地址。
			*  
			*  @param packet RTP数据包（写时复制缓冲区）
			*  @param addr 目标地址（IP和端口）
			*  @param options 数据包选项（可包含优先级、DSCP等）
			*  @return 返回发送的字节数，失败返回负数
			*  @note RTP数据包用于传输音视频媒体数据
			*/
			int SendRtpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			
			/**
			*  @brief 批量发送RTP数据包到指定地址（Send RTP Packets To）
			*  
			*  该方法用于批量发送多个RTP数据包到指定的目标地址。
			*  批量发送可以提高发送效率，减少系统调用次数。
			*  
			*  @param packets RTP数据包向量
			*  @param addr 目标地址（IP和端口）
			*  @param options 数据包选项
			*  @return 返回成功发送的数据包数量，失败返回负数
			*  @note 批量发送适用于高吞吐量场景
			*/
			int32_t SendRtpPacketTo(std::vector< std::unique_ptr<libmedia_transfer_protocol::RtpPacketToSend>>  packets,
				const rtc::SocketAddress& addr, const rtc::PacketOptions& options);
			
			/**
			*  @brief 发送RTCP数据包到指定地址（Send RTCP Packet To）
			*  
			*  该方法用于发送RTCP数据包到指定的目标地址。
			*  
			*  @param packet RTCP数据包（写时复制缓冲区）
			*  @param addr 目标地址（IP和端口）
			*  @param options 数据包选项
			*  @return 返回发送的字节数，失败返回负数
			*  @note RTCP数据包用于传输控制信息（SR、RR、PLI、FIR等）
			*/
			int SendRtcpPacketTo(rtc::CopyOnWriteBuffer packet, const rtc::SocketAddress& addr, const rtc::PacketOptions& options);

		public:
			/**
			*  @brief 接收数据包回调（异步套接字）（On Receive Packet Callback - Async）
			*  
			*  该方法用于处理从异步套接字接收到的数据包。
			*  
			*  处理流程：
			*  1. 识别数据包类型（STUN/DTLS/RTP/RTCP）
			*  2. 触发对应的信号
			*  3. 数据包处理器接收并处理数据包
			*  
			*  @param socket 异步套接字指针
			*  @param data 数据包数据指针
			*  @param len 数据包长度
			*  @param addr 发送方地址（IP和端口）
			*  @param ms 接收时间戳（毫秒）
			*  @note 此方法在网络线程执行
			*/
			void  OnRecvPacket(rtc::AsyncPacketSocket * socket, const uint8_t * data, size_t len,
				const rtc::SocketAddress & addr, const int64_t & ms);
			
			/**
			*  @brief 接收数据包回调（同步套接字）（On Receive Packet Callback - Sync）
			*  
			*  该方法用于处理从同步套接字接收到的数据包。
			*  
			*  @param socket 同步套接字指针
			*  @param data 数据包数据指针
			*  @param len 数据包长度
			*  @param addr 发送方地址（IP和端口）
			*  @param ms 接收时间戳（毫秒）
			*  @note 此方法在网络线程执行
			*/
			void  OnRecvPacket(rtc::Socket* socket, const uint8_t* data, size_t len,
				const rtc::SocketAddress& addr, const int64_t  ms);
		public:
			/**
			*  @brief 获取信令线程（Get Signaling Thread）
			*  
			*  该方法用于获取信令线程指针。信令线程用于处理WebRTC信令。
			*  
			*  @return 返回信令线程指针
			*/
			rtc::Thread* signaling_thread() { return udp_server_->signaling_thread(); }
			
			/** 获取信令线程（常量版本） */
			const rtc::Thread* signaling_thread() const { return udp_server_->signaling_thread(); }
			
			/**
			*  @brief 获取工作线程（Get Worker Thread）
			*  
			*  该方法用于获取工作线程指针。工作线程用于处理媒体数据和SCTP。
			*  
			*  @return 返回工作线程指针
			*/
			rtc::Thread* worker_thread() { return udp_server_->worker_thread(); }
			
			/** 获取工作线程（常量版本） */
			const rtc::Thread* worker_thread() const { return udp_server_->worker_thread(); }
			
			/**
			*  @brief 获取网络线程（Get Network Thread）
			*  
			*  该方法用于获取网络线程指针。网络线程用于处理UDP收发。
			*  
			*  @return 返回网络线程指针
			*/
			rtc::Thread* network_thread() { return udp_server_->network_thread(); }
			
			/** 获取网络线程（常量版本） */
			const rtc::Thread* network_thread() const { return udp_server_->network_thread(); }
			 
		public:
			/**
			*  @brief STUN数据包信号（异步套接字）
			*  
			*  该信号在接收到STUN数据包时触发。
			*  
			*  参数：
			*  1. AsyncPacketSocket*: 异步套接字指针
			*  2. const uint8_t*: STUN数据包指针
			*  3. size_t: STUN数据包长度
			*  4. const SocketAddress&: 发送方地址
			*  5. const int64_t&: 接收时间戳（毫秒）
			*  
			*  @note 用于ICE连接建立和NAT穿透
			*/
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalStunPacket;
			
			/**
			*  @brief DTLS数据包信号（异步套接字）
			*  
			*  该信号在接收到DTLS数据包时触发。
			*  
			*  参数：
			*  1. AsyncPacketSocket*: 异步套接字指针
			*  2. const uint8_t*: DTLS数据包指针
			*  3. size_t: DTLS数据包长度
			*  4. const SocketAddress&: 发送方地址
			*  5. const int64_t&: 接收时间戳（毫秒）
			*  
			*  @note 用于DTLS握手和SCTP数据传输
			*/
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalDtlsPacket;
			
			/**
			*  @brief RTP数据包信号（异步套接字）
			*  
			*  该信号在接收到RTP数据包时触发。
			*  
			*  参数：
			*  1. AsyncPacketSocket*: 异步套接字指针
			*  2. const uint8_t*: RTP数据包指针
			*  3. size_t: RTP数据包长度
			*  4. const SocketAddress&: 发送方地址
			*  5. const int64_t&: 接收时间戳（毫秒）
			*  
			*  @note 用于接收音视频媒体数据
			*/
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalRtpPacket;
			
			/**
			*  @brief RTCP数据包信号（异步套接字）
			*  
			*  该信号在接收到RTCP数据包时触发。
			*  
			*  参数：
			*  1. AsyncPacketSocket*: 异步套接字指针
			*  2. const uint8_t*: RTCP数据包指针
			*  3. size_t: RTCP数据包长度
			*  4. const SocketAddress&: 发送方地址
			*  5. const int64_t&: 接收时间戳（毫秒）
			*  
			*  @note 用于接收控制信息（SR、RR、PLI、FIR等）
			*/
			sigslot::signal5<rtc::AsyncPacketSocket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalRtcpPacket;


		public:
			/**
			*  @brief STUN数据包信号（同步套接字）
			*  
			*  该信号在接收到STUN数据包时触发（同步套接字版本）。
			*  
			*  参数：
			*  1. Socket*: 同步套接字指针
			*  2. const uint8_t*: STUN数据包指针
			*  3. size_t: STUN数据包长度
			*  4. const SocketAddress&: 发送方地址
			*  5. const int64_t&: 接收时间戳（毫秒）
			*/
			sigslot::signal5<rtc::Socket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalSyncStunPacket;
			
			/**
			*  @brief DTLS数据包信号（同步套接字）
			*  
			*  该信号在接收到DTLS数据包时触发（同步套接字版本）。
			*  
			*  参数：
			*  1. Socket*: 同步套接字指针
			*  2. const uint8_t*: DTLS数据包指针
			*  3. size_t: DTLS数据包长度
			*  4. const SocketAddress&: 发送方地址
			*  5. const int64_t&: 接收时间戳（毫秒）
			*/
			sigslot::signal5<rtc::Socket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalSyncDtlsPacket;
			
			/**
			*  @brief RTP数据包信号（同步套接字）
			*  
			*  该信号在接收到RTP数据包时触发（同步套接字版本）。
			*  
			*  参数：
			*  1. Socket*: 同步套接字指针
			*  2. const uint8_t*: RTP数据包指针
			*  3. size_t: RTP数据包长度
			*  4. const SocketAddress&: 发送方地址
			*  5. const int64_t&: 接收时间戳（毫秒）
			*/
			sigslot::signal5<rtc::Socket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalSyncRtpPacket;
			
			/**
			*  @brief RTCP数据包信号（同步套接字）
			*  
			*  该信号在接收到RTCP数据包时触发（同步套接字版本）。
			*  
			*  参数：
			*  1. Socket*: 同步套接字指针
			*  2. const uint8_t*: RTCP数据包指针
			*  3. size_t: RTCP数据包长度
			*  4. const SocketAddress&: 发送方地址
			*  5. const int64_t&: 接收时间戳（毫秒）
			*/
			sigslot::signal5<rtc::Socket*,
				const uint8_t*,
				size_t,
				const rtc::SocketAddress&,
				// TODO(bugs.webrtc.org/9584): Change to passing the int64_t
				// timestamp by value.
				const int64_t&>
				SignalSyncRtcpPacket;



		 
		public:
			/**
			*  @brief 判断是否为STUN数据包（Is STUN Packet）
			*  
			*  该方法用于判断数据包是否为STUN数据包。
			*  
			*  判断条件：
			*  1. 第一字节 < 2（STUN头部标志）
			*  2. Magic Cookie = 0x2112A442（STUN魔术Cookie）
			*  
			*  @param data 数据包指针
			*  @param len 数据包长度
			*  @return 如果是STUN数据包返回true，否则返回false
			*/
			bool IsStun(const uint8_t  * data, int32_t len);
			
			/**
			*  @brief 判断是否为DTLS数据包（Is DTLS Packet）
			*  
			*  该方法用于判断数据包是否为DTLS数据包。
			*  
			*  判断条件：
			*  1. 第一字节范围 20-64（DTLS Content Type）
			*  2. 支持的Content Type：
			*     - 20: ChangeCipherSpec
			*     - 21: Alert
			*     - 22: Handshake
			*     - 23: ApplicationData
			*  
			*  @param data 数据包指针
			*  @param len 数据包长度
			*  @return 如果是DTLS数据包返回true，否则返回false
			*/
			bool IsDtls(const uint8_t * data, int32_t len);
			
			/**
			*  @brief 判断是否为RTP数据包（Is RTP Packet）
			*  
			*  该方法用于判断数据包是否为RTP数据包。
			*  
			*  判断条件：
			*  1. 版本号 = 2（V=2）
			*  2. Payload Type < 64（非RTCP范围）
			*  3. 第二字节 < 200
			*  
			*  @param data 数据包指针
			*  @param len 数据包长度
			*  @return 如果是RTP数据包返回true，否则返回false
			*  @note RTP用于传输音视频媒体数据
			*/
			bool IsRtp(const uint8_t * data, int32_t len);
			
			/**
			*  @brief 判断是否为RTCP数据包（Is RTCP Packet）
			*  
			*  该方法用于判断数据包是否为RTCP数据包。
			*  
			*  判断条件：
			*  1. 版本号 = 2（V=2）
			*  2. Payload Type >= 200（RTCP范围）
			*  3. 常见RTCP类型：
			*     - 200: SR (Sender Report)
			*     - 201: RR (Receiver Report)
			*     - 202: SDES (Source Description)
			*     - 203: BYE (Goodbye)
			*     - 204: APP (Application-Defined)
			*     - 205: RTPFB (Generic RTP Feedback)
			*     - 206: PSFB (Payload-Specific Feedback，包括PLI、FIR等)
			*  
			*  @param data 数据包指针
			*  @param len 数据包长度
			*  @return 如果是RTCP数据包返回true，否则返回false
			*  @note RTCP用于传输控制信息
			*/
			bool IsRtcp(const uint8_t * data, int32_t len);
		 
		private:
			std::unique_ptr<libnetwork::UdpServer>      udp_server_;  ///< UDP服务器，负责UDP数据包的收发

		};

	}
}

#endif // 