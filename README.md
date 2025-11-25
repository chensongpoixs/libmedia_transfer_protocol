# libmedia_transfer_protocol 媒体传输协议库

## 📖 项目简介

`libmedia_transfer_protocol` 是一个功能强大的媒体传输协议库，提供了完整的流媒体传输解决方案。该库支持多种主流的流媒体协议和封装格式，包括 RTSP、RTP/RTCP、HTTP、HLS、FLV、MPEG-TS、GB28181、SIP 等，适用于音视频流的推拉、分发和处理场景。

## ✨ 核心功能

### 1. 流媒体协议支持

#### ①、RTSP (Real-Time Streaming Protocol)
- ✅ RTSP 服务器和客户端实现
- ✅ 支持 OPTIONS、DESCRIBE、SETUP、PLAY、PAUSE、TEARDOWN 等方法
- ✅ RTP/RTCP 数据包解析和封装
- ✅ 支持 TCP 和 UDP 传输模式
- ✅ 支持 Interleaved 模式（RTSP over TCP）

#### ②、RTP/RTCP (Real-time Transport Protocol)
- ✅ RTP 数据包封装和解析
- ✅ RTCP 反馈包处理（SR、RR、SDES、BYE、APP 等）
- ✅ H.264/H.265 视频 RTP 打包
- ✅ AAC/MP3/G.711 音频 RTP 打包
- ✅ RTP 帧重组和参考帧查找
- ✅ 支持 VP8、VP9、H.264、H.265 等视频编码格式

#### ③、HTTP/HTTPS
- ✅ HTTP 服务器和客户端实现
- ✅ HTTP 请求/响应解析
- ✅ 支持 HTTP-FLV 流媒体传输
- ✅ URL 编码/解码支持
- ✅ Cookie 和 Session 管理

#### ④、HLS (HTTP Live Streaming)
- ✅ HLS M3U8 播放列表生成
- ✅ TS 切片管理和窗口滑动
- ✅ 支持直播和点播模式
- ✅ 自适应码率（ABR）支持

#### ⑤、FLV (Flash Video)
- ✅ FLV 文件封装
- ✅ 支持视频（H.264、H.265）和音频（AAC、MP3、G.711）
- ✅ Script Tag (onMetaData) 支持
- ✅ HTTP-FLV 流媒体传输

#### ⑥、MPEG-TS (Transport Stream)
- ✅ TS 流封装和解析
- ✅ PSI 表生成（PAT、PMT）
- ✅ PES 包封装
- ✅ 支持视频和音频复用
- ✅ PCR 时间戳同步

### 2. 网络层

#### TCP/UDP 服务器
- ✅ 高性能 TCP 服务器
- ✅ UDP 服务器支持
- ✅ 异步 I/O 处理
- ✅ 连接管理和会话处理
- ✅ 发送队列和流量控制

#### 网络控制
- ✅ 拥塞控制算法（GCC、BBR 等）
- ✅ 码率估计和自适应
- ✅ 数据包调度（Pacing）
- ✅ 网络状态预测

### 3. 视频监控协议

#### GB28181 (国标 28181)
- ✅ SIP 信令交互
- ✅ RTP 媒体传输
- ✅ 设备注册和目录查询
- ✅ 视频点播和回放
- ✅ 支持 TCP 和 UDP 传输

#### SIP (Session Initiation Protocol)
- ✅ SIP 消息解析和生成
- ✅ INVITE、ACK、BYE 等请求处理
- ✅ 200 OK、180 Ringing 等响应处理
- ✅ SDP 协商支持

### 4. WebRTC 支持

- ✅ RTC 传输层接口
- ✅ DTLS/SRTP 加密支持
- ✅ SCTP 数据通道
- ✅ ICE 候选收集
- ✅ 媒体流接口

### 5. 音视频处理

- ✅ 音视频帧缓冲
- ✅ 时间戳同步
- ✅ 码率控制
- ✅ 帧率控制

