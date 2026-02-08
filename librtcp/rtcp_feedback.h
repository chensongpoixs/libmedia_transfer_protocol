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
 created: 		2025-11-09

 author:			chensong

 purpose:		RTCP Feedback Messages (FCI Structures)
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


#ifndef LIBRTCP_RTCP_FEEDBACK_H_
#define LIBRTCP_RTCP_FEEDBACK_H_

//#include "Rtcp.h"
#include <map>
#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include "rtc_base/checks.h"


namespace libmedia_transfer_protocol {
    namespace librtcp { 
#pragma pack(push, 1)

/////////////////////////////////////////// PSFB (Payload-Specific Feedback) ////////////////////////////////////////////////////

/**
*  @author chensong
*  @date 2025-11-09
*  @brief SLI反馈控制信息（Slice Loss Indication FCI）
*  
*  SLI（Slice Loss Indication）用于指示视频宏块（Macroblock）的丢失。
*  该结构是PSFB（Payload-Specific Feedback）消息的FCI部分，格式类型为2。
*  
*  RFC 4585规范：https://tools.ietf.org/html/rfc4585#section-6.3.2.2
*  
*  数据包格式（4字节）：
*      0                   1                   2                   3
*      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*     |            First        |        Number           | PictureID |
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*  
*  字段说明：
*  - First (13位)：第一个丢失宏块的地址
*    宏块编号从左上角开始为1，按光栅扫描顺序（从左到右，从上到下）递增
*    如果图片总共有N个宏块，右下角的宏块编号为N
*  
*  - Number (13位)：丢失的宏块数量，按扫描顺序计数
*  
*  - PictureID (6位)：编解码器特定的图片标识符的最低6位
*    用于引用发生宏块丢失的图片
*    对于许多视频编解码器，PictureID等同于时间参考（Temporal Reference）
*  
*  使用场景：
*  - 视频传输中检测到宏块丢失
*  - 接收端通知发送端重传丢失的宏块
*  - 用于H.264等视频编解码器的错误恢复
*  
*  @note 该结构使用紧凑的位域表示，总大小为4字节
*  @note 宏块是视频编码的基本单元，通常为16x16像素
*/
class FCI_SLI {
public:
    /**
    *  @brief FCI_SLI结构大小（字节）
    */
    static size_t constexpr kSize = 4;

    /**
    *  @brief 构造SLI反馈控制信息
    *  @param first 第一个丢失宏块的地址（13位）
    *  @param number 丢失的宏块数量（13位）
    *  @param pic_id 图片标识符（6位）
    */
    FCI_SLI(uint16_t first, uint16_t number, uint8_t pic_id);

    /**
    *  @brief 检查数据大小是否正确
    *  @param size 数据大小
    */
    void check(size_t size);

    /**
    *  @brief 获取第一个丢失宏块的地址
    *  @return 宏块地址
    */
    uint16_t getFirst() const;

    /**
    *  @brief 获取丢失的宏块数量
    *  @return 宏块数量
    */
    uint16_t getNumber() const;

    /**
    *  @brief 获取图片标识符
    *  @return 图片ID
    */
    uint8_t getPicID() const;

