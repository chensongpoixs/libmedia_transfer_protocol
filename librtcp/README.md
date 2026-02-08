# librtcp - RTCP协议实现库

## 概述

librtcp 是一个完整的 RTCP（RTP Control Protocol）协议实现库，提供实时传输控制功能，包括传输质量监控、拥塞控制和反馈机制。

## 核心功能

### 1. TWCC（Transport-Wide Congestion Control）
- **传输层拥塞控制**：基于 draft-holmer-rmcat-transport-wide-cc-extensions 实现
- **序列号管理**：支持序列号回绕处理
- **到达时间记录**：精确记录数据包到达时间
- **反馈报文生成**：生成 TWCC 反馈报文

### 2. RTCP 统计信息
- **发送端统计**：发送包数、字节数、时间戳
- **接收端统计**：接收包数、丢包率、抖动
- **往返时延（RTT）**：计算网络延迟
- **带宽估算**：估算可用带宽

### 3. RTCP 反馈机制
- **NACK（Negative Acknowledgement）**：请求重传丢失的数据包
- **FIR（Full Intra Request）**：请求关键帧
- **PLI（Picture Loss Indication）**：指示图像丢失
- **REMB（Receiver Estimated Maximum Bitrate）**：接收端带宽估算
- **SLI（Slice Loss Indication）**：指示切片丢失

### 4. 循环缓冲区
- **高效存储**：使用循环缓冲区存储数据包信息
- **自动覆盖**：旧数据自动被新数据覆盖
- **序列号索引**：通过序列号快速查找数据

## 文件结构

```
librtcp/
├── README.md                              # 本文件
├── twcc_context.h/cpp                     # TWCC上下文管理
├── packet_arrival_map.h/cpp               # 数据包到达时间映射
├── buffer.h/cpp                           # 循环缓冲区实现
├── rtcp_context.h/cpp                     # RTCP上下文（发送端）
├── rtcp_context_recv.h/cpp                # RTCP上下文（接收端）
├── rtcp_feedback.h                        # RTCP反馈消息定义
└── transport_sequence_number_feedback.h   # TWCC反馈消息定义
```

## 核心类说明

### TwccContext
TWCC 拥塞控制上下文，管理传输层拥塞控制。

**主要功能：**
- 记录数据包发送和接收信息
- 生成 TWCC 反馈报文
- 处理序列号回绕

**使用示例：**
```cpp
TwccContext twcc;

// 发送端：记录发送信息
twcc.OnSendPacket(seq_num, timestamp);

// 接收端：记录接收信息
twcc.OnReceivePacket(seq_num, arrival_time);

// 生成反馈报文
auto feedback = twcc.GenerateFeedback();
```

### PacketArrivalMap
数据包到达时间映射表，记录数据包的到达时间。

**主要功能：**
- 存储数据包到达时间
- 支持序列号回绕
- 高效查询和更新

### Buffer
循环缓冲区，用于存储固定数量的数据包信息。

**主要功能：**
- 固定大小的循环存储
- 自动覆盖旧数据
- 通过索引快速访问

### RtcpContext
RTCP 发送端上下文，管理发送端的统计信息。

**主要功能：**
- 记录发送统计信息
- 生成 SR（Sender Report）
- 计算发送速率

### RtcpContextRecv
RTCP 接收端上下文，管理接收端的统计信息。

**主要功能：**
- 记录接收统计信息
- 生成 RR（Receiver Report）
- 计算丢包率和抖动

## RTCP 报文类型

### SR（Sender Report）
发送端报告，包含发送端的统计信息。

**字段：**
- NTP 时间戳
- RTP 时间戳
- 发送包数
- 发送字节数

### RR（Receiver Report）
接收端报告，包含接收端的统计信息。

**字段：**
- 丢包率
- 累计丢包数
- 最高序列号
- 抖动
- LSR（Last SR）
- DLSR（Delay since Last SR）

### TWCC Feedback
传输层拥塞控制反馈报文。

**字段：**
- 基准序列号
- 包状态向量
- 接收增量时间

## 使用场景

### 1. WebRTC 应用
```cpp
// 创建 RTCP 上下文
RtcpContext sender_ctx;
RtcpContextRecv receiver_ctx;

// 发送端：记录发送信息
sender_ctx.OnSendPacket(packet);

// 接收端：记录接收信息
receiver_ctx.OnReceivePacket(packet);

// 生成 RTCP 报告
auto sr = sender_ctx.GenerateSR();
auto rr = receiver_ctx.GenerateRR();
```

### 2. 拥塞控制
```cpp
TwccContext twcc;

// 记录数据包信息
for (auto& packet : packets) {
    twcc.OnReceivePacket(packet.seq, packet.arrival_time);
}

// 生成反馈
auto feedback = twcc.GenerateFeedback();

// 发送反馈给发送端
SendFeedback(feedback);
```

### 3. 丢包检测和重传
```cpp
RtcpContextRecv recv_ctx;

// 检测丢包
auto lost_packets = recv_ctx.GetLostPackets();

// 生成 NACK 请求
auto nack = GenerateNACK(lost_packets);

// 发送 NACK
SendNACK(nack);
```

## 关键概念

### 序列号回绕
RTP 序列号是 16 位的，范围是 0-65535。当序列号达到最大值后会回绕到 0。

**处理方式：**
```cpp
// 使用扩展序列号（32位）
uint32_t extended_seq = CalculateExtendedSeq(seq16);

// 比较序列号时考虑回绕
bool is_newer = IsNewerSequence(seq1, seq2);
```

### 抖动计算
抖动是数据包到达时间间隔的统计方差。

**计算公式：**
```
J(i) = J(i-1) + (|D(i-1,i)| - J(i-1)) / 16
其中 D(i-1,i) = (R(i) - R(i-1)) - (S(i) - S(i-1))
```

### RTT 计算
往返时延通过 SR 和 RR 报文计算。

**计算方式：**
```
RTT = 当前时间 - LSR - DLSR
```

## 性能优化

### 1. 循环缓冲区
使用固定大小的循环缓冲区，避免动态内存分配。

### 2. 序列号索引
通过序列号直接计算索引，O(1) 时间复杂度。

### 3. 批量处理
批量生成反馈报文，减少网络开销。

## 注意事项

1. **线程安全**：本库不是线程安全的，需要外部同步
2. **序列号回绕**：正确处理序列号回绕，避免错误判断
3. **时间戳精度**：使用高精度时间戳（微秒级）
4. **缓冲区大小**：根据网络条件调整缓冲区大小
5. **反馈频率**：控制 RTCP 反馈频率，避免过载

## 参考资料

- [RFC 3550 - RTP: A Transport Protocol for Real-Time Applications](https://tools.ietf.org/html/rfc3550)
- [RFC 4585 - Extended RTP Profile for RTCP-Based Feedback](https://tools.ietf.org/html/rfc4585)
- [draft-holmer-rmcat-transport-wide-cc-extensions](https://tools.ietf.org/html/draft-holmer-rmcat-transport-wide-cc-extensions)
- [RFC 5104 - Codec Control Messages in AVPF](https://tools.ietf.org/html/rfc5104)

## 作者

chensong - 2025

## 许可证

BSD-style license
