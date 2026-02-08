# librtc - WebRTC传输层库

## 概述

librtc 是一个完整的 WebRTC 传输层实现库，提供了 WebRTC 所需的所有核心协议支持，包括 STUN、DTLS、SRTP、SCTP 等。它为上层应用提供了安全、可靠的实时音视频和数据传输能力。

**核心特性：**
- 完整的 WebRTC 协议栈实现
- STUN/ICE 连接建立和 NAT 穿透
- DTLS 密钥交换和证书管理
- SRTP/SRTCP 媒体加密
- SCTP 数据通道支持
- SDP 协议解析和生成
- 高性能的数据包分发机制

## WebRTC 协议栈

```
┌─────────────────────────────────────────┐
│         Application Layer               │
│    (Audio/Video/DataChannel API)        │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         RTP/RTCP Layer                  │
│    (Media Packetization/Control)        │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         SRTP/SRTCP Layer                │
│    (Media Encryption/Authentication)    │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         DTLS Layer                      │
│    (Key Exchange/SCTP Transport)        │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         ICE/STUN Layer                  │
│    (NAT Traversal/Connectivity Check)   │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         UDP Transport                   │
└─────────────────────────────────────────┘
```

## 文件结构

```
librtc/
├── rtc_server.h/cpp          # RTC服务器（数据包分发）
├── dtls.h/cpp                # DTLS协议实现
├── dtls_certs.h/cpp          # DTLS证书管理
├── srtp_session.h/cpp        # SRTP会话管理
├── sctp_association.h/cpp    # SCTP关联管理
├── stun.h/cpp                # STUN协议实现
├── rtc_sdp.h/cpp             # SDP协议处理
├── rtc_utils.h/cpp           # 工具函数（字节序转换）
├── rtc_errors.h              # 错误定义
├── rtc_log.h                 # 日志定义
└── README.md                 # 本文档
```

## 核心类说明

### 1. RtcServer - RTC服务器

RtcServer 是 WebRTC 传输层的核心组件，负责 UDP 数据包的接收和分发。

**主要功能：**
- UDP 数据包接收
- 数据包类型识别（STUN/DTLS/RTP/RTCP）
- 基于信号槽的数据包分发
- 线程管理（信令/工作/网络线程）

**数据包识别流程：**

```
UDP数据包
    ↓
RtcServer::OnRecvPacket()
    ↓
    +---> IsStun()  -----> SignalStunPacket  -----> StunHandler
    |
    +---> IsDtls()  -----> SignalDtlsPacket  -----> DtlsHandler
    |
    +---> IsRtp()   -----> SignalRtpPacket   -----> RtpHandler
    |
    +---> IsRtcp()  -----> SignalRtcpPacket  -----> RtcpHandler
```

**数据包识别规则：**

| 类型 | 识别条件 | 说明 |
|------|---------|------|
| STUN | 第一字节 < 2，Magic Cookie = 0x2112A442 | ICE连接建立 |
| DTLS | 第一字节 20-64 | 密钥交换、SCTP传输 |
| RTP  | V=2，PT < 64 | 音视频媒体数据 |
| RTCP | V=2，PT >= 200 | 控制信息 |

**使用示例：**

```cpp
RtcServer rtc_server;

// 连接STUN数据包信号
rtc_server.SignalStunPacket.connect([](auto socket, auto data, auto len, auto addr, auto ts) {
    // 处理STUN数据包（ICE连接建立）
    ProcessStunPacket(data, len, addr);
});

// 连接DTLS数据包信号
rtc_server.SignalDtlsPacket.connect([](auto socket, auto data, auto len, auto addr, auto ts) {
    // 处理DTLS数据包（密钥交换）
    ProcessDtlsPacket(data, len, addr);
});

// 连接RTP数据包信号
rtc_server.SignalRtpPacket.connect([](auto socket, auto data, auto len, auto addr, auto ts) {
    // 处理RTP数据包（音视频数据）
    ProcessRtpPacket(data, len, addr);
});

// 启动RTC服务器
rtc_server.Start("0.0.0.0", 10000);
```