## 🏗️ 项目结构

```
libmedia_transfer_protocol/
├── libnetwork/              # 网络层
│   ├── tcp_server.h/cpp    # TCP 服务器
│   ├── udp_server.h/cpp    # UDP 服务器
│   └── connection.h/cpp    # 连接管理
├── libhttp/                # HTTP 协议
│   ├── http_server.h/cpp   # HTTP 服务器
│   ├── http_request.h/cpp  # HTTP 请求解析
│   └── http_session.h/cpp  # HTTP 会话
├── librtsp/                # RTSP 协议
│   ├── rtsp_server.h/cpp   # RTSP 服务器
│   └── rtsp_session.h/cpp  # RTSP 会话
├── librtp/                 # RTP 协议
│   ├── rtp_packetizer.h    # RTP 打包
│   └── rtp_depacketizer.h  # RTP 解包
├── librtcp/                # RTCP 协议
│   └── rtcp_packet.h/cpp   # RTCP 包处理
├── libhls/                 # HLS 支持
│   ├── chls_muxer.h/cpp    # HLS 复用器
│   ├── cfragment.h/cpp     # TS 切片
│   └── cfragment_window.h/cpp # 切片窗口
├── libflv/                 # FLV 支持
│   ├── cflv_encoder.h/cpp  # FLV 编码器
│   └── amf0.h/c            # AMF0 编码
├── libmpeg/                # MPEG-TS 支持
│   ├── cvideo_encoder.h/cpp # 视频编码器
│   ├── caudio_encoder.h/cpp # 音频编码器
│   ├── cpat_writer.h/cpp   # PAT 表写入
│   ├── cpmt_writer.h/cpp   # PMT 表写入
│   └── mpeg_decoder.h/cpp  # TS 流解析
├── libgb28181/             # GB28181 协议
│   ├── gb28181_server.h/cpp # GB28181 服务器
│   └── gb28181_session.h/cpp # GB28181 会话
├── libsip/                 # SIP 协议
│   └── sip_parser.h/cpp    # SIP 解析器
├── librtc/                 # WebRTC 接口
├── congestion_controller/  # 拥塞控制
├── pacing/                 # 数据包调度
├── remote_bitrate_estimator/ # 码率估计
└── crypto/                 # 加密支持
```

## 🚀 快速开始

### 环境要求

- **C++ 标准**: C++17 或更高版本
- **编译器**: GCC 7.0+ / Clang 8.0+ / MSVC 2017+
- **CMake**: 3.8 或更高版本
- **依赖库**:
  - WebRTC (libwebrtc)
  - libyuv
  - abseil-cpp
  - jsoncpp

### 编译步骤

1. **克隆项目**
   ```bash
   git clone https://github.com/chensongpoixs/libmedia_transfer_protocol.git
   cd libmedia_transfer_protocol
   ```

2. **配置 CMake**
   ```bash
   mkdir build
   cd build
   cmake .. \
     -DWEBRTC_ROOT=/path/to/webrtc \
     -DLIBYUV_ROOT=/path/to/libyuv \
     -DABSEIL_ROOT=/path/to/abseil \
     -DJSONCPP_ROOT=/path/to/jsoncpp
   ```

3. **编译**
   ```bash
   cmake --build . --config Release
   ```

### 使用示例

#### RTSP 服务器示例

```cpp
#include "libmedia_transfer_protocol/librtsp/rtsp_server.h"

using namespace libmedia_transfer_protocol;

// 创建 RTSP 服务器
RtspServer server;
server.Start("0.0.0.0", 554);

// 服务器会自动处理 RTSP 请求
// 客户端可以通过 rtsp://localhost:554/stream 访问
```

#### HTTP-FLV 流媒体示例

