# libflv - FLV 格式封装库

## 概述

libflv 是一个完整的 FLV（Flash Video）格式封装库，支持将 H.264 视频和 AAC 音频封装为 FLV 格式，用于流媒体传输和播放。

## 核心功能

### 1. FLV 格式封装
- **FLV Header**：生成 FLV 文件头
- **FLV Tag**：封装音频、视频、脚本数据标签
- **时间戳管理**：处理 PTS/DTS 时间戳
- **Previous Tag Size**：维护标签大小信息

### 2. H.264 视频封装
- **AVC Sequence Header**：封装 SPS/PPS
- **AVC NALU**：封装 H.264 NALU 单元
- **关键帧标识**：标记 IDR 帧
- **时间戳转换**：DTS/PTS 转换为 FLV 时间戳

### 3. AAC 音频封装
- **AAC Sequence Header**：封装 AudioSpecificConfig
- **AAC Raw Data**：封装 AAC 原始音频数据
- **采样率转换**：支持多种采样率

### 4. AMF0 元数据
- **onMetaData**：封装视频元信息
- **数据类型支持**：Number、String、Object、Array 等
- **自定义属性**：支持添加自定义元数据

## 文件结构

```
libflv/
├── README.md              # 本文件
├── cflv_encoder.h/cpp     # FLV 编码器
└── amf0.h/c               # AMF0 格式编解码
```

## FLV 格式说明

### FLV 文件结构
```
┌─────────────────────────────────────┐
│  FLV Header (9 bytes)               │
│  ├─ Signature: "FLV" (3 bytes)     │
│  ├─ Version: 1 (1 byte)            │
│  ├─ Flags: 0x05 (1 byte)           │
│  └─ Header Size: 9 (4 bytes)       │
├─────────────────────────────────────┤
│  Previous Tag Size 0 (4 bytes)      │
├─────────────────────────────────────┤
│  FLV Tag 1                          │
│  ├─ Tag Type (1 byte)               │
│  ├─ Data Size (3 bytes)             │
│  ├─ Timestamp (3 bytes)             │
│  ├─ Timestamp Extended (1 byte)     │
│  ├─ Stream ID (3 bytes)             │
│  └─ Tag Data (Data Size bytes)      │
├─────────────────────────────────────┤
│  Previous Tag Size 1 (4 bytes)      │
├─────────────────────────────────────┤
│  FLV Tag 2                          │
│  ...                                │
└─────────────────────────────────────┘
```

### Tag 类型
- **0x08**：音频标签
- **0x09**：视频标签
- **0x12**：脚本数据标签（元数据）

### 视频标签格式
```
┌─────────────────────────────────────┐
│  Frame Type (4 bits)                │
│  ├─ 1: keyframe (IDR)               │
│  ├─ 2: inter frame                  │
│  └─ 5: video info/command frame     │
├─────────────────────────────────────┤
│  Codec ID (4 bits)                  │
│  └─ 7: AVC (H.264)                  │
├─────────────────────────────────────┤
│  AVC Packet Type (1 byte)           │
│  ├─ 0: AVC sequence header          │
│  ├─ 1: AVC NALU                     │
│  └─ 2: AVC end of sequence          │
├─────────────────────────────────────┤
│  Composition Time (3 bytes)         │
│  (CTS = PTS - DTS)                  │
├─────────────────────────────────────┤
│  Video Data                         │
│  ├─ Sequence Header: SPS/PPS        │
│  └─ NALU: H.264 数据                │
└─────────────────────────────────────┘
```

### 音频标签格式
```
┌─────────────────────────────────────┐
│  Sound Format (4 bits)              │
│  └─ 10: AAC                         │
├─────────────────────────────────────┤
│  Sound Rate (2 bits)                │
│  ├─ 0: 5.5 kHz                      │
│  ├─ 1: 11 kHz                       │
│  ├─ 2: 22 kHz                       │
│  └─ 3: 44 kHz                       │
├─────────────────────────────────────┤
│  Sound Size (1 bit)                 │
│  ├─ 0: 8-bit                        │
│  └─ 1: 16-bit                       │
├─────────────────────────────────────┤
│  Sound Type (1 bit)                 │
│  ├─ 0: Mono                         │
│  └─ 1: Stereo                       │
├─────────────────────────────────────┤
│  AAC Packet Type (1 byte)           │
│  ├─ 0: AAC sequence header          │
│  └─ 1: AAC raw data                 │
├─────────────────────────────────────┤
│  Audio Data                         │
│  ├─ Sequence Header: ASC            │
│  └─ Raw Data: AAC 数据              │
└─────────────────────────────────────┘
```

## 核心类说明

### FlvEncoder
FLV 编码器，负责将音视频数据封装为 FLV 格式。

**主要功能：**
- 生成 FLV 文件头
- 封装视频标签
- 封装音频标签
- 封装元数据标签

**使用示例：**
```cpp
FlvEncoder encoder;

// 写入 FLV 头部
auto header = encoder.WriteFlvHeader(true, true);

// 写入元数据
auto metadata = encoder.WriteMetadata(width, height, framerate, ...);

// 写入视频 Sequence Header (SPS/PPS)
auto video_header = encoder.WriteVideoSequenceHeader(sps, sps_len, pps, pps_len);

// 写入视频帧
auto video_frame = encoder.WriteVideoFrame(
    nalu_data, nalu_len, 
    timestamp, 
    is_keyframe
);

// 写入音频 Sequence Header (ASC)
auto audio_header = encoder.WriteAudioSequenceHeader(asc, asc_len);

// 写入音频帧
auto audio_frame = encoder.WriteAudioFrame(aac_data, aac_len, timestamp);
```

