# libnetwork - 网络传输层库

## 概述

libnetwork 是一个统一的网络传输层库，提供了 TCP 和 UDP 协议的封装和管理。它为上层应用提供了简洁、高效、线程安全的网络通信接口，支持多种流媒体协议（HTTP、RTMP、WebRTC、GB28181 等）。

**核心特性：**
- 统一的 TCP/UDP 连接抽象
- 基于信号槽的事件驱动模型
- 多协议上下文管理
- 线程安全的异步操作
- 高性能的数据收发

## 文件结构

```
libnetwork/
├── connection.h/cpp          # 连接抽象类（统一TCP/UDP接口）
├── tcp_server.h/cpp          # TCP服务器（支持多客户端）
├── udp_server.h/cpp          # UDP服务器（RTP/RTCP传输）
├── tcp_session.h/cpp         # TCP会话类（已废弃，使用Connection代替）
└── README.md                 # 本文档
```

## 核心类说明

### 1. Connection - 连接抽象类

Connection 类提供了 TCP 和 UDP 连接的统一抽象接口，屏蔽了底层传输协议的差异。

**主要功能：**
- 统一的数据收发接口
- 多类型上下文管理
- 异步数据发送
- 基于信号槽的事件通知

**连接生命周期：**

```
创建连接
   ↓
初始化Socket信号
   ↓
┌─────────────┐
│  连接活跃   │ ←──┐
└─────────────┘    │
   ↓               │
数据收发           │
   ↓               │
触发信号 ──────────┘
   ↓
关闭连接
   ↓
清理资源
```

**使用示例：**

```cpp
// TCP连接示例
rtc::Socket* tcp_socket = ...;
Connection* conn = new Connection(network_thread, tcp_socket);

// 设置HTTP上下文
auto http_ctx = std::make_shared<HttpContext>();
conn->SetContext(kHttpContext, http_ctx);

// 监听数据接收
conn->SignalOnRecv.connect([](Connection* c, const rtc::CopyOnWriteBuffer& data) {
    // 处理接收到的数据
    ProcessData(data);
});

// 异步发送数据
rtc::CopyOnWriteBuffer send_data = ...;
conn->AsyncSend(std::move(send_data));

// UDP连接示例
rtc::AsyncPacketSocket* udp_socket = ...;
rtc::SocketAddress remote_addr("192.168.1.100", 5000);
Connection* udp_conn = new Connection(network_thread, udp_socket, remote_addr);

// 发送RTP数据包
udp_conn->AsyncSend(std::move(rtp_packet));
```

### 2. TcpServer - TCP服务器

TcpServer 提供了完整的 TCP 服务器功能，支持多客户端连接管理。

**主要功能：**
- 监听端口，接受连接
- 管理多个客户端连接
- 自动的连接生命周期管理
- 服务器级别的上下文共享

**服务器生命周期：**

```
Startup()
   ↓
创建监听Socket
   ↓
Bind + Listen
   ↓
┌──────────────┐
│  等待连接    │
└──────────────┘
   ↓
OnRead() - Accept新连接
   ↓
创建Connection对象
   ↓
触发SignalOnNewConnection
   ↓
┌──────────────┐
│  数据收发    │ ←──┐
└──────────────┘    │
   ↓               │
OnSessionRecv()    │
   ↓               │
触发SignalOnRecv ──┘
   ↓
OnSessionClose()
   ↓
触发SignalOnDestory
   ↓
清理Connection
```

**使用示例：**

```cpp
// 创建TCP服务器
TcpServer* server = new TcpServer();

// 监听新连接
server->SignalOnNewConnection.connect([](Connection* conn) {
    std::cout << "New client connected" << std::endl;
    
    // 设置HTTP上下文
    auto http_ctx = std::make_shared<HttpContext>();
    conn->SetContext(kHttpContext, http_ctx);
});

// 监听数据接收
server->SignalOnRecv.connect([](Connection* conn, const rtc::CopyOnWriteBuffer& data) {
    // 处理接收到的数据
    auto http_ctx = conn->GetContext<HttpContext>(kHttpContext);
    http_ctx->ProcessData(data);
});

// 监听连接关闭
server->SignalOnDestory.connect([](Connection* conn) {
    std::cout << "Client disconnected" << std::endl;
});

// 启动服务器
if (server->Startup("0.0.0.0", 8080)) {
    std::cout << "Server started on port 8080" << std::endl;
}

// 关闭指定连接
server->CloseSession(conn);
```

### 3. UdpServer - UDP服务器

UdpServer 专门用于 UDP 数据包的收发，主要用于 RTP/RTCP 媒体传输。