```cpp
#include "libmedia_transfer_protocol/libhttp/http_server.h"
#include "libmedia_transfer_protocol/libflv/cflv_encoder.h"

// 创建 HTTP 服务器
HttpServer http_server;
http_server.Start("0.0.0.0", 8080);

// 注册 FLV 流处理器
http_server.RegisterHandler("/live.flv", [](HttpRequest& req, HttpResponse& resp) {
    // 创建 FLV 编码器
    FlvEncoder encoder;
    
    // 处理视频帧
    encoder.WriteVideoFrame(h264_data, timestamp);
    
    // 发送 FLV 数据
    resp.SetBody(encoder.GetData());
});
```

#### HLS 流媒体示例

```cpp
#include "libmedia_transfer_protocol/libhls/chls_muxer.h"

// 创建 HLS 复用器
HLSMuxer hls_muxer;
hls_muxer.SetStreamName("live");
hls_muxer.SetMinFragmentSize(64 * 1024);  // 64KB
hls_muxer.SetMaxFragmentSize(2 * 1024 * 1024);  // 2MB

// 处理媒体包
hls_muxer.OnPacket(video_packet, timestamp);

// 获取 M3U8 播放列表
std::string playlist = hls_muxer.GetPlayList();
```

#### MPEG-TS 编码示例

```cpp
#include "libmedia_transfer_protocol/libmpeg/cvideo_encoder.h"
#include "libmedia_transfer_protocol/libmpeg/caudio_encoder.h"
#include "libmedia_transfer_protocol/libmpeg/cpat_writer.h"
#include "libmedia_transfer_protocol/libmpeg/cpmt_writer.h"

// 创建 TS 流写入器
StreamWriter writer;

// 写入 PAT 表
PatWriter pat_writer(0x0000, 0x1000);  // PAT PID=0x0000, PMT PID=0x1000
pat_writer.WritePat(&writer);

// 写入 PMT 表
PmtWriter pmt_writer(0x1000);
pmt_writer.AddProgramInfo(0x1001, 0x1B);  // 视频 PID=0x1001, H.264
pmt_writer.AddProgramInfo(0x1002, 0x0F);  // 音频 PID=0x1002, AAC
pmt_writer.WritePmt(&writer);

// 编码视频
VideoEncoder video_encoder;
video_encoder.SetPid(0x1001);
video_encoder.EncodeVideo(&writer, h264_data, pts, dts);

// 编码音频
AudioEncoder audio_encoder;
audio_encoder.SetPid(0x1002);
audio_encoder.EncodeAudio(&writer, aac_data, pts);
```

## 📚 模块详解

### 网络层 (libnetwork)

提供底层网络通信能力，包括 TCP 和 UDP 服务器的实现。

**主要类:**
- `TcpServer`: TCP 服务器
- `UdpServer`: UDP 服务器
- `Connection`: 连接管理
- `TcpSession`: TCP 会话

**特性:**
- 异步 I/O 处理
- 连接池管理
- 发送队列和流量控制
- 错误处理和重连机制

### HTTP 协议 (libhttp)

完整的 HTTP 协议实现，支持 HTTP 服务器和客户端功能。

**主要类:**
- `HttpServer`: HTTP 服务器
- `HttpRequest`: HTTP 请求解析
- `HttpResponse`: HTTP 响应生成
- `HttpSession`: HTTP 会话管理

**特性:**
- HTTP/1.1 支持
- 请求路由和处理器
- Cookie 和 Session 支持
- URL 编码/解码
- 支持 HTTP-FLV 流媒体

### RTSP 协议 (librtsp)

完整的 RTSP 协议实现，支持 RTSP 服务器和客户端。

**主要类:**
- `RtspServer`: RTSP 服务器
- `RtspClient`: RTSP 客户端
- `RtspSession`: RTSP 会话

**支持的方法:**
- OPTIONS
- DESCRIBE
- SETUP
- PLAY
- PAUSE
- TEARDOWN
- GET_PARAMETER

