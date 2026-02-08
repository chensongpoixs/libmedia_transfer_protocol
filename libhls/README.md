# libhls - HLS 流媒体封装库

## 概述

libhls 是一个完整的 HLS（HTTP Live Streaming）协议实现库，支持将音视频数据封装为 MPEG-TS 切片和 M3U8 播放列表，用于自适应流媒体传输。

## 核心功能

### 1. MPEG-TS 封装
- **TS Packet**：生成 188 字节的 TS 包
- **PAT/PMT**：生成节目关联表和节目映射表
- **PES Packet**：封装 H.264/AAC 为 PES 包
- **PCR**：生成节目时钟参考

### 2. HLS 切片管理
- **切片创建**：创建固定时长的 TS 切片
- **切片窗口**：管理滑动窗口中的切片
- **切片释放**：自动释放过期切片
- **内存管理**：动态扩展切片缓冲区

### 3. M3U8 播放列表
- **Master Playlist**：多码率播放列表
- **Media Playlist**：单码率播放列表
- **EXT-X-TARGETDURATION**：目标切片时长
- **EXT-X-MEDIA-SEQUENCE**：切片序列号

## 文件结构

```
libhls/
├── README.md                  # 本文件
├── cfragment.h/cpp            # HLS 切片管理
├── cfragment_window.h/cpp     # HLS 切片窗口
└── chls_muxer.h/cpp           # HLS 复用器
```

## HLS 协议说明

### HLS 工作流程
```
┌─────────────────────────────────────────────┐
│  1. 客户端请求 M3U8 播放列表                │
│     GET /stream.m3u8                        │
├─────────────────────────────────────────────┤
│  2. 服务器返回播放列表                      │
│     #EXTM3U                                 │
│     #EXT-X-TARGETDURATION:10                │
│     #EXT-X-MEDIA-SEQUENCE:0                 │
│     #EXTINF:10.0,                           │
│     segment0.ts                             │
│     #EXTINF:10.0,                           │
│     segment1.ts                             │
├─────────────────────────────────────────────┤
│  3. 客户端请求 TS 切片                      │
│     GET /segment0.ts                        │
├─────────────────────────────────────────────┤
│  4. 服务器返回 TS 切片                      │
│     (MPEG-TS 格式的音视频数据)              │
├─────────────────────────────────────────────┤
│  5. 客户端定期刷新播放列表                  │
│     GET /stream.m3u8                        │
└─────────────────────────────────────────────┘
```

### M3U8 播放列表格式
```
#EXTM3U
#EXT-X-VERSION:3
#EXT-X-TARGETDURATION:10
#EXT-X-MEDIA-SEQUENCE:0
#EXTINF:10.0,
segment0.ts
#EXTINF:10.0,
segment1.ts
#EXTINF:10.0,
segment2.ts
```

### MPEG-TS 包结构
```
┌─────────────────────────────────────┐
│  Sync Byte: 0x47 (1 byte)          │
├─────────────────────────────────────┤
│  Transport Error Indicator (1 bit)  │
│  Payload Unit Start (1 bit)         │
│  Transport Priority (1 bit)         │
│  PID (13 bits)                      │
├─────────────────────────────────────┤
│  Scrambling Control (2 bits)        │
│  Adaptation Field (2 bits)          │
│  Continuity Counter (4 bits)        │
├─────────────────────────────────────┤
│  Adaptation Field (optional)        │
├─────────────────────────────────────┤
│  Payload (184 bytes max)            │
└─────────────────────────────────────┘
```

## 核心类说明

### Fragment
HLS 切片类，管理单个 TS 切片的数据。

**主要功能：**
- 创建切片缓冲区
- 写入 TS 数据
- 动态扩展缓冲区
- 标记切片完成

**使用示例：**
```cpp
Fragment fragment(0, 10000);  // 序列号0，时长10秒

// 写入数据
fragment.Write(ts_data, ts_len);

// 标记完成
fragment.SetComplete(true);

// 获取数据
const char* data = fragment.Data();
size_t size = fragment.Size();
```

### FragmentWindow
HLS 切片窗口类，管理滑动窗口中的多个切片。

**主要功能：**
- 创建新切片
- 获取当前切片
- 释放过期切片
- 生成 M3U8 播放列表

**使用示例：**
```cpp
FragmentWindow window(3);  // 保持3个切片

// 创建新切片
auto fragment = window.CreateFragment(duration);

// 写入数据
fragment->Write(ts_data, ts_len);

// 完成切片
fragment->SetComplete(true);

// 生成播放列表
std::string m3u8 = window.GeneratePlaylist();
```

