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
        enum
        {
            kPacketTypeVideo = 1,
            kPacketTypeAudio = 2,
            kPacketTypeMeta = 4,
            kPacketTypeMeta3 = 8,
            kFrameTypeKeyFrame = 16,
            kFrameTypeIDR = 32,
            kPacketTypeUnknowed = 255,
        };
       // class Packet;
        //using PacketPtr = std::shared_ptr<Packet>;
#pragma pack(push)
#pragma pack(1)
        class Packet
        {
        public:
            Packet(int32_t size)
            :capacity_(size)
            {

            }
            ~Packet() {}
            static std::shared_ptr<Packet> NewPacket(int32_t size);

            bool IsVideo() const
            {
                return (type_&kPacketTypeVideo)==kPacketTypeVideo;
            }
            bool IsKeyFrame() const
            {
                return ((type_&kPacketTypeVideo)==kPacketTypeVideo)
                        &&(type_&kFrameTypeKeyFrame)==kFrameTypeKeyFrame;
            }
            bool IsAudio() const
            {
                return type_ == kPacketTypeAudio;
            }
            bool IsMeta() const
            {
                return type_ == kPacketTypeMeta;
            }
            bool IsMeta3() const
            {
                return type_ == kPacketTypeMeta3;
            }

            inline int32_t PacketSize() const
            {
                return size_;
            }
            inline int Space() const
            {
                return capacity_ - size_;
            }
            inline void SetPacketSize(size_t len)
            {
                size_ = len;
            }
            inline void UpdatePacketSize(size_t len)
            {
                size_ += len;
            }
            void SetIndex(int32_t index)
            {
                index_ = index;
            }
            int32_t Index() const
            {
                return index_;
            }
            void SetPacketType(int32_t type)
            {
                type_ = type;
            }
            int32_t PacketType() const
            {
                return type_;
            }
            void SetTimeStamp(uint64_t timestamp)
            {
                timestamp_ = timestamp;
            }
            uint64_t TimeStamp() const
            {
                return timestamp_;
            }
            inline char *Data()
            {
                return (char*)this+sizeof(Packet);
            }

            template <typename T>            
            inline std::shared_ptr<T> Ext() const
            {
                return std::static_pointer_cast<T>(ext_);
            }
            inline void SetExt(const std::shared_ptr<void> &ext)
            {
                ext_ = ext;
            }

        private:
            int32_t type_{kPacketTypeUnknowed};
            uint32_t size_{0};
            int32_t index_{-1};
            uint64_t timestamp_{0};
            uint32_t capacity_{0};
            std::shared_ptr<void> ext_;
        };
#pragma pack()        
    }
}

#endif // _C_LIBMEDIA_TRNER_PASCKET______