# libhttp - HTTP 协议实现库

## 概述

libhttp 是一个完整的 HTTP/1.1 协议实现库，支持 HTTP 服务器和客户端功能，包括普通传输、分块传输和流式传输。

## 核心功能

### 1. HTTP 协议支持
- **HTTP/1.0 和 HTTP/1.1**：支持两个版本的 HTTP 协议
- **请求方法**：GET、POST、PUT、DELETE、HEAD、OPTIONS、PATCH
- **状态码**：完整的 1xx-5xx 状态码支持
- **头部管理**：添加、删除、查询 HTTP 头部字段

### 2. 传输模式
- **普通传输**：使用 Content-Length 指定长度
- **分块传输**：使用 Transfer-Encoding: chunked
- **流式传输**：持续传输数据，无固定长度

### 3. HTTP 服务器
- **多连接管理**：支持多个并发连接
- **请求解析**：解析 HTTP 请求消息
- **响应发送**：发送 HTTP 响应消息
- **CORS 支持**：跨域资源共享

### 4. 工具函数
- **URL 编解码**：URL 百分号编码和解码
- **状态码转换**：状态码与消息的转换
- **Content-Type 处理**：MIME 类型管理

## 文件结构

```
libhttp/
├── README.md              # 本文件
├── http_type.h            # HTTP 类型定义
├── http_utils.h/cpp       # HTTP 工具函数
├── http_request.h/cpp     # HTTP 请求/响应类
├── http_parser.h/cpp      # HTTP 解析器
├── http_context.h/cpp     # HTTP 上下文
├── http_server.h/cpp      # HTTP 服务器
├── msg_buffer.h/cpp       # 消息缓冲区
└── packet.h/cpp           # 数据包封装
```

## HTTP 消息格式

### HTTP 请求格式
```
┌─────────────────────────────────────┐
│  请求行                             │
│  GET /api/users?id=123 HTTP/1.1     │
├─────────────────────────────────────┤
│  头部字段                           │
│  Host: example.com                  │
│  Content-Type: application/json     │
│  Content-Length: 123                │
├─────────────────────────────────────┤
│  空行 (\r\n)                        │
├─────────────────────────────────────┤
│  消息体（可选）                     │
│  {"key": "value"}                   │
└─────────────────────────────────────┘
```

### HTTP 响应格式
```
┌─────────────────────────────────────┐
│  状态行                             │
│  HTTP/1.1 200 OK                    │
├─────────────────────────────────────┤
│  头部字段                           │
│  Content-Type: application/json     │
│  Content-Length: 123                │
├─────────────────────────────────────┤
│  空行 (\r\n)                        │
├─────────────────────────────────────┤
│  消息体                             │
│  {"status": "ok"}                   │
└─────────────────────────────────────┘
```

### 分块传输格式
```
HTTP/1.1 200 OK
Transfer-Encoding: chunked

5\r\n
Hello\r\n
6\r\n
 World\r\n
0\r\n
\r\n
```

## 核心类说明

### HttpServer
HTTP 服务器类，管理 TCP 连接和 HTTP 协议处理。

**使用示例：**
```cpp
HttpServer server;

// 连接请求处理信号
server.SignalOnRequest.connect([](Connection* conn, 
                                   const HttpRequestPtr& req, 
                                   const PacketPtr& packet) {
    // 处理请求
    if (req->Path() == "/api/test") {
        auto response = std::make_shared<HttpRequest>(false);
        response->SetStatusCode(200);
        response->AddHeader("Content-Type", "application/json");
        response->SetBody("{\"status\":\"ok\"}");
        
        auto context = conn->GetContext<HttpContext>(kHttpContext);
        context->PostRequest(response);
    }
});

// 启动服务器
server.Startup("0.0.0.0", 8080);
```

### HttpRequest
HTTP 请求/响应类，封装 HTTP 消息。

**使用示例：**
```cpp
// 创建请求
auto request = std::make_shared<HttpRequest>(true);
request->SetMethod(kGet);
request->SetPath("/api/users");
request->SetParameter("id", "123");
request->AddHeader("Host", "example.com");

// 创建响应
auto response = std::make_shared<HttpRequest>(false);
response->SetStatusCode(200);
response->AddHeader("Content-Type", "application/json");
response->SetBody("{\"status\":\"ok\"}");
```

### HttpParser
HTTP 解析器类，解析 HTTP 消息流。

**使用示例：**
```cpp
HttpParser parser;
MsgBuffer buffer;

// 接收数据
buffer.Append(tcp_data, tcp_data_len);

// 解析
HttpParserState state = parser.Parse(buffer);

if (state == kExpectHttpComplete) {
    auto request = parser.GetHttpRequest();
    auto packet = parser.Chunk();
    // 处理请求...
}
```

### HttpContext
HTTP 上下文类，管理单个连接的 HTTP 协议处理。

**使用示例：**
```cpp
auto context = std::make_shared<HttpContext>(connection);

// 解析请求
MsgBuffer buffer;
buffer.Append(data, len);
context->Parse(buffer);

// 发送响应
auto response = std::make_shared<HttpRequest>(false);
response->SetStatusCode(200);
context->PostRequest(response);
```