### 2. DTLS - 密钥交换

DTLS 类实现了 DTLS 协议，用于密钥交换和建立 SRTP 加密会话。

**主要功能：**
- DTLS 握手（客户端/服务端）
- SRTP 密钥导出
- 证书指纹验证
- SCTP 数据传输

**DTLS 握手流程：**

```
Client                          Server
  |                                |
  | ClientHello                   |
  |------------------------------>|
  |                                |
  |    ServerHello, Certificate   |
  |<------------------------------|
  |                                |
  | Certificate, ClientKeyExchange|
  |------------------------------>|
  |                                |
  | [ChangeCipherSpec] Finished   |
  |------------------------------>|
  |                                |
  |  [ChangeCipherSpec] Finished  |
  |<------------------------------|
  |                                |
  | Extract SRTP Keys             |
  |<=============================>|
```

**使用示例：**

```cpp
Dtls dtls(task_queue_factory);

// 连接信号
dtls.SignalDtlsConnected.connect([](Dtls* d, auto suite, auto local_key, auto local_len,
                                     auto remote_key, auto remote_len, auto& fingerprint) {
    // DTLS握手成功，获取SRTP密钥
    CreateSrtpSession(suite, local_key, local_len, remote_key, remote_len);
});

// 设置远程证书指纹
Fingerprint fp;
fp.algorithm = FingerprintAlgorithm::SHA256;
fp.value = "AB:CD:EF:...";
dtls.SetRemoteFingerprint(fp);

// 开始握手
dtls.Run(Role::CLIENT);

// 接收DTLS数据
dtls.OnRecv(data, size);

// 发送应用数据（SCTP）
dtls.SendApplicationData(sctp_data, sctp_len);
```

### 3. SRTP - 媒体加密

SrtpSession 类用于 RTP/RTCP 数据包的加密和解密。

**主要功能：**
- RTP 加密/解密
- RTCP 加密/解密
- 多种加密套件支持
- 流管理（SSRC）

**SRTP 数据包格式：**

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source (SSRC) identifier            |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|                   Encrypted Payload ...                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                 Authentication Tag (RECOMMENDED)              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**加密套件：**

| 套件 | 密钥长度 | 盐长度 | 认证标签 | 说明 |
|------|---------|--------|---------|------|
| AES_CM_128_HMAC_SHA1_80 | 16字节 | 14字节 | 10字节 | 常用 |
| AES_CM_128_HMAC_SHA1_32 | 16字节 | 14字节 | 4字节 | 低开销 |
| AEAD_AES_128_GCM | 16字节 | 12字节 | 16字节 | 推荐 |
| AEAD_AES_256_GCM | 32字节 | 12字节 | 16字节 | 最安全 |

**使用示例：**

```cpp
// 创建出站会话（发送）
SrtpSession* send_session = new SrtpSession(
    OUTBOUND,
    CryptoSuite::AEAD_AES_128_GCM,
    client_key,
    28  // AES-128-GCM key + salt
);

// 加密RTP数据包
const uint8_t* rtp_data = ...;
size_t rtp_len = ...;
if (send_session->EncryptRtp(&rtp_data, &rtp_len)) {
    // 发送加密后的数据
    SendPacket(rtp_data, rtp_len);
}

// 创建入站会话（接收）
SrtpSession* recv_session = new SrtpSession(
    INBOUND,
    CryptoSuite::AEAD_AES_128_GCM,
    server_key,
    28
);

// 解密SRTP数据包
uint8_t* srtp_data = ...;
size_t srtp_len = ...;
if (recv_session->DecryptSrtp(srtp_data, &srtp_len)) {
    // 处理解密后的RTP数据
    ProcessRtpPacket(srtp_data, srtp_len);
}
```