**主要功能：**
- UDP 数据包收发
- RTP/RTCP 数据包发送
- 支持异步和同步两种模式
- 高性能的实时传输

**UDP 数据包格式：**

```
+--------+--------+--------+--------+
|  IP头  | UDP头  |   数据负载      |
+--------+--------+--------+--------+
| 20字节 | 8字节  |   N字节         |
+--------+--------+--------+--------+
```

**RTP 数据包格式：**

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
|            contributing source (CSRC) identifiers             |
|                             ....                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**使用示例：**

```cpp
// 创建UDP服务器
UdpServer* server = new UdpServer();

// 监听数据接收
server->SignalReadPacket.connect([](rtc::AsyncPacketSocket* socket,
                                     const uint8_t* data,
                                     size_t len,
                                     const rtc::SocketAddress& addr,
                                     const int64_t& timestamp) {
    // 处理接收到的数据包
    if (IsRtpPacket(data, len)) {
        ProcessRtpPacket(data, len, addr);
    } else if (IsRtcpPacket(data, len)) {
        ProcessRtcpPacket(data, len, addr);
    }
});

// 启动服务器
if (server->Startup("0.0.0.0", 5000)) {
    std::cout << "UDP server started on port 5000" << std::endl;
}

// 发送RTP数据包
rtc::CopyOnWriteBuffer rtp_packet = ...;
rtc::SocketAddress client_addr("192.168.1.100", 6000);
server->SendRtpPacketTo(std::move(rtp_packet), client_addr, rtc::PacketOptions());

// 发送RTCP数据包
rtc::CopyOnWriteBuffer rtcp_packet = ...;
server->SendRtcpPacketTo(std::move(rtcp_packet), client_addr, rtc::PacketOptions());
```

## 上下文类型

libnetwork 支持多种协议上下文，每个连接可以同时持有多个不同类型的上下文对象：

```cpp
enum {
    kNormalContext = 0,        // 普通上下文（默认）
    kRtmpContext,              // RTMP协议上下文
    kHttpContext,              // HTTP协议上下文
    kShareResourceContext,     // 共享资源上下文
    kFlvContext,               // FLV格式上下文
    kRtcContext,               // WebRTC协议上下文
    kGb28181Context,           // GB28181协议上下文
};
```

**上下文管理示例：**

```cpp
// 设置上下文
auto http_ctx = std::make_shared<HttpContext>();
conn->SetContext(kHttpContext, http_ctx);

// 获取上下文
auto http_ctx = conn->GetContext<HttpContext>(kHttpContext);
if (http_ctx) {
    http_ctx->ProcessRequest();
}

// 清除上下文
conn->ClearContext(kHttpContext);

// 清除所有上下文
conn->ClearContext();
```

## TCP vs UDP 对比

| 特性 | TCP | UDP |
|------|-----|-----|
| 连接方式 | 面向连接 | 无连接 |
| 可靠性 | 可靠传输 | 不可靠传输 |
| 顺序保证 | 保证顺序 | 不保证顺序 |
| 速度 | 较慢 | 快速 |
| 开销 | 较大 | 较小 |
| 适用场景 | 文件传输、HTTP、RTMP | 实时音视频、RTP/RTCP |
| 流量控制 | 支持 | 不支持 |
| 拥塞控制 | 支持 | 不支持 |

## GB28181 RTP over TCP

GB28181 使用 RFC4571 定义的 RTP over TCP 格式，与 RTSP（RFC2326）不同：

**RFC2326 格式（RTSP）：**
```
+--------+--------+--------+--------+
| magic  |channel | length |  data  |
| number | number |        |        |
+--------+--------+--------+--------+
| 1 byte | 1 byte | 2 bytes| N bytes|
+--------+--------+--------+--------+
```

**RFC4571 格式（GB28181）：**
```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+---------------------------------------------------------------+
|             LENGTH            |  RTP or RTCP packet ...       |
+---------------------------------------------------------------+
```

**区别：**
- RFC2326 使用 channel number 区分信令和媒体数据
- RFC4571 不需要 channel number，因为信令和媒体使用不同的 TCP 连接

## 线程模型

libnetwork 使用三线程模型：

```
┌─────────────────┐
│ Signaling Thread│  信令线程（处理信令消息）
└─────────────────┘
         ↓
┌─────────────────┐
│  Worker Thread  │  工作线程（处理编解码等）
└─────────────────┘
         ↓
┌─────────────────┐
│ Network Thread  │  网络线程（处理所有网络IO）
└─────────────────┘
```

**重要规则：**
- 所有网络操作必须在 network_thread 中执行
- AsyncSend 会自动切换到网络线程
- 信号回调可能在不同线程触发，需要注意线程安全

