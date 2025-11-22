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
 created: 		2025-04-29

 author:			chensong

 purpose:		http_parser
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

#ifndef _C_LIBMEDIA_TRNER_PASCKET______
#define _C_LIBMEDIA_TRNER_PASCKET______

#include <string>
#include <memory>
#include <cstring>
#include <cstdint>

namespace libmedia_transfer_protocol
{
    namespace libmpeg
    {
        /**
        *  @author chensong
        *  @date 2025-04-29
        *  @brief 数据包类型枚举（Packet Type Enumeration）
        *  
        *  该枚举用于标识数据包的类型和帧类型。数据包可以是视频、音频或元数据，
        *  视频帧可以是关键帧或IDR帧。
        *  
        *  数据包类型说明：
        *  - kPacketTypeVideo: 视频数据包
        *  - kPacketTypeAudio: 音频数据包
        *  - kPacketTypeMeta: 元数据包
        *  - kPacketTypeMeta3: 元数据包类型3
        *  - kFrameTypeKeyFrame: 关键帧类型（I帧）
        *  - kFrameTypeIDR: IDR帧类型（立即解码刷新帧）
        *  - kPacketTypeUnknowed: 未知类型
        *  
        *  @note 数据包类型使用位标志，可以组合使用（如视频+关键帧）
        */
        enum
        {
            kPacketTypeVideo = 1,        ///< 视频数据包类型
            kPacketTypeAudio = 2,        ///< 音频数据包类型
            kPacketTypeMeta = 4,         ///< 元数据包类型
            kPacketTypeMeta3 = 8,        ///< 元数据包类型3
            kFrameTypeKeyFrame = 16,     ///< 关键帧类型（I帧）
            kFrameTypeIDR = 32,          ///< IDR帧类型（立即解码刷新帧）
            kPacketTypeUnknowed = 255,   ///< 未知类型
        };
       // class Packet;
        //using PacketPtr = std::shared_ptr<Packet>;
#pragma pack(push)
#pragma pack(1)
        /**
        *  @author chensong
        *  @date 2025-04-29
        *  @brief 媒体数据包类（Media Packet Class）
        *  
        *  Packet类用于表示一个媒体数据包（音频或视频包）。它包含包的元数据（类型、大小、时间戳等）
        *  和数据缓冲区。Packet类使用紧凑的内存布局（#pragma pack(1)），数据缓冲区紧跟在Packet对象之后。
        *  
        *  数据包数据结构（Packet Data Structure）：
        *  
        *    0                   1                   2                   3
        *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   |  type_         |  size_          |  index_                     |
        *   |  (32 bits)     |  (32 bits)      |  (32 bits)                  |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   |  timestamp_                                                    |
        *   |  (64 bits)                                                      |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   |  capacity_                                                     |
        *   |  (32 bits)                                                      |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   |  ext_ (shared_ptr, variable size)                              |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   :                        Packet Data Buffer                      :
        *   |                        (capacity_ bytes)                        |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *  
        *  内存布局说明（Memory Layout）：
        *  
        *    0                   1                   2                   3
        *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   |  Packet Header (sizeof(Packet))                                |
        *   |  - type_, size_, index_, timestamp_, capacity_, ext_          |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   |  Packet Data Buffer (capacity_ bytes)                          |
        *   |  - Actual media data (audio/video)                              |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   |  Total size = sizeof(Packet) + capacity_                       |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *  
        *  数据包类型标识（Packet Type Identification）：
        *  
        *    0                   1                   2                   3
        *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *   |  type_ (32 bits)                                               |
        *   |  Bit 0: kPacketTypeVideo (1)                                   |
        *   |  Bit 1: kPacketTypeAudio (2)                                   |
        *   |  Bit 2: kPacketTypeMeta (4)                                    |
        *   |  Bit 3: kPacketTypeMeta3 (8)                                   |
        *   |  Bit 4: kFrameTypeKeyFrame (16)                                |
        *   |  Bit 5: kFrameTypeIDR (32)                                     |
        *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        *  
        *  @note Packet类使用紧凑内存布局（#pragma pack(1)），数据缓冲区紧跟在对象之后
        *  @note 数据包总内存 = sizeof(Packet) + capacity_
        *  @note 可以通过Data()方法获取数据缓冲区指针
        *  @note 支持扩展数据（ext_）用于存储额外的元数据
        *  
        *  使用示例：
        *  @code
        *  auto packet = Packet::NewPacket(1024);
        *  packet->SetPacketType(kPacketTypeVideo | kFrameTypeKeyFrame);
        *  packet->SetTimeStamp(timestamp);
        *  memcpy(packet->Data(), video_data, size);
        *  packet->SetPacketSize(size);
        *  @endcode
        */
        class Packet
        {
        public:
            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 构造函数（Constructor）
            *  
            *  该构造函数用于创建Packet实例。会分配包含Packet头和数据缓冲区的内存块。
            *  
            *  内存分配：
            *  1. 分配总内存：sizeof(Packet) + capacity_
            *  2. 在分配的内存块中构造Packet对象
            *  3. 数据缓冲区紧跟在Packet对象之后
            *  
            *  @param size 数据缓冲区的容量，单位为字节
            *  @note 数据缓冲区容量决定了可以存储的最大数据量
            *  @note 实际数据大小通过size_成员变量存储，可以小于capacity_
            */
            Packet(int32_t size)
            :capacity_(size)
            {

            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 析构函数（Destructor）
            *  
            *  该析构函数用于清理Packet实例。使用默认析构函数，自动释放成员变量。
            *  
            *  @note 使用默认析构函数，智能指针自动管理扩展数据
            */
            ~Packet() {}

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 创建新的数据包（Create New Packet）
            *  
            *  该静态方法用于创建并分配一个新的Packet实例。它会分配包含Packet头
            *  和数据缓冲区的连续内存块。
            *  
            *  内存分配流程（Memory Allocation Process）：
            *  
            *    0                   1                   2                   3
            *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |  1. Allocate memory block: sizeof(Packet) + size             |
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |  2. Construct Packet object at the start of memory block     |
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |  3. Initialize capacity_ = size                               |
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |  4. Return shared_ptr to Packet                                |
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *  
            *  @param size 数据缓冲区的容量，单位为字节
            *  @return 返回指向新创建的Packet对象的共享指针
            *  @note 该方法会分配连续的内存块，包含Packet头和数据缓冲区
            *  @note 使用shared_ptr管理内存，自动释放
            *  
            *  使用示例：
            *  @code
            *  auto packet = Packet::NewPacket(1024);
            *  // packet已分配，可以存储最多1024字节的数据
            *  @endcode
            */
            static std::shared_ptr<Packet> NewPacket(int32_t size);

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 检查是否为视频包（Check if Video Packet）
            *  
            *  该方法用于检查数据包是否为视频包。通过检查type_中是否包含
            *  kPacketTypeVideo标志来判断。
            *  
            *  @return 如果数据包是视频包，返回true；否则返回false
            *  @note 使用位运算检查type_ & kPacketTypeVideo
            */
            bool IsVideo() const
            {
                return (type_&kPacketTypeVideo)==kPacketTypeVideo;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 检查是否为关键帧（Check if Key Frame）
            *  
            *  该方法用于检查数据包是否为关键帧（I帧）。关键帧是视频编码中的
            *  独立帧，不需要参考其他帧即可解码。
            *  
            *  @return 如果数据包是视频包且是关键帧，返回true；否则返回false
            *  @note 同时检查是否为视频包和关键帧类型
            *  @note 关键帧包括I帧和IDR帧
            */
            bool IsKeyFrame() const
            {
                return ((type_&kPacketTypeVideo)==kPacketTypeVideo)
                        &&(type_&kFrameTypeKeyFrame)==kFrameTypeKeyFrame;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 检查是否为音频包（Check if Audio Packet）
            *  
            *  该方法用于检查数据包是否为音频包。通过检查type_是否等于
            *  kPacketTypeAudio来判断。
            *  
            *  @return 如果数据包是音频包，返回true；否则返回false
            *  @note 音频包的type_必须严格等于kPacketTypeAudio
            */
            bool IsAudio() const
            {
                return type_ == kPacketTypeAudio;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 检查是否为元数据包（Check if Meta Packet）
            *  
            *  该方法用于检查数据包是否为元数据包。通过检查type_是否等于
            *  kPacketTypeMeta来判断。
            *  
            *  @return 如果数据包是元数据包，返回true；否则返回false
            *  @note 元数据包通常包含编解码器配置信息（如SPS/PPS）
            */
            bool IsMeta() const
            {
                return type_ == kPacketTypeMeta;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 检查是否为元数据包类型3（Check if Meta3 Packet）
            *  
            *  该方法用于检查数据包是否为元数据包类型3。通过检查type_是否等于
            *  kPacketTypeMeta3来判断。
            *  
            *  @return 如果数据包是元数据包类型3，返回true；否则返回false
            */
            bool IsMeta3() const
            {
                return type_ == kPacketTypeMeta3;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 获取数据包大小（Get Packet Size）
            *  
            *  该方法用于获取数据包中实际存储的数据大小。这个大小表示
            *  数据缓冲区中已使用的字节数，不一定等于容量。
            *  
            *  @return 返回数据包中实际存储的数据大小，单位为字节
            *  @note 返回的大小是已使用的数据大小，而不是缓冲区容量
            */
            inline int32_t PacketSize() const
            {
                return size_;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 获取剩余空间（Get Remaining Space）
            *  
            *  该方法用于获取数据包中剩余的可用空间。剩余空间等于容量
            *  减去已使用的数据大小。
            *  
            *  @return 返回数据包中剩余的可用空间，单位为字节
            *  @note 剩余空间 = capacity_ - size_
            */
            inline int Space() const
            {
                return capacity_ - size_;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 设置数据包大小（Set Packet Size）
            *  
            *  该方法用于设置数据包中实际存储的数据大小。通常在使用Data()
            *  写入数据后调用此方法。
            *  
            *  @param len 数据大小，单位为字节，必须小于等于capacity_
            *  @note 设置的大小不应超过容量capacity_
            */
            inline void SetPacketSize(size_t len)
            {
                size_ = len;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 更新数据包大小（Update Packet Size）
            *  
            *  该方法用于增加数据包的大小。通常在追加数据时使用，
            *  将新增的数据大小添加到当前大小。
            *  
            *  @param len 要增加的数据大小，单位为字节
            *  @note 更新后的大小不应超过容量capacity_
            */
            inline void UpdatePacketSize(size_t len)
            {
                size_ += len;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 设置索引（Set Index）
            *  
            *  该方法用于设置数据包的索引。索引用于标识数据包在序列中的位置，
            *  通常用于帧序号或序列号。
            *  
            *  @param index 索引值，从0开始
            *  @note 索引用于标识数据包在序列中的顺序
            */
            void SetIndex(int32_t index)
            {
                index_ = index;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 获取索引（Get Index）
            *  
            *  该方法用于获取数据包的索引。索引用于标识数据包在序列中的位置。
            *  
            *  @return 返回数据包的索引值。如果未设置，返回-1
            */
            int32_t Index() const
            {
                return index_;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 设置数据包类型（Set Packet Type）
            *  
            *  该方法用于设置数据包的类型。类型可以是视频、音频、元数据等，
            *  也可以组合使用（如视频+关键帧）。
            *  
            *  类型设置格式（Type Setting Format）：
            *  
            *    0                   1                   2                   3
            *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |  type_ (32 bits)                                               |
            *   |  Bit flags:                                                     |
            *   |  - Bit 0: kPacketTypeVideo                                      |
            *   |  - Bit 1: kPacketTypeAudio                                      |
            *   |  - Bit 2: kPacketTypeMeta                                       |
            *   |  - Bit 3: kPacketTypeMeta3                                      |
            *   |  - Bit 4: kFrameTypeKeyFrame                                    |
            *   |  - Bit 5: kFrameTypeIDR                                         |
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *  
            *  @param type 数据包类型，可以是单个类型或类型组合（使用位或运算）
            *  @note 类型可以使用位或运算组合，如 kPacketTypeVideo | kFrameTypeKeyFrame
            */
            void SetPacketType(int32_t type)
            {
                type_ = type;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 获取数据包类型（Get Packet Type）
            *  
            *  该方法用于获取数据包的类型。类型值可以是单个类型或类型组合。
            *  
            *  @return 返回数据包的类型值
            *  @note 返回的类型值可能包含多个位标志的组合
            */
            int32_t PacketType() const
            {
                return type_;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 设置时间戳（Set Time Stamp）
            *  
            *  该方法用于设置数据包的时间戳。时间戳用于标识数据包的播放时间，
            *  通常以毫秒或90KHz时钟为单位。
            *  
            *  @param timestamp 时间戳值，单位为毫秒或90KHz时钟
            *  @note 时间戳用于同步音视频播放
            *  @note 90KHz时钟单位：1秒 = 90000个单位
            */
            void SetTimeStamp(uint64_t timestamp)
            {
                timestamp_ = timestamp;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 获取时间戳（Get Time Stamp）
            *  
            *  该方法用于获取数据包的时间戳。时间戳用于标识数据包的播放时间。
            *  
            *  @return 返回数据包的时间戳值，单位为毫秒或90KHz时钟
            */
            uint64_t TimeStamp() const
            {
                return timestamp_;
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 获取数据缓冲区指针（Get Data Buffer Pointer）
            *  
            *  该方法用于获取数据包的缓冲区指针。数据缓冲区紧跟在Packet对象之后，
            *  可以通过此指针直接访问和修改数据。
            *  
            *  数据缓冲区位置（Data Buffer Location）：
            *  
            *    0                   1                   2                   3
            *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |  Packet Object (sizeof(Packet) bytes)                         |
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |  Data Buffer (capacity_ bytes) <-- Data() returns pointer here|
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *  
            *  @return 返回指向数据缓冲区的字符指针
            *  @note 数据缓冲区紧跟在Packet对象之后
            *  @note 可以通过此指针直接访问和修改数据
            *  @note 数据大小通过size_成员变量控制
            *  
            *  使用示例：
            *  @code
            *  char* data = packet->Data();
            *  memcpy(data, source_data, size);
            *  packet->SetPacketSize(size);
            *  @endcode
            */
            inline char *Data()
            {
                return (char*)this+sizeof(Packet);
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 获取扩展数据（Get Extension Data）
            *  
            *  该方法用于获取数据包的扩展数据。扩展数据可以是任意类型的对象，
            *  用于存储额外的元数据。
            *  
            *  @tparam T 扩展数据的类型
            *  @return 返回指向扩展数据的共享指针，如果未设置则返回空指针
            *  @note 扩展数据通过SetExt()方法设置
            *  @note 使用模板方法进行类型转换
            *  
            *  使用示例：
            *  @code
            *  auto video_info = packet->Ext<VideoInfo>();
            *  if (video_info) {
            *      // 使用video_info
            *  }
            *  @endcode
            */
            template <typename T>            
            inline std::shared_ptr<T> Ext() const
            {
                return std::static_pointer_cast<T>(ext_);
            }

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 设置扩展数据（Set Extension Data）
            *  
            *  该方法用于设置数据包的扩展数据。扩展数据可以是任意类型的对象，
            *  用于存储额外的元数据。
            *  
            *  @param ext 指向扩展数据的共享指针
            *  @note 扩展数据可以是任意类型的对象
            *  @note 使用shared_ptr管理扩展数据，自动释放
            */
            inline void SetExt(const std::shared_ptr<void> &ext)
            {
                ext_ = ext;
            }

        private:
            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 数据包类型（Packet Type）
            *  
            *  该成员变量用于存储数据包的类型。类型可以是视频、音频、元数据等，
            *  也可以组合使用（使用位标志）。
            *  
            *  类型格式（Type Format）：
            *  
            *    0                   1                   2                   3
            *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |  type_ (32 bits)                                               |
            *   |  Bit flags for packet type and frame type                      |
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *  
            *  @note 初始值为kPacketTypeUnknowed，表示未知类型
            *  @note 类型可以使用位或运算组合
            */
            int32_t type_{kPacketTypeUnknowed};

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 数据大小（Data Size）
            *  
            *  该成员变量用于存储数据包中实际存储的数据大小。这个大小表示
            *  数据缓冲区中已使用的字节数，不一定等于容量。
            *  
            *  @note 初始值为0，表示尚未存储数据
            *  @note 大小不应超过容量capacity_
            */
            uint32_t size_{0};

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 索引（Index）
            *  
            *  该成员变量用于存储数据包的索引。索引用于标识数据包在序列中的位置，
            *  通常用于帧序号或序列号。
            *  
            *  @note 初始值为-1，表示尚未设置索引
            *  @note 索引通常从0开始，按顺序递增
            */
            int32_t index_{-1};

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 时间戳（Time Stamp）
            *  
            *  该成员变量用于存储数据包的时间戳。时间戳用于标识数据包的播放时间，
            *  通常以毫秒或90KHz时钟为单位。
            *  
            *  时间戳格式（Timestamp Format）：
            *  
            *    0                   1                   2                   3
            *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *   |                        timestamp_                              |
            *   |                        (64 bits)                                |
            *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            *  
            *  @note 初始值为0，表示尚未设置时间戳
            *  @note 时间戳单位通常是毫秒或90KHz时钟
            */
            uint64_t timestamp_{0};

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 缓冲区容量（Buffer Capacity）
            *  
            *  该成员变量用于存储数据缓冲区的容量。容量决定了可以存储的最大数据量，
            *  在创建Packet时设置。
            *  
            *  @note 容量在构造函数中设置，通常不会更改
            *  @note 实际数据大小size_不应超过容量
            */
            uint32_t capacity_{0};

            /**
            *  @author chensong
            *  @date 2025-04-29
            *  @brief 扩展数据（Extension Data）
            *  
            *  该成员变量用于存储数据包的扩展数据。扩展数据可以是任意类型的对象，
            *  用于存储额外的元数据（如编解码器信息、帧信息等）。
            *  
            *  @note 扩展数据通过shared_ptr管理，自动释放
            *  @note 扩展数据是可选的，可以为空
            *  @note 可以通过Ext<T>()方法获取特定类型的扩展数据
            */
            std::shared_ptr<void> ext_;
        };
#pragma pack()        
    }
}

#endif // _C_LIBMEDIA_TRNER_PASCKET______