### RTP/RTCP (librtp/librtcp)

RTP 数据包封装和 RTCP 控制协议实现。

**主要类:**
- `RtpPacket`: RTP 数据包
- `RtpPacketizer`: RTP 打包器
- `RtpVideoFrameAssembler`: RTP 视频帧重组
- `RtcpPacket`: RTCP 控制包

**支持的功能:**
- H.264/H.265 RTP 打包
- AAC/MP3/G.711 音频打包
- RTCP SR/RR/SDES/BYE 包
- 丢包检测和重传请求

### HLS (libhls)

HTTP Live Streaming 支持，包括 TS 切片生成和 M3U8 播放列表管理。

**主要类:**
- `HLSMuxer`: HLS 复用器
- `Fragment`: TS 切片
- `FragmentWindow`: 切片窗口管理

**特性:**
- 自动切片生成
- 滑动窗口管理
- M3U8 播放列表生成
- 支持直播和点播模式
- 自适应码率（ABR）支持

### FLV (libflv)

Flash Video 格式支持，包括 FLV 文件封装和 HTTP-FLV 流媒体传输。

**主要类:**
- `FlvEncoder`: FLV 编码器
- `AMF0`: AMF0 数据编码

**特性:**
- H.264/H.265 视频封装
- AAC/MP3/G.711 音频封装
- Script Tag (onMetaData) 支持
- HTTP-FLV 流媒体传输

### MPEG-TS (libmpeg)

MPEG Transport Stream 支持，包括 TS 流封装和解析。

**主要类:**
- `VideoEncoder`: 视频编码器（TS 格式）
- `AudioEncoder`: 音频编码器（TS 格式）
- `PatWriter`: PAT 表写入
- `PmtWriter`: PMT 表写入
- `MpegDecoder`: TS 流解析器

**特性:**
- PSI 表生成（PAT、PMT）
- PES 包封装
- PCR 时间戳同步
- TS 包连续性计数
- 支持视频和音频复用

### GB28181 (libgb28181)

国标 28181 视频监控协议支持。

**主要类:**
- `Gb28181Server`: GB28181 服务器
- `Gb28181Session`: GB28181 会话

**特性:**
- SIP 信令交互
- RTP 媒体传输（TCP/UDP）
- 设备注册和目录查询
- 视频点播和回放
- 支持 RFC 4571 格式

### SIP (libsip)

Session Initiation Protocol 支持。

**主要类:**
- `SipParser`: SIP 消息解析器

**特性:**
- SIP 消息解析和生成
- INVITE、ACK、BYE 等请求处理
- 200 OK、180 Ringing 等响应处理
- SDP 协商支持

### 拥塞控制 (congestion_controller)

网络拥塞控制算法实现。

**主要算法:**
- GCC (Google Congestion Control)
- BBR (Bottleneck Bandwidth and Round-trip propagation time)
- 自适应码率控制

**特性:**
- 实时码率估计
- 延迟梯度检测
- 丢包率监控
- 自适应码率调整

### 数据包调度 (pacing)

数据包发送节奏控制。

**特性:**
- 平滑数据包发送
- 避免网络拥塞
- 提高传输效率

### 码率估计 (remote_bitrate_estimator)

远程码率估计算法。

**特性:**
- 基于延迟的码率估计
- 基于丢包的码率估计
- 自适应码率调整

## 📊 使用场景

### 1. 流媒体服务器

`libmedia_transfer_protocol` 可用于构建高性能的流媒体服务器：

```cpp
// 支持 RTSP、HTTP-FLV、HLS 等多种协议的流媒体服务器
```

### 2. 视频监控系统

支持 GB28181 协议，可用于构建视频监控平台：

```cpp
// GB28181 设备接入和管理
```

### 3. 直播推流

支持 RTSP 推流、HTTP-FLV 推流等：

```cpp
// 多协议推流支持
```

### 4. 点播系统