### 4. SCTP - 数据通道

SctpAssociation 类用于管理 SCTP 关联，提供 WebRTC DataChannel 的底层传输。

**主要功能：**
- SCTP 连接建立
- 可靠/不可靠数据传输
- 多流复用（最多65535个流）
- 有序/无序传输

**SCTP 连接流程：**

```
Client                          Server
  |                                |
  | DTLS Handshake Complete       |
  |<=============================>|
  |                                |
  | SCTP INIT                     |
  |------------------------------>|
  |                                |
  |              SCTP INIT-ACK    |
  |<------------------------------|
  |                                |
  | SCTP COOKIE-ECHO              |
  |------------------------------>|
  |                                |
  |         SCTP COOKIE-ACK       |
  |<------------------------------|
  |                                |
  | SCTP CONNECTED                |
  |<=============================>|
```

**传输模式：**

| 模式 | ordered | maxRetransmits | 说明 |
|------|---------|----------------|------|
| 可靠有序 | true | 0（无限） | TCP类似 |
| 可靠无序 | false | 0（无限） | 乱序到达 |
| 不可靠有序 | true | N | 超时丢弃 |
| 不可靠无序 | false | N | 最快传输 |

**使用示例：**

```cpp
class MyListener : public SctpAssociation::Listener {
    void OnSctpAssociationConnected(SctpAssociation* sctp) override {
        // SCTP连接建立
        std::cout << "SCTP connected" << std::endl;
    }
    
    void OnSctpAssociationMessageReceived(
        SctpAssociation* sctp, uint16_t streamId, 
        uint32_t ppid, const uint8_t* msg, size_t len) override {
        // 接收到DataChannel消息
        ProcessDataChannelMessage(streamId, ppid, msg, len);
    }
    
    void OnSctpAssociationSendData(
        SctpAssociation* sctp, const uint8_t* data, size_t len) override {
        // 通过DTLS发送SCTP数据
        dtls.SendApplicationData(data, len);
    }
};

MyListener listener;
SctpAssociation sctp(&listener, 1024, 1024, 262144, true);

// DTLS连接建立后
sctp.TransportConnected();

// 发送DataChannel消息
SctpStreamParameters params;
params.streamId = 0;
params.ordered = true;
params.maxRetransmits = 0;  // 可靠传输
sctp.SendSctpMessage(params, PPID_WEBRTC_STRING, data, len);

// 接收DTLS传来的SCTP数据
sctp.ProcessSctpData(data, len);
```

### 5. STUN - NAT穿透

Stun 类用于处理 STUN 协议消息，实现 ICE 连接建立和 NAT 穿透。

**主要功能：**
- STUN Binding Request/Response
- XOR-MAPPED-ADDRESS 编码
- MESSAGE-INTEGRITY 校验
- ICE 用户名验证

**STUN 消息格式：**

```
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
```

**ICE 连接流程：**

```
Client                          Server
  |                                |
  | STUN Binding Request          |
  |------------------------------>|
  | (Username, Priority, ICE-CONTROLLING) |
  |                                |
  |        STUN Binding Response  |
  |<------------------------------|
  | (XOR-MAPPED-ADDRESS, MESSAGE-INTEGRITY) |
  |                                |
  | ICE连接建立                     |
  |<=============================>|
```

**使用示例：**

```cpp
Stun stun;

// 解析STUN请求
if (stun.Decode(data, size)) {
    std::string ufrag = stun.LocalUFrag();
    
    // 验证用户名
    if (ValidateUFrag(ufrag)) {
        // 生成STUN响应
        stun.SetPassword("ice_password");
        stun.SetMappedAddr(client_ip);
        stun.SetMappedPort(client_port);
        stun.SetMessageType(kStunMsgBindingResponse);
        
        rtc::Buffer response = stun.Encode();
        // 发送响应
        SendPacket(response.data(), response.size());
    }
}
```