### AMF0
AMF0（Action Message Format 0）编解码器。

**支持的数据类型：**
- Number（双精度浮点数）
- Boolean（布尔值）
- String（字符串）
- Object（对象）
- Null（空值）
- Undefined（未定义）
- ECMAArray（关联数组）
- StrictArray（严格数组）
- Date（日期）
- LongString（长字符串）

**使用示例：**
```cpp
// 写入 Number
amf0_write_number(buffer, &pos, 1920.0);

// 写入 String
amf0_write_string(buffer, &pos, "width");

// 写入 Object
amf0_write_object_start(buffer, &pos);
amf0_write_string(buffer, &pos, "width");
amf0_write_number(buffer, &pos, 1920.0);
amf0_write_object_end(buffer, &pos);
```

## 使用场景

### 1. HTTP-FLV 直播
```cpp
FlvEncoder encoder;

// 初始化
auto header = encoder.WriteFlvHeader(true, true);
SendToClient(header);

// 发送元数据
auto metadata = encoder.WriteMetadata(...);
SendToClient(metadata);

// 发送视频 Sequence Header
auto video_header = encoder.WriteVideoSequenceHeader(sps, sps_len, pps, pps_len);
SendToClient(video_header);

// 持续发送视频帧
while (streaming) {
    auto frame = encoder.WriteVideoFrame(data, len, timestamp, is_keyframe);
    SendToClient(frame);
}
```

### 2. FLV 文件录制
```cpp
FlvEncoder encoder;
FILE* file = fopen("output.flv", "wb");

// 写入文件头
auto header = encoder.WriteFlvHeader(true, true);
fwrite(header.data(), 1, header.size(), file);

// 写入音视频数据
// ...

fclose(file);
```

### 3. FLV 转封装
```cpp
// 从其他格式（如 MP4）读取数据
H264Frame video_frame = ReadH264Frame();
AACFrame audio_frame = ReadAACFrame();

// 封装为 FLV
FlvEncoder encoder;
auto flv_video = encoder.WriteVideoFrame(
    video_frame.data, 
    video_frame.len, 
    video_frame.timestamp,
    video_frame.is_keyframe
);
auto flv_audio = encoder.WriteAudioFrame(
    audio_frame.data,
    audio_frame.len,
    audio_frame.timestamp
);
```

## H.264 封装说明

### AVC Sequence Header
包含 SPS 和 PPS 信息，必须在第一个视频帧之前发送。

**格式：**
```
┌─────────────────────────────────────┐
│  Configuration Version (1 byte)     │
│  AVC Profile (1 byte)               │
│  Profile Compatibility (1 byte)     │
│  AVC Level (1 byte)                 │
│  NALU Length Size - 1 (1 byte)      │
│  Number of SPS (1 byte)             │
│  SPS Length (2 bytes)               │
│  SPS Data (SPS Length bytes)        │
│  Number of PPS (1 byte)             │
│  PPS Length (2 bytes)               │
│  PPS Data (PPS Length bytes)        │
└─────────────────────────────────────┘
```

### AVC NALU
包含 H.264 NALU 单元。

**格式：**
```
┌─────────────────────────────────────┐
│  NALU Length (4 bytes)              │
│  NALU Data (NALU Length bytes)      │
│  ...                                │
└─────────────────────────────────────┘
```

## AAC 封装说明

### AAC Sequence Header
包含 AudioSpecificConfig 信息。

**格式：**
```
┌─────────────────────────────────────┐
│  Audio Object Type (5 bits)         │
│  Sampling Frequency Index (4 bits)  │
│  Channel Configuration (4 bits)     │
│  ...                                │
└─────────────────────────────────────┘
```

### AAC Raw Data
包含 AAC 原始音频数据（不包含 ADTS 头）。

## 时间戳处理

### DTS 和 PTS
- **DTS（Decoding Time Stamp）**：解码时间戳
- **PTS（Presentation Time Stamp）**：显示时间戳
- **CTS（Composition Time）**：PTS - DTS

### FLV 时间戳
FLV 使用 32 位时间戳，单位为毫秒。

**计算方式：**
```cpp
// 将 90kHz 时间戳转换为毫秒
uint32_t flv_timestamp = rtp_timestamp / 90;

// 计算 CTS
int32_t cts = (pts - dts) / 90;
```

## 性能优化

### 1. 内存预分配
预分配足够大的缓冲区，避免频繁分配。

### 2. 批量写入
批量写入多个标签，减少系统调用。

### 3. 零拷贝
尽可能使用指针操作，避免数据拷贝。

## 注意事项

1. **字节序**：FLV 使用大端字节序（网络字节序）
2. **时间戳回绕**：处理 32 位时间戳回绕问题
3. **关键帧**：确保第一帧是关键帧
4. **Sequence Header**：必须在第一帧之前发送
5. **Previous Tag Size**：正确计算和写入

## 参考资料

- [Adobe FLV File Format Specification](https://www.adobe.com/devnet/f4v.html)
- [ISO/IEC 14496-10 - H.264/AVC](https://www.itu.int/rec/T-REC-H.264)
- [ISO/IEC 14496-3 - AAC](https://www.iso.org/standard/53943.html)
- [AMF0 Specification](https://www.adobe.com/content/dam/acom/en/devnet/pdf/amf0-file-format-specification.pdf)

## 作者

chensong - 2025

## 许可证

BSD-style license