    /**
    *  @brief 转储为字符串（用于调试）
    *  @return 格式化的字符串
    */
    std::string dumpString() const;

private:
    // 紧凑存储的32位数据，包含First、Number和PictureID字段
    uint32_t data;
};

#if 0
//PSFB fmt = 3
//https://tools.ietf.org/html/rfc4585#section-6.3.3.2
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |      PB       |0| Payload Type|    Native RPSI bit string     |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |   defined per codec          ...                | Padding (0) |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FCI_RPSI {
public:
    //The number of unused bits required to pad the length of the RPSI
    //      message to a multiple of 32 bits.
    uint8_t pb;

#if __linux__
    //0:  1 bit
    //      MUST be set to zero upon transmission and ignored upon reception.
    uint8_t zero : 1;
    //Payload Type: 7 bits
    //      Indicates the RTP payload type in the context of which the native
    //      RPSI bit string MUST be interpreted.
    uint8_t pt : 7;
#else
    uint8_t pt: 7;
    uint8_t zero: 1;
#endif

    // Native RPSI bit string: variable length
    //      The RPSI information as natively defined by the video codec.
    char bit_string[5];

    //Padding: #PB bits
    //      A number of bits set to zero to fill up the contents of the RPSI
    //      message to the next 32-bit boundary.  The number of padding bits
    //      MUST be indicated by the PB field.
    uint8_t padding;

    static size_t constexpr kSize = 8;
};
#endif

/**
*  @author chensong
*  @date 2025-11-09
*  @brief FIR反馈控制信息（Full Intra Request FCI）
*  
*  FIR（Full Intra Request）用于请求发送端生成完整的关键帧（I帧）。
*  该结构是PSFB消息的FCI部分，格式类型为4。
*  
*  RFC 5104规范：https://tools.ietf.org/html/rfc5104#section-4.3.1.1
*  
*  数据包格式（8字节）：
*      0                   1                   2                   3
*      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*     |                              SSRC                             |
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*     | Seq nr.       |    Reserved                                   |
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*  
*  字段说明：
*  - SSRC (32位)：请求关键帧的媒体源的同步源标识符
*  - Seq nr. (8位)：FIR请求的序列号，每次请求递增，用于去重
*  - Reserved (24位)：保留字段，必须设置为0
*  
*  使用场景：
*  - 视频解码器需要关键帧才能开始解码
*  - 发生严重的丢包，需要重新同步
*  - 新的接收者加入会话，需要关键帧
*  - 视频质量严重下降，需要刷新
*  
*  @note FIR请求会导致发送端立即生成关键帧，带宽消耗较大
*  @note 应避免频繁发送FIR请求，建议使用序列号去重
*/
class FCI_FIR {
public:
    /**
    *  @brief FCI_FIR结构大小（字节）
    */
    static size_t constexpr kSize = 8;

    /**
    *  @brief 构造FIR反馈控制信息
    *  @param ssrc 媒体源的SSRC
    *  @param seq_number FIR序列号
    *  @param reserved 保留字段，默认为0
    */
    FCI_FIR(uint32_t ssrc, uint8_t seq_number, uint32_t reserved = 0);

    /**
    *  @brief 检查数据大小是否正确
    *  @param size 数据大小
    */
    void check(size_t size);

    /**
    *  @brief 获取SSRC
    *  @return 同步源标识符
    */
    uint32_t getSSRC() const;

    /**
    *  @brief 获取FIR序列号
    *  @return 序列号
    */
    uint8_t getSeq() const;

    /**
    *  @brief 获取保留字段
    *  @return 保留字段值
    */
    uint32_t getReserved() const;

    /**
    *  @brief 转储为字符串（用于调试）
    *  @return 格式化的字符串
    */
    std::string dumpString() const;

private:
    // 媒体源的同步源标识符
    uint32_t ssrc;
    // FIR请求序列号
    uint8_t seq_number;
    // 保留字段（3字节）
    uint8_t reserved[3];
};

#if 0
//PSFB fmt = 5
//https://tools.ietf.org/html/rfc5104#section-4.3.2.1
// 0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                              SSRC                             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  Seq nr.      |  Reserved                           | Index   |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FCI_TSTR {
public:
    static size_t constexpr kSize = 8;

    void check(size_t size) {
        RTC_CHECK(size == kSize);
    }

private:
    uint8_t data[kSize];
};

//PSFB fmt = 6
//https://tools.ietf.org/html/rfc5104#section-4.3.2.1
// 0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                              SSRC                             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |  Seq nr.      |  Reserved                           | Index   |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FCI_TSTN : public FCI_TSTR{

};

//PSFB fmt = 7
//https://tools.ietf.org/html/rfc5104#section-4.3.4.1
//0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                              SSRC                             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   | Seq nr.       |0| Payload Type| Length                        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                    VBCM Octet String....      |    Padding    |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FCI_VBCM {
public:
    static size_t constexpr kSize = 12;

    void check(size_t size) {
        RTC_CHECK(size == kSize);
    }

private:
    uint8_t data[kSize];
};