### 6. RtcSdp - SDP协议

RtcSdp 类用于处理 WebRTC 的 SDP 协议，包括 SDP 的解析和生成。

**主要功能：**
- SDP Offer 解析
- SDP Answer 生成
- 媒体格式协商
- ICE/DTLS 参数管理

**SDP 结构：**

```
v=0                                    # 版本
o=- 123456 2 IN IP4 127.0.0.1         # 会话源
s=-                                    # 会话名称
t=0 0                                  # 时间描述
a=group:BUNDLE 0 1                     # 媒体捆绑
a=msid-semantic: WMS stream            # 媒体流语义

m=audio 9 UDP/TLS/RTP/SAVPF 111       # 音频媒体描述
c=IN IP4 0.0.0.0                       # 连接信息
a=rtcp:9 IN IP4 0.0.0.0               # RTCP端口
a=ice-ufrag:abcd                       # ICE用户名片段
a=ice-pwd:1234567890abcdef            # ICE密码
a=fingerprint:sha-256 AB:CD:EF:...    # DTLS证书指纹
a=setup:actpass                        # DTLS角色
a=mid:0                                # 媒体ID
a=sendrecv                             # 媒体方向
a=rtcp-mux                             # RTCP复用
a=rtpmap:111 opus/48000/2             # RTP负载类型映射
a=ssrc:12345678 cname:stream          # SSRC标识

m=video 9 UDP/TLS/RTP/SAVPF 96        # 视频媒体描述
c=IN IP4 0.0.0.0
a=rtcp:9 IN IP4 0.0.0.0
a=ice-ufrag:abcd
a=ice-pwd:1234567890abcdef
a=fingerprint:sha-256 AB:CD:EF:...
a=setup:actpass
a=mid:1
a=sendrecv
a=rtcp-mux
a=rtpmap:96 H264/90000                # H.264编码
a=fmtp:96 profile-level-id=42e01f    # H.264参数
a=ssrc:87654321 cname:stream
a=ssrc-group:FID 87654321 87654322   # RTX分组
```

**使用示例：**

```cpp
RtcSdp sdp;

// 解析Offer
std::string offer = "v=0\r\no=- ...";
sdp.SetSdpType(kRtcSdpPlay);
if (sdp.Decode(offer)) {
    // 设置本地参数
    sdp.SetLocalUFrag("abcd");
    sdp.SetLocalPasswd("1234567890abcdef");
    sdp.SetLocalFingerprint(fingerprints);
    sdp.SetVideoSsrc(12345678);
    sdp.SetAudioSsrc(87654321);
    
    // 生成Answer
    std::string answer = sdp.Encode();
    
    // 发送Answer给客户端
    SendSdpAnswer(answer);
}
```

### 7. DtlsCerts - 证书管理

DtlsCerts 类用于管理 DTLS 证书和私钥，采用单例模式。

**主要功能：**
- 证书和私钥生成
- 证书指纹计算
- SSL 上下文管理
- 证书文件加载

**证书指纹算法：**

| 算法 | 长度 | 说明 |
|------|------|------|
| SHA-1 | 160位 | 不推荐，安全性低 |
| SHA-224 | 224位 | 较少使用 |
| SHA-256 | 256位 | 推荐使用 |
| SHA-384 | 384位 | 高安全性 |
| SHA-512 | 512位 | 最高安全性 |

**使用示例：**

```cpp
// 获取单例实例
auto& dtls_certs = DtlsCerts::GetInstance();

// 初始化（自动生成证书）
dtls_certs.Init();

// 或从文件加载
dtls_certs.Init("cert.pem", "key.pem");

// 获取指纹
auto fingerprints = dtls_certs.Fingerprints();
for (const auto& fp : fingerprints) {
    std::string algo = DtlsCerts::GetFingerprintAlgorithmString(fp.algorithm);
    std::cout << "Fingerprint (" << algo << "): " << fp.value << std::endl;
}

// 获取SSL上下文
SSL_CTX* ctx = dtls_certs.GetSslCtx();
```