### HlsMuxer
HLS 复用器类，将音视频数据封装为 MPEG-TS 格式。

**主要功能：**
- 生成 PAT/PMT 表
- 封装 H.264 视频
- 封装 AAC 音频
- 生成 PCR

**使用示例：**
```cpp
HlsMuxer muxer;

// 初始化
muxer.Init(video_pid, audio_pid);

// 写入视频帧
muxer.WriteVideo(h264_data, h264_len, pts, dts, is_keyframe);

// 写入音频帧
muxer.WriteAudio(aac_data, aac_len, pts);

// 获取 TS 数据
auto ts_data = muxer.GetTsData();
```

## 使用场景

### 1. HLS 直播
```cpp
FragmentWindow window(3);
HlsMuxer muxer;

// 初始化
muxer.Init(256, 257);

// 创建切片
auto fragment = window.CreateFragment(10000);

// 持续写入数据
while (streaming) {
    // 编码视频帧
    H264Frame video = EncodeVideo();
    auto ts_video = muxer.WriteVideo(
        video.data, video.len, 
        video.pts, video.dts, 
        video.is_keyframe
    );
    fragment->Write(ts_video.data(), ts_video.size());
    
    // 检查是否需要切换切片
    if (fragment->Duration() >= 10000) {
        fragment->SetComplete(true);
        fragment = window.CreateFragment(10000);
    }
}

// 生成播放列表
std::string m3u8 = window.GeneratePlaylist();
```

### 2. HLS 点播
```cpp
// 读取完整视频文件
VideoFile video = ReadVideoFile("input.mp4");

// 切片
std::vector<Fragment> fragments;
for (int i = 0; i < video.duration / 10; i++) {
    Fragment fragment(i, 10000);
    
    // 写入10秒的数据
    auto segment_data = video.GetSegment(i * 10, 10);
    fragment.Write(segment_data.data(), segment_data.size());
    fragment.SetComplete(true);
    
    fragments.push_back(fragment);
}

// 生成播放列表
std::string m3u8 = GenerateVodPlaylist(fragments);
```

## MPEG-TS 封装说明

### PAT（Program Association Table）
节目关联表，PID = 0。

**字段：**
- Table ID: 0x00
- Program Number: 1
- Program Map PID: 通常为 0x1000

### PMT（Program Map Table）
节目映射表，PID 由 PAT 指定。

**字段：**
- Table ID: 0x02
- PCR PID: 视频 PID
- Stream Type: 0x1B (H.264), 0x0F (AAC)
- Elementary PID: 音视频 PID

### PES（Packetized Elementary Stream）
打包的基本流。

**格式：**
```
┌─────────────────────────────────────┐
│  Packet Start Code: 0x000001        │
│  Stream ID (1 byte)                 │
│  PES Packet Length (2 bytes)        │
│  PES Header Flags (2 bytes)         │
│  PES Header Length (1 byte)         │
│  PTS/DTS (optional)                 │
│  PES Data                           │
└─────────────────────────────────────┘
```

### PCR（Program Clock Reference）
节目时钟参考，用于同步。

**计算方式：**
```cpp
// PCR = base * 300 + extension
uint64_t pcr_base = timestamp / 300;
uint16_t pcr_ext = timestamp % 300;
```

## 切片策略

### 切片时长
- **推荐值**：6-10 秒
- **最小值**：2 秒
- **最大值**：不超过 30 秒

### 切片窗口
- **直播**：保持 3-5 个切片
- **点播**：保持所有切片

### 关键帧对齐
每个切片必须以关键帧开始。

```cpp
if (is_keyframe && fragment->Duration() >= target_duration) {
    // 完成当前切片
    fragment->SetComplete(true);
    
    // 创建新切片
    fragment = window.CreateFragment(target_duration);
}
```

## 性能优化

### 1. 缓冲区预分配
预分配足够大的缓冲区，减少动态扩展。

### 2. 批量写入
批量写入 TS 包，减少系统调用。

### 3. 内存池
使用内存池管理切片对象。

## 注意事项

1. **关键帧对齐**：切片必须以关键帧开始
2. **时间戳连续**：确保时间戳连续递增
3. **PCR 频率**：每 100ms 插入一次 PCR
4. **切片时长**：保持切片时长稳定
5. **播放列表更新**：及时更新播放列表

## 参考资料

- [RFC 8216 - HTTP Live Streaming](https://tools.ietf.org/html/rfc8216)
- [ISO/IEC 13818-1 - MPEG-TS](https://www.iso.org/standard/62074.html)
- [Apple HLS Authoring Specification](https://developer.apple.com/documentation/http_live_streaming)

## 作者

chensong - 2025

## 许可证

BSD-style license