#endif

/**
*  @author chensong
*  @date 2025-11-09
*  @brief REMB反馈控制信息（Receiver Estimated Maximum Bitrate FCI）
*  
*  REMB（Receiver Estimated Maximum Bitrate）用于接收端向发送端反馈估算的最大码率。
*  该结构是PSFB消息的FCI部分，格式类型为15（应用层反馈）。
*  
*  Draft规范：https://tools.ietf.org/html/draft-alvestrand-rmcat-remb-03
*  
*  数据包格式（至少8字节）：
*      0                   1                   2                   3
*      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*     |  Unique identifier 'R' 'E' 'M' 'B'                            |
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*     |  Num SSRC     | BR Exp    |  BR Mantissa                      |
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*     |   SSRC feedback                                               |
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*     |  ...                                                          |
*  
*  字段说明：
*  - Unique identifier (32位)：固定为"REMB"（0x52454D42），用于识别REMB消息
*  
*  - Num SSRC (8位)：本消息中包含的SSRC数量
*  
*  - BR Exp (6位)：码率尾数的指数缩放因子，无符号整数[0..63]
*    用于表示大范围的码率值，类似科学计数法
*  
*  - BR Mantissa (18位)：最大总媒体码率的尾数（忽略所有包开销）
*    接收端估算的码率值，单位为比特每秒（bps）
*    实际码率 = Mantissa * 2^Exp
*  
*  - SSRC feedback (32位 * N)：一个或多个SSRC条目，表示此反馈消息适用的媒体流
*  
*  码率计算：
*  - Bitrate = BR_Mantissa * 2^BR_Exp (bps)
*  - 例如：Mantissa=1000, Exp=10 -> Bitrate = 1000 * 1024 = 1,024,000 bps = 1 Mbps
*  
*  使用场景：
*  - 接收端基于网络状况估算可用带宽
*  - 通知发送端调整发送码率，避免拥塞
*  - 用于WebRTC的带宽自适应算法
*  - 替代传统的TMMBR（Temporary Maximum Media Stream Bit Rate Request）
*  
*  @note REMB是Google提出的带宽估算方案，广泛用于WebRTC
*  @note 相比TWCC，REMB是接收端估算，TWCC是发送端估算
*  @note 现代WebRTC更倾向使用TWCC进行拥塞控制
*/
class FCI_REMB {
public:
    /**
    *  @brief FCI_REMB最小结构大小（字节）
    */
    static size_t constexpr kSize = 8;

    /**
    *  @brief 创建REMB反馈消息
    *  @param ssrcs SSRC列表，表示适用的媒体流
    *  @param bitrate 估算的最大码率（bps）
    *  @return REMB消息的字符串表示
    */
    static std::string create(const std::vector<uint32_t> &ssrcs, uint32_t bitrate);

    /**
    *  @brief 检查数据大小是否正确
    *  @param size 数据大小
    */
    void check(size_t size);

    /**
    *  @brief 转储为字符串（用于调试）
    *  @return 格式化的字符串
    */
    std::string dumpString() const;

    /**
    *  @brief 获取估算的码率
    *  @return 码率值（bps）
    *  @note 通过BR_Mantissa * 2^BR_Exp计算
    */
    uint32_t getBitRate() const;

    /**
    *  @brief 获取SSRC列表
    *  @return SSRC向量
    */
    std::vector<uint32_t> getSSRC();

private:
    // 唯一标识符 'R' 'E' 'M' 'B' (0x52454D42)
    char magic[4];
    // Num SSRC (8位) / BR Exp (6位) / BR Mantissa (18位)
    uint8_t bitrate[4];
    // SSRC反馈列表（可变长度），包含一个或多个SSRC条目
    uint32_t ssrc_feedback[1];
};

/////////////////////////////////////////// RTPFB (RTP Feedback) ////////////////////////////////////////////////////