## 性能优化建议

### TCP 优化

1. **使用异步发送**
   ```cpp
   // 推荐：异步发送（线程安全）
   conn->AsyncSend(std::move(data));
   
   // 不推荐：同步发送（可能阻塞）
   conn->AyncSend(data.data(), data.size());
   ```

2. **批量处理数据**
   ```cpp
   // 一次性读取所有可用数据
   do {
       int bytes = socket->Recv(buffer + offset, capacity - offset, nullptr);
       if (bytes <= 0) break;
       offset += bytes;
   } while (offset < capacity);
   ```

3. **使用移动语义**
   ```cpp
   // 避免数据拷贝
   conn->AsyncSend(std::move(data));
   ```

### UDP 优化

1. **使用异步模式**
   ```cpp
   // 编译时启用异步模式
   #define ASYNC_UDP 1
   ```

2. **控制数据包大小**
   ```cpp
   // 单个UDP数据包不超过MTU
   const size_t MAX_UDP_PACKET_SIZE = 1472;  // 1500 - 20(IP) - 8(UDP)
   ```

3. **批量发送RTP包**
   ```cpp
   // 批量发送多个RTP数据包
   std::vector<std::unique_ptr<RtpPacketToSend>> packets;
   // ... 填充packets
   server->SendRtpPacketTo(std::move(packets), addr, options);
   ```

## 注意事项

1. **线程安全**
   - 所有网络操作必须在 network_thread 中执行
   - 使用 AsyncSend 保证线程安全
   - 信号回调可能在不同线程触发

2. **资源管理**
   - Connection 对象由 TcpServer 管理生命周期
   - 使用 unique_ptr 避免内存泄漏
   - 连接关闭时会自动清理资源

3. **UDP 特性**
   - UDP 不保证数据包送达
   - UDP 不保证数据包顺序
   - 需要上层协议处理丢包和乱序

4. **TCP 特性**
   - TCP 保证数据可靠送达
   - TCP 保证数据顺序
   - 可能出现粘包和拆包问题

5. **上下文管理**
   - 使用 shared_ptr 管理上下文生命周期
   - 支持多种协议上下文共存
   - 服务器级别的上下文会自动传递给新连接

## 使用场景

### HTTP 服务器
```cpp
TcpServer* http_server = new TcpServer();
http_server->SignalOnNewConnection.connect([](Connection* conn) {
    auto http_ctx = std::make_shared<HttpContext>();
    conn->SetContext(kHttpContext, http_ctx);
});
http_server->Startup("0.0.0.0", 8080);
```

### RTMP 推流服务器
```cpp
TcpServer* rtmp_server = new TcpServer();
rtmp_server->SignalOnNewConnection.connect([](Connection* conn) {
    auto rtmp_ctx = std::make_shared<RtmpContext>();
    conn->SetContext(kRtmpContext, rtmp_ctx);
});
rtmp_server->Startup("0.0.0.0", 1935);
```

### WebRTC 媒体服务器
```cpp
UdpServer* rtp_server = new UdpServer();
rtp_server->SignalReadPacket.connect([](auto socket, auto data, auto len, auto addr, auto ts) {
    if (IsRtpPacket(data, len)) {
        ProcessRtpPacket(data, len, addr);
    }
});
rtp_server->Startup("0.0.0.0", 5000);
```

### GB28181 视频监控
```cpp
// TCP 模式
TcpServer* gb_tcp_server = new TcpServer();
gb_tcp_server->SignalOnNewConnection.connect([](Connection* conn) {
    auto gb_ctx = std::make_shared<Gb28181Context>();
    conn->SetContext(kGb28181Context, gb_ctx);
});
gb_tcp_server->Startup("0.0.0.0", 28181);

// UDP 模式
UdpServer* gb_udp_server = new UdpServer();
gb_udp_server->SignalReadPacket.connect([](auto socket, auto data, auto len, auto addr, auto ts) {
    ProcessGb28181Packet(data, len, addr);
});
gb_udp_server->Startup("0.0.0.0", 28181);
```

## 参考资料

- [RFC 793 - TCP](https://tools.ietf.org/html/rfc793)
- [RFC 768 - UDP](https://tools.ietf.org/html/rfc768)
- [RFC 3550 - RTP](https://tools.ietf.org/html/rfc3550)
- [RFC 4571 - RTP over TCP](https://tools.ietf.org/html/rfc4571)
- [RFC 2326 - RTSP 1.0](https://tools.ietf.org/html/rfc2326)
- [GB/T 28181 - 公共安全视频监控联网系统信息传输、交换、控制技术要求](http://www.gb28181.org/)