### MsgBuffer
消息缓冲区类，管理接收和发送缓冲区。

**使用示例：**
```cpp
MsgBuffer buffer(1024);

// 追加数据
buffer.Append("GET / HTTP/1.1\r\n", 16);

// 查找 CRLF
const char* crlf = buffer.FindCRLF();

// 读取数据
std::string line = buffer.Read(crlf - buffer.Peek());
buffer.RetrieveUntil(crlf + 2);
```

### Packet
数据包类，封装 HTTP 消息体数据。

**使用示例：**
```cpp
auto packet = Packet::NewPacket(1024);

// 写入数据
memcpy(packet->Data(), data, data_len);
packet->SetPacketSize(data_len);

// 设置类型
packet->SetPacketType(kPacketTypeVideo);
packet->SetTimeStamp(timestamp);
```

## 使用场景

### 1. HTTP API 服务器
```cpp
HttpServer server;

server.SignalOnRequest.connect([](Connection* conn, 
                                   const HttpRequestPtr& req, 
                                   const PacketPtr& packet) {
    if (req->Method() == kPost && req->Path() == "/api/login") {
        // 解析 JSON 请求体
        std::string body = req->Body();
        auto json = ParseJson(body);
        
        // 处理登录逻辑
        bool success = Login(json["username"], json["password"]);
        
        // 构造响应
        auto response = std::make_shared<HttpRequest>(false);
        response->SetStatusCode(success ? 200 : 401);
        response->AddHeader("Content-Type", "application/json");
        response->SetBody(success ? 
            "{\"status\":\"ok\"}" : 
            "{\"error\":\"Invalid credentials\"}");
        
        auto context = conn->GetContext<HttpContext>(kHttpContext);
        context->PostRequest(response);
    }
});

server.Startup("0.0.0.0", 8080);
```

### 2. HTTP-FLV 流媒体服务器
```cpp
HttpServer server;

server.SignalOnRequest.connect([](Connection* conn, 
                                   const HttpRequestPtr& req, 
                                   const PacketPtr& packet) {
    if (req->Path().find(".flv") != std::string::npos) {
        // 发送 FLV 头部
        auto response = std::make_shared<HttpRequest>(false);
        response->SetStatusCode(200);
        response->AddHeader("Content-Type", "video/x-flv");
        response->SetIsStream(true);
        
        auto context = conn->GetContext<HttpContext>(kHttpContext);
        context->PostStreamHeader(response->MakeHeaders());
        
        // 持续发送 FLV 数据
        StartStreamingFlv(conn);
    }
});

server.Startup("0.0.0.0", 8080);
```

### 3. 文件下载服务器
```cpp
HttpServer server;

server.SignalOnRequest.connect([](Connection* conn, 
                                   const HttpRequestPtr& req, 
                                   const PacketPtr& packet) {
    std::string file_path = GetFilePath(req->Path());
    
    if (FileExists(file_path)) {
        // 读取文件
        auto file_data = ReadFile(file_path);
        
        // 构造响应
        auto response = std::make_shared<HttpRequest>(false);
        response->SetStatusCode(200);
        response->AddHeader("Content-Type", GetContentType(file_path));
        response->AddHeader("Content-Length", std::to_string(file_data.size()));
        response->SetBody(file_data);
        
        auto context = conn->GetContext<HttpContext>(kHttpContext);
        context->PostRequest(response);
    } else {
        // 404 响应
        auto response = HttpRequest::NewHttp404Response();
        auto context = conn->GetContext<HttpContext>(kHttpContext);
        context->PostRequest(response);
    }
});

server.Startup("0.0.0.0", 8080);
```

## 性能优化

### 1. 连接复用
支持 HTTP/1.1 持久连接（Keep-Alive）。

### 2. 零拷贝
使用指针操作，避免不必要的数据拷贝。

### 3. 内存池
使用内存池管理 Packet 对象。

### 4. 批量发送
批量发送多个响应，减少系统调用。

## 注意事项

1. **线程安全**：所有网络操作在 network_thread 中执行
2. **内存管理**：使用智能指针管理对象生命周期
3. **错误处理**：正确处理解析错误和网络错误
4. **CORS**：根据需要配置跨域策略
5. **超时处理**：设置合理的连接超时时间

## 参考资料

- [RFC 7230 - HTTP/1.1: Message Syntax and Routing](https://tools.ietf.org/html/rfc7230)
- [RFC 7231 - HTTP/1.1: Semantics and Content](https://tools.ietf.org/html/rfc7231)
- [RFC 7232 - HTTP/1.1: Conditional Requests](https://tools.ietf.org/html/rfc7232)
- [RFC 7233 - HTTP/1.1: Range Requests](https://tools.ietf.org/html/rfc7233)
- [RFC 7234 - HTTP/1.1: Caching](https://tools.ietf.org/html/rfc7234)
- [RFC 7235 - HTTP/1.1: Authentication](https://tools.ietf.org/html/rfc7235)

## 作者

chensong - 2025

## 许可证

BSD-style license