支持 HLS、FLV 等多种格式的点播：

```cpp
// 多格式点播支持
```

## 🔧 配置说明

### 编译选项

```cmake
# CMake 配置选项
option(ENABLE_RTSP "Enable RTSP support" ON)
option(ENABLE_HLS "Enable HLS support" ON)
option(ENABLE_FLV "Enable FLV support" ON)
option(ENABLE_GB28181 "Enable GB28181 support" ON)
option(ENABLE_WEBRTC "Enable WebRTC support" ON)
```

### 运行时配置

各模块支持通过配置文件或 API 进行运行时配置。

## 📖 API 文档

详细的 API 文档请参考：

- [网络层 API](docs/API_NETWORK.md)
- [HTTP API](docs/API_HTTP.md)
- [RTSP API](docs/API_RTSP.md)
- [RTP/RTCP API](docs/API_RTP.md)
- [HLS API](docs/API_HLS.md)
- [FLV API](docs/API_FLV.md)
- [MPEG-TS API](docs/API_MPEG.md)

## 🎯 性能优化

### 1. 多线程处理

库支持多线程并发处理，可充分利用多核 CPU：

```cpp
// 配置线程池大小
server.SetWorkerThreads(4);
```

### 2. 内存管理

- 使用对象池减少内存分配
- 使用零拷贝技术减少内存拷贝
- 及时释放不需要的资源

### 3. 网络优化

- 使用发送队列平滑发送
- 合理设置缓冲区大小
- 使用拥塞控制算法

## 🐛 故障排除

### 常见问题


1. **编译错误**
   - 确保安装了所有依赖库
   - 检查 CMake 配置路径是否正确
   - 确认编译器支持 C++17
   - 确保正确配置了 `WEBRTC_ROOT` 路径


2. **运行时错误**
   - 检查端口是否被占用
   - 确认网络权限配置
   - 查看日志输出
 
3. **HLS 切片生成失败**
   - 检查文件写入权限和磁盘空间

4. **性能问题**
   - 调整缓冲区大小
   - 优化数据包处理逻辑
   - 使用多线程处理

## 🤝 贡献指南

欢迎贡献代码！请遵循以下步骤：

1. Fork 项目
2. 创建功能分支 (`git checkout -b temp`)
3. 提交更改 (`git commit -m 'Add some New '`)
4. 推送到分支 (`git push origin temp`)
5. 创建 Pull Request

### 代码规范

- 使用 C++17 标准
- 遵循项目现有的代码风格
- 添加详细的 Doxygen 注释
- 确保代码通过编译和测试

## 📄 许可证

本项目采用 BSD 许可证，详情请查看 [LICENSE](LICENSE) 文件。

## 👥 作者

- [**chensong**](https://github.com/chensongpoixs) - 项目创建者和维护者

## 🙏 致谢

- Google WebRTC 团队
- 所有贡献者和用户

## 📚 相关文档

- [架构文档](ARCHITECTURE.md) - 详细的系统架构说明
- [开发指南](docs/DEVELOPMENT.md) - 开发指南
- [协议文档](docs/PROTOCOLS.md) - 支持的协议详细说明

## 🔮 未来计划

### 功能增强

- [ ] 支持更多视频编解码器（HEVC、AV1）
- [ ] 实现 RTMP 协议支持
- [ ] 添加 DASH 协议支持
- [ ] 实现媒体录制功能

### 性能优化

- [ ] GPU 加速编码
- [ ] 零拷贝优化
- [ ] 更好的内存管理

### 平台扩展

- [ ] Linux 平台优化
- [ ] macOS 平台支持
- [ ] Android 平台支持

## 🐛 问题反馈

如果遇到问题或有建议，请通过以下方式反馈：

- 提交 [Issue](../../issues)
- 发送邮件
- 创建 Pull Request

---

**最后更新**：2025-01-XX  
**版本**：1.0.0