/**
*  @author chensong
*  @date 2025-11-09
*  @brief NACK反馈控制信息（Generic NACK FCI）
*  
*  NACK（Negative Acknowledgement）用于请求重传丢失的RTP包。
*  该结构是RTPFB（RTP Feedback）消息的FCI部分，格式类型为1。
*  
*  RFC 4585规范：https://tools.ietf.org/html/rfc4585#section-6.2.1
*  
*  数据包格式（4字节）：
*      0                   1                   2                   3
*      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*     |            PID                |             BLP               |
*     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*  
*  字段说明：
*  - PID (Packet ID, 16位)：丢失包的RTP序列号
*    指定第一个丢失的包的序列号
*  
*  - BLP (Bitmask of following Lost Packets, 16位)：后续丢失包的位掩码
*    每一位表示PID之后的包是否丢失
*    位0表示PID+1，位1表示PID+2，以此类推
*    位值为1表示该包丢失，0表示该包已收到
*  
*  工作原理：
*  - 一个NACK FCI可以表示最多17个丢失的包（1个PID + 16个BLP位）
*  - 例如：PID=100, BLP=0x0005 (二进制: 0000000000000101)
*    表示包100、101、103丢失（位0和位2为1）
*  
*  使用场景：
*  - 检测到RTP包丢失时，请求发送端重传
*  - 用于可靠传输场景，如视频会议、屏幕共享
*  - 相比FEC（前向纠错），NACK延迟更低但需要往返时间
*  
*  优化策略：
*  - 合并多个NACK请求，减少RTCP包数量
*  - 使用BLP位掩码表示连续的丢包范围
*  - 避免重复发送NACK，使用定时器去重
*  
*  @note NACK是最常用的丢包恢复机制
*  @note 一个NACK FCI可以高效表示多个丢失的包
*  @note 发送端收到NACK后应尽快重传请求的包
*/
class FCI_NACK {
public:
    /**
    *  @brief FCI_NACK结构大小（字节）
    */
    static constexpr size_t kSize = 4;

    /**
    *  @brief BLP位掩码的位数
    */
    static constexpr size_t kBitSize = 16;

    /**
    *  @brief 构造NACK反馈控制信息
    *  @param pid_h 丢失包的序列号（PID）
    *  @param type 后续丢失包的位掩码数组（长度为16）
    */
    FCI_NACK(uint16_t pid_h, const std::vector<bool> &type);

    /**
    *  @brief 检查数据大小是否正确
    *  @param size 数据大小
    */
    void check(size_t size);

    /**
    *  @brief 获取PID（丢失包序列号）
    *  @return 序列号
    */
    uint16_t getPid() const;

    /**
    *  @brief 获取BLP（位掩码）
    *  @return 16位位掩码
    */
    uint16_t getBlp() const;

    /**
    *  @brief 获取丢包列表的位数组
    *  @return 长度为17的布尔向量，第一个包必丢（PID），后续16个根据BLP确定
    *  @note 返回的数组总长度为17，第一个元素对应PID（必为true）
    */
    std::vector<bool> getBitArray() const;

    /**
    *  @brief 转储为字符串（用于调试）
    *  @return 格式化的字符串
    */
    std::string dumpString() const;

private:
    // PID字段，用于指定丢失包的RTP序列号
    uint16_t pid;
    // BLP字段，后续丢失包的位掩码（16位）
    uint16_t blp;
};

#if 0
//RTPFB fmt = 3
//https://tools.ietf.org/html/rfc5104#section-4.2.1.1
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                              SSRC                             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   | MxTBR Exp |  MxTBR Mantissa                 |Measured Overhead|
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FCI_TMMBR {
public:
    static size_t constexpr kSize = 8;

    void check(size_t size) {
        RTC_CHECK(size == kSize);
    }

private:
    //SSRC (32 bits): The SSRC value of the media sender that is
    //              requested to obey the new maximum bit rate.
    uint32_t ssrc;

    //     MxTBR Exp (6 bits): The exponential scaling of the mantissa for the
    //              maximum total media bit rate value.  The value is an
    //              unsigned integer [0..63].
    //     MxTBR Mantissa (17 bits): The mantissa of the maximum total media
    //              bit rate value as an unsigned integer.
    //     Measured Overhead (9 bits): The measured average packet overhead
    //              value in bytes.  The measurement SHALL be done according
    //              to the description in section 4.2.1.2. The value is an
    //              unsigned integer [0..511].
    uint32_t max_tbr;
};