## WebRTC 连接建立流程

完整的 WebRTC 连接建立流程包括以下步骤：

```
1. SDP交换（Signaling）
   Client                          Server
     |                                |
     | SDP Offer                     |
     |------------------------------>|
     |                                |
     |              SDP Answer       |
     |<------------------------------|

2. ICE连接建立（Connectivity Check）
   Client                          Server
     |                                |
     | STUN Binding Request          |
     |------------------------------>|
     |                                |
     |        STUN Binding Response  |
     |<------------------------------|
     |                                |
     | ICE连接建立                     |
     |<=============================>|

3. DTLS握手（Key Exchange）
   Client                          Server
     |                                |
     | DTLS ClientHello              |
     |------------------------------>|
     |                                |
     |    DTLS ServerHello, Cert     |
     |<------------------------------|
     |                                |
     | DTLS Finished                 |
     |------------------------------>|
     |                                |
     |          DTLS Finished        |
     |<------------------------------|
     |                                |
     | Extract SRTP Keys             |
     |<=============================>|

4. SRTP媒体传输（Media Transport）
   Client                          Server
     |                                |
     | SRTP Packets (Audio/Video)    |
     |<=============================>|
     |                                |
     | SRTCP Packets (Control)       |
     |<=============================>|

5. SCTP数据通道（Data Channel，可选）
   Client                          Server
     |                                |
     | SCTP INIT                     |
     |------------------------------>|
     |                                |
     |              SCTP INIT-ACK    |
     |<------------------------------|
     |                                |
     | DataChannel Messages          |
     |<=============================>|
```

## 使用场景

### 场景1：WebRTC 推流服务器

```cpp
class WebRTCPublisher {
public:
    void Start() {
        // 1. 启动RTC服务器
        rtc_server_.Start("0.0.0.0", 10000);
        
        // 2. 连接信号
        rtc_server_.SignalStunPacket.connect(this, &WebRTCPublisher::OnStunPacket);
        rtc_server_.SignalDtlsPacket.connect(this, &WebRTCPublisher::OnDtlsPacket);
        rtc_server_.SignalRtpPacket.connect(this, &WebRTCPublisher::OnRtpPacket);
    }
    
    void OnStunPacket(auto socket, auto data, auto len, auto addr, auto ts) {
        // 处理STUN，建立ICE连接
        stun_.Decode(data, len);
        // 生成响应...
    }
    
    void OnDtlsPacket(auto socket, auto data, auto len, auto addr, auto ts) {
        // 处理DTLS，建立加密会话
        dtls_.OnRecv(data, len);
    }
    
    void OnRtpPacket(auto socket, auto data, auto len, auto addr, auto ts) {
        // 解密RTP数据包
        uint8_t* rtp_data = const_cast<uint8_t*>(data);
        size_t rtp_len = len;
        if (srtp_recv_->DecryptSrtp(rtp_data, &rtp_len)) {
            // 处理音视频数据
            ProcessMediaData(rtp_data, rtp_len);
        }
    }

private:
    RtcServer rtc_server_;
    Stun stun_;
    Dtls dtls_;
    SrtpSession* srtp_recv_;
};
```

### 场景2：WebRTC 拉流服务器

```cpp
class WebRTCPlayer {
public:
    void SendMedia(const uint8_t* data, size_t len) {
        // 加密RTP数据包
        const uint8_t* rtp_data = data;
        size_t rtp_len = len;
        if (srtp_send_->EncryptRtp(&rtp_data, &rtp_len)) {
            // 发送SRTP数据包
            rtc_server_.SendRtpPacketTo(
                rtc::CopyOnWriteBuffer(rtp_data, rtp_len),
                client_addr_,
                rtc::PacketOptions()
            );
        }
    }

private:
    RtcServer rtc_server_;
    SrtpSession* srtp_send_;
    rtc::SocketAddress client_addr_;
};
```

### 场景3：WebRTC 数据通道

```cpp
class WebRTCDataChannel : public SctpAssociation::Listener {
public:
    void SendMessage(const std::string& message) {
        SctpStreamParameters params;
        params.streamId = 0;
        params.ordered = true;
        
        sctp_.SendSctpMessage(
            params,
            PPID_WEBRTC_STRING,
            reinterpret_cast<const uint8_t*>(message.data()),
            message.size()
        );
    }
    
    void OnSctpAssociationMessageReceived(
        SctpAssociation* sctp, uint16_t streamId,
        uint32_t ppid, const uint8_t* msg, size_t len) override {
        // 接收到DataChannel消息
        std::string message(reinterpret_cast<const char*>(msg), len);
        std::cout << "Received: " << message << std::endl;
    }

private:
    SctpAssociation sctp_;
};
```

## 性能优化建议

### 1. 数据包处理优化

- 使用零拷贝技术（CopyOnWriteBuffer）
- 批量发送RTP数据包
- 避免频繁的内存分配

```cpp
// 批量发送RTP数据包
std::vector<std::unique_ptr<RtpPacketToSend>> packets;
// ... 填充packets
rtc_server_.SendRtpPacketTo(std::move(packets), addr, options);
```

### 2. 加密性能优化

- 使用 AES-GCM 加密套件（硬件加速）
- 复用 SRTP 会话
- 避免重复的密钥派生

```cpp
// 推荐使用AES-GCM
SrtpSession* session = new SrtpSession(
    OUTBOUND,
    CryptoSuite::AEAD_AES_128_GCM,  // 硬件加速
    key,
    28
);
```

### 3. 线程模型优化

- 网络操作在 network_thread 执行
- 媒体处理在 worker_thread 执行
- 信令处理在 signaling_thread 执行

```cpp
// 在正确的线程执行操作
network_thread()->PostTask([this]() {
    // 网络操作
    SendPacket(data, len);
});
```

## 注意事项

1. **线程安全**
   - 所有网络操作必须在 network_thread 执行
   - DTLS 操作在独立的任务队列执行
   - 使用信号槽机制进行线程间通信

2. **证书管理**
   - 证书指纹必须在 SDP 中交换
   - DTLS 握手时验证远程证书指纹
   - 证书有效期默认365天

3. **密钥安全**
   - SRTP 密钥通过 DTLS 协商导出
   - 发送和接收使用不同的密钥
   - 密钥材料不应明文存储

4. **NAT 穿透**
   - 使用 STUN 获取公网地址
   - ICE 连接性检查确保连通性
   - 支持 TURN 中继（需要额外实现）

5. **数据通道**
   - SCTP 运行在 DTLS 之上
   - 支持多流复用（最多65535个流）
   - 可配置可靠性和有序性

## 参考资料

- [RFC 3550 - RTP](https://tools.ietf.org/html/rfc3550)
- [RFC 3711 - SRTP](https://tools.ietf.org/html/rfc3711)
- [RFC 4571 - RTP over TCP](https://tools.ietf.org/html/rfc4571)
- [RFC 5245 - ICE](https://tools.ietf.org/html/rfc5245)
- [RFC 5389 - STUN](https://tools.ietf.org/html/rfc5389)
- [RFC 5764 - DTLS-SRTP](https://tools.ietf.org/html/rfc5764)
- [RFC 6347 - DTLS](https://tools.ietf.org/html/rfc6347)
- [RFC 8831 - WebRTC Data Channels](https://tools.ietf.org/html/rfc8831)
- [RFC 8832 - WebRTC Data Channel Protocol](https://tools.ietf.org/html/rfc8832)
- [WebRTC 官方文档](https://webrtc.org/)