//RTPFB fmt = 4
// https://tools.ietf.org/html/rfc5104#section-4.2.2.1
// 0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                              SSRC                             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   | MxTBR Exp |  MxTBR Mantissa                 |Measured Overhead|
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FCI_TMMBN : public FCI_TMMBR{
public:

};
#endif

/**
*  @brief 包状态符号枚举（Symbol Status）
*  
*  用于TWCC反馈中表示RTP包的接收状态和时间增量类型。
*  
*  状态说明：
*  - not_received (0)：包未接收到
*  - small_delta (1)：包已接收，时间增量较小（可用1字节表示，范围0-255，精度250us）
*  - large_delta (2)：包已接收，时间增量较大或为负（需用2字节表示）
*  - reserved (3)：保留状态，未使用
*  
*  时间增量说明：
*  - small_delta：增量范围0-63.75ms（0-255 * 250us）
*  - large_delta：增量范围-8192ms到+8191.75ms（-32768到+32767 * 250us）
*  
*  @note small_delta是指能用一个字节表示的数值
*  @note large_delta是指能用两个字节表示的数值
*/
enum class SymbolStatus : uint8_t {
    // 包未接收
    not_received = 0,
    // 包已接收，小增量（1字节）
    small_delta = 1,
    // 包已接收，大增量或负增量（2字节）
    large_delta = 2,
    // 保留
    reserved = 3
};

// RTPFB fmt = 15
// https://tools.ietf.org/html/draft-holmer-rmcat-transport-wide-cc-extensions-01#section-3.1
// https://zhuanlan.zhihu.com/p/206656654
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |      base sequence number     |      packet status count      |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                 reference time                | fb pkt. count |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |          packet chunk         |         packet chunk          |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   .                                                               .
//   .                                                               .
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |         packet chunk          |  recv delta   |  recv delta   |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   .                                                               .
//   .                                                               .
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |           recv delta          |  recv delta   | zero padding  |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FCI_TWCC {
public:
    static size_t constexpr kSize = 8;
    using TwccPacketStatus
        = std::map<uint16_t /*rtp ext seq*/, std::pair<SymbolStatus, int16_t /*recv delta,单位为250us*/>>;
    void check(size_t size);
    std::string dumpString(size_t total_size) const;
    uint16_t getBaseSeq() const;
    // 单位64ms  [AUTO-TRANSLATED:992ffed7]
    // Unit 64ms
    uint32_t getReferenceTime() const;
    uint16_t getPacketCount() const;
    TwccPacketStatus getPacketChunkList(size_t total_size) const;

    static std::string create(uint32_t ref_time, uint8_t fb_pkt_count, TwccPacketStatus &status);

private:
    // base sequence number,基础序号,本次反馈的第一个包的序号;也就是RTP扩展头的序列号  [AUTO-TRANSLATED:4e43ffcc]
    // base sequence number, basic sequence number, the sequence number of the first packet in this feedback; that is, the sequence number of the RTP extension header
    uint16_t base_seq;
    // packet status count, 包个数,本次反馈包含多少个包的状态;从基础序号开始算  [AUTO-TRANSLATED:533efb94]
    // packet status count, number of packets, how many packet statuses are included in this feedback; counted from the base sequence number
    uint16_t pkt_status_count;
    // reference time,基准时间,绝对时间;计算该包中每个媒体包的到达时间都要基于这个基准时间计算  [AUTO-TRANSLATED:5265d98e]
    // reference time, reference time, absolute time; the arrival time of each media packet in this packet is calculated based on this reference time
    uint8_t ref_time[3];
    // feedback packet count,反馈包号,本包是第几个transport-cc包，每次加1                          |  [AUTO-TRANSLATED:1ff6d73e]
    // feedback packet count, feedback packet number, this packet is the nth transport-cc packet, incremented by 1 each time                          |
    uint8_t fb_pkt_count;
};
#pragma pack(pop)
}
} // namespace  
#endif //  
