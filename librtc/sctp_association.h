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
created: 		2025-11-11

author:			chensong

purpose:		SCTP关联管理（SCTP Association Management）


SCTP协议说明：
- SCTP（Stream Control Transmission Protocol）是流控制传输协议
- SCTP用于WebRTC的DataChannel数据传输
- SCTP运行在DTLS之上，提供可靠或不可靠的数据传输
- SCTP支持多流复用、消息边界保护、有序/无序传输

SCTP在WebRTC中的位置（SCTP in WebRTC Stack）：

    Application Layer
         |
         v
    DataChannel API
         |
         v
    SCTP Association  <--- 本文件实现
         |
         v
    DTLS Transport
         |
         v
    UDP/ICE Transport

SCTP连接建立流程（SCTP Connection Flow）：

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
      |                                |
      | DataChannel Open              |
      |<=============================>|

SCTP数据包格式（SCTP Packet Format）：

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |     Source Port Number        |     Destination Port Number   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                      Verification Tag                         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                           Checksum                            |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    |                          Chunk Data                           |
    |                             ...                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

DataChannel传输模式：
- 可靠有序：maxRetransmits = 无限，ordered = true
- 可靠无序：maxRetransmits = 无限，ordered = false
- 不可靠有序：maxRetransmits = N，ordered = true
- 不可靠无序：maxRetransmits = N，ordered = false


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


#ifndef _C_LIBRTC_SCTP_ASSOCIATION__H_
#define _C_LIBRTC_SCTP_ASSOCIATION__H_

#include <cstdint>
#include "usrsctp.h"
#include <unordered_map>




 /*
 rtc_transfer �ӿڣ�

     1. dtls transfer application data received  ���� dtls��ת�������� �Ĵ��� ���� sctp -> Process Sctp Data  ����
     2.  sctp assoication connecting �����е�״̬
     3. sctp assonication  connected ���ӳɹ�״̬
     4. sctp  assonication failed ����ʧ�ܵ�״̬
     5. sctp assonication closed  ���ӹر�״̬
     6. sctp assonication  Send Data   Sctp �������ݵĽӿ�  ���� dtls transfer ���� SendApplicationData
     7. sctp assonication  Message Receivered ��������   1. ��id  2. ppid   3. ���ݰ�
     8. sctp assonication  ���� dataChannel ����  1. ��id 2.  ppid �� 3. ���ݰ�



 */
 //#ifdef ENABLE_SCTP
 //#include <usrsctp.h>
 // 
 // 
 // 
 //#include "Utils.hpp"
 //#include "Poller/EventPoller.h"
#include <memory>
#include <cstdint>
#include <cstdbool>
#include <cstdbool>
#include <cstdalign>
#include <cstdarg>
#include "usrsctp.h"

#include "libmedia_transfer_protocol/librtc/rtc_utils.h"
#include "rtc_base/thread.h"



namespace libmedia_transfer_protocol
{
    namespace librtc {
        class SctpEnv;

        /**
        *  @brief SCTP流参数（SCTP Stream Parameters）
        *  
        *  该类定义了SCTP流的参数，用于配置DataChannel的传输特性。
        *  
        *  参数说明：
        *  - streamId: 流ID，范围0-65535
        *  - ordered: 是否有序传输
        *  - maxPacketLifeTime: 最大数据包生存时间（毫秒），0表示无限制
        *  - maxRetransmits: 最大重传次数，0表示无限制
        *  
        *  传输模式组合：
        *  1. 可靠有序：ordered=true, maxRetransmits=0
        *  2. 可靠无序：ordered=false, maxRetransmits=0
        *  3. 不可靠有序：ordered=true, maxRetransmits=N
        *  4. 不可靠无序：ordered=false, maxRetransmits=N
        *  
        *  @note maxPacketLifeTime和maxRetransmits不能同时设置
        */
        class SctpStreamParameters
        {
        public:
            uint16_t streamId{ 0u };           ///< SCTP流ID
            bool ordered{ true };              ///< 是否有序传输
            uint16_t maxPacketLifeTime{ 0u };  ///< 最大数据包生存时间（毫秒），0表示无限制
            uint16_t maxRetransmits{ 0u };     ///< 最大重传次数，0表示无限制
        };

        /**
        *  @author chensong
        *  @date 2025-11-11
        *  @brief SCTP关联管理类（SCTP Association Manager）
        *  
        *  SctpAssociation类用于管理SCTP关联，提供WebRTC DataChannel的底层传输。
        *  它封装了usrsctp库，为DataChannel提供可靠或不可靠的数据传输。
        *  
        *  主要功能：
        *  1. SCTP关联建立：发起或接受SCTP连接
        *  2. 数据传输：发送和接收SCTP数据包
        *  3. 流管理：管理多个SCTP流（streamId）
        *  4. 状态管理：跟踪SCTP连接状态
        *  5. 事件通知：通知上层SCTP事件
        *  
        *  工作流程：
        *  1. DTLS握手完成
        *  2. 创建SctpAssociation
        *  3. 调用TransportConnected()开始SCTP握手
        *  4. SCTP连接建立（INIT/INIT-ACK/COOKIE-ECHO/COOKIE-ACK）
        *  5. 创建DataChannel，分配streamId
        *  6. 发送/接收数据
        *  
        *  @note SCTP运行在DTLS之上
        *  @note 支持多流复用（最多65535个流）
        *  @note 使用固定端口5000（WebRTC约定）
        *  
        *  使用示例：
        *  @code
        *  class MyListener : public SctpAssociation::Listener {
        *      void OnSctpAssociationConnected(SctpAssociation* sctp) override {
        *          // SCTP连接建立
        *      }
        *      void OnSctpAssociationMessageReceived(
        *          SctpAssociation* sctp, uint16_t streamId, 
        *          uint32_t ppid, const uint8_t* msg, size_t len) override {
        *          // 接收到DataChannel消息
        *      }
        *  };
        *  
        *  MyListener listener;
        *  SctpAssociation sctp(&listener, 1024, 1024, 262144, true);
        *  
        *  // DTLS连接建立后
        *  sctp.TransportConnected();
        *  
        *  // 发送DataChannel消息
        *  SctpStreamParameters params;
        *  params.streamId = 0;
        *  params.ordered = true;
        *  sctp.SendSctpMessage(params, PPID_WEBRTC_STRING, data, len);
        *  
        *  // 接收DTLS传来的SCTP数据
        *  sctp.ProcessSctpData(data, len);
        *  @endcode
        */
        class SctpAssociation
        {
        public:
            /**
            *  @brief SCTP状态枚举（SCTP State）
            *  
            *  该枚举定义了SCTP关联的状态。
            *  
            *  状态说明：
            *  - NEW: 初始状态，未开始连接
            *  - CONNECTING: 连接中（INIT/INIT-ACK/COOKIE-ECHO）
            *  - CONNECTED: 连接已建立
            *  - FAILED: 连接失败
            *  - CLOSED: 连接已关闭
            */
            enum class SctpState
            {
                NEW = 1,      ///< 初始状态
                CONNECTING,   ///< 连接中
                CONNECTED,    ///< 已连接
                FAILED,       ///< 连接失败
                CLOSED        ///< 已关闭
            };

        private:
            /**
            *  @brief 流方向枚举（Stream Direction）
            *  
            *  该枚举定义了SCTP流的方向。
            *  
            *  方向说明：
            *  - INCOMING: 入站流（接收）
            *  - OUTGOING: 出站流（发送）
            */
            enum class StreamDirection
            {
                INCOMING = 1,  ///< 入站流
                OUTGOING       ///< 出站流
            };

        public:
            /**
            *  @brief SCTP关联监听器接口（SCTP Association Listener）
            *  
            *  该接口定义了SCTP关联事件的回调方法。上层需要实现此接口来处理SCTP事件。
            *  
            *  事件说明：
            *  - OnSctpAssociationConnecting: SCTP连接正在建立
            *  - OnSctpAssociationConnected: SCTP连接已建立
            *  - OnSctpAssociationFailed: SCTP连接失败
            *  - OnSctpAssociationClosed: SCTP连接已关闭
            *  - OnSctpAssociationSendData: SCTP需要通过DTLS发送数据
            *  - OnSctpAssociationMessageReceived: 接收到SCTP消息
            */
            class Listener
            {
            public:
                /**
                *  @brief SCTP连接正在建立回调
                *  @param sctpAssociation SCTP关联对象指针
                */
                virtual void OnSctpAssociationConnecting(SctpAssociation* sctpAssociation) = 0;
                
                /**
                *  @brief SCTP连接已建立回调
                *  @param sctpAssociation SCTP关联对象指针
                */
                virtual void OnSctpAssociationConnected(SctpAssociation* sctpAssociation) = 0;
                
                /**
                *  @brief SCTP连接失败回调
                *  @param sctpAssociation SCTP关联对象指针
                */
                virtual void OnSctpAssociationFailed(SctpAssociation* sctpAssociation) = 0;
                
                /**
                *  @brief SCTP连接已关闭回调
                *  @param sctpAssociation SCTP关联对象指针
                */
                virtual void OnSctpAssociationClosed(SctpAssociation* sctpAssociation) = 0;
                
                /**
                *  @brief SCTP需要发送数据回调
                *  
                *  该回调用于通知上层将SCTP数据包通过DTLS发送出去。
                *  
                *  @param sctpAssociation SCTP关联对象指针
                *  @param data SCTP数据包指针
                *  @param len SCTP数据包长度
                *  @note 上层需要调用Dtls::SendApplicationData()发送数据
                */
                virtual void OnSctpAssociationSendData(
                    SctpAssociation* sctpAssociation, const uint8_t* data, size_t len) = 0;
                
                /**
                *  @brief 接收到SCTP消息回调
                *  
                *  该回调用于通知上层接收到DataChannel消息。
                *  
                *  @param sctpAssociation SCTP关联对象指针
                *  @param streamId SCTP流ID
                *  @param ppid 负载协议标识符（PPID）
                *                - 50: WEBRTC_DCEP（DataChannel建立协议）
                *                - 51: WEBRTC_STRING（UTF-8字符串）
                *                - 53: WEBRTC_BINARY（二进制数据）
                *  @param msg 消息数据指针
                *  @param len 消息长度
                */
                virtual void OnSctpAssociationMessageReceived(
                    SctpAssociation* sctpAssociation,
                    uint16_t streamId,
                    uint32_t ppid,
                    const uint8_t* msg,
                    size_t len) = 0;
            };

        public:
            /**
            *  @brief 判断是否为SCTP数据包（Is SCTP Packet）
            *  
            *  该静态方法用于判断DTLS应用数据是否为SCTP数据包。
            *  
            *  判断条件：
            *  1. 数据长度至少12字节（SCTP头部）
            *  2. 源端口和目标端口都为5000（WebRTC约定）
            *  
            *  @param data 数据指针
            *  @param len 数据长度
            *  @return 如果是SCTP数据包返回true，否则返回false
            *  @note WebRTC使用固定端口5000，这是约定
            */
            static bool IsSctp(const uint8_t* data, size_t len)
            {
                // clang-format off
                return (
                    (len >= 12) &&
                    // Must have Source Port Number and Destination Port Number set to 5000 (hack).
                    (Byte::Get2Bytes(data, 0) == 5000) &&
                    (Byte::Get2Bytes(data, 2) == 5000)
                    );
                // clang-format on
            }

        public:
            /**
            *  @brief SCTP关联构造函数
            *  
            *  该构造函数用于创建SCTP关联，初始化usrsctp套接字。
            *  
            *  初始化流程：
            *  1. 保存参数（监听器、流数量、最大消息大小）
            *  2. 创建usrsctp套接字
            *  3. 设置套接字选项（非阻塞、发送缓冲区等）
            *  4. 绑定本地地址（端口5000）
            *  5. 注册usrsctp回调
            *  
            *  @param listener SCTP事件监听器
            *  @param os 出站流数量（Outbound Streams）
            *  @param mis 入站流数量（Max Inbound Streams）
            *  @param maxSctpMessageSize 最大SCTP消息大小（字节）
            *  @param isDataChannel 是否为DataChannel模式
            *  @note os和mis通常设置为1024
            *  @note maxSctpMessageSize通常设置为262144（256KB）
            */
            explicit SctpAssociation(
                Listener* listener, uint16_t os, uint16_t mis, size_t maxSctpMessageSize, bool isDataChannel);
            
            /**
            *  @brief SCTP关联析构函数
            *  
            *  该析构函数用于销毁SCTP关联，关闭usrsctp套接字。
            */
            virtual ~SctpAssociation();

        public:
            /**
            *  @brief 传输已连接（Transport Connected）
            *  
            *  该方法用于通知SCTP关联底层传输（DTLS）已连接，开始SCTP握手。
            *  
            *  SCTP握手流程：
            *  1. 调用usrsctp_connect()发起SCTP INIT
            *  2. 接收SCTP INIT-ACK
            *  3. 发送SCTP COOKIE-ECHO
            *  4. 接收SCTP COOKIE-ACK
            *  5. SCTP连接建立
            *  
            *  @note 必须在DTLS握手完成后调用
            */
            void TransportConnected();
            
            /**
            *  @brief 获取最大SCTP消息大小（Get Max SCTP Message Size）
            *  
            *  该方法用于获取SCTP支持的最大消息大小。
            *  
            *  @return 返回最大SCTP消息大小（字节）
            */
            size_t GetMaxSctpMessageSize() const
            {
                return this->maxSctpMessageSize;
            }
            
            /**
            *  @brief 获取SCTP状态（Get SCTP State）
            *  
            *  该方法用于获取当前SCTP连接状态。
            *  
            *  @return 返回SCTP状态枚举值
            */
            SctpState GetState() const
            {
                return this->state;
            }
            
            /**
            *  @brief 处理SCTP数据（Process SCTP Data）
            *  
            *  该方法用于处理从DTLS接收的SCTP数据包。
            *  
            *  处理流程：
            *  1. 验证SCTP数据包格式
            *  2. 调用usrsctp_conninput()输入数据
            *  3. usrsctp触发回调处理数据
            *  
            *  @param data SCTP数据包指针
            *  @param len SCTP数据包长度
            *  @note 此方法在接收到DTLS应用数据时调用
            */
            void ProcessSctpData(const uint8_t* data, size_t len);
            
            /**
            *  @brief 发送SCTP消息（Send SCTP Message）
            *  
            *  该方法用于通过SCTP发送DataChannel消息。
            *  
            *  发送流程：
            *  1. 构建SCTP发送参数（streamId、PPID、有序性等）
            *  2. 调用usrsctp_sendv()发送消息
            *  3. usrsctp触发OnUsrSctpSendSctpData()回调
            *  4. 通过DTLS发送SCTP数据包
            *  
            *  @param params SCTP流参数
            *  @param ppid 负载协议标识符（PPID）
            *  @param msg 消息数据指针
            *  @param len 消息长度
            *  @note 消息长度不能超过maxSctpMessageSize
            */
            void SendSctpMessage(const  SctpStreamParameters& params, uint32_t ppid, const uint8_t* msg, size_t len);
            
            /**
            *  @brief 处理数据消费者（Handle Data Consumer）
            *  
            *  该方法用于处理新的数据消费者，重置SCTP流。
            *  
            *  @param params SCTP流参数
            */
            void HandleDataConsumer(const  SctpStreamParameters& params);
            
            /**
            *  @brief 数据生产者关闭（Data Producer Closed）
            *  
            *  该方法用于通知数据生产者关闭，重置出站流。
            *  
            *  @param params SCTP流参数
            */
            void DataProducerClosed(const  SctpStreamParameters& params);
            
            /**
            *  @brief 数据消费者关闭（Data Consumer Closed）
            *  
            *  该方法用于通知数据消费者关闭，重置入站流。
            *  
            *  @param params SCTP流参数
            */
            void DataConsumerClosed(const  SctpStreamParameters& params);

        private:
            /**
            *  @brief 重置SCTP流（Reset SCTP Stream）
            *  
            *  该方法用于重置指定方向的SCTP流。
            *  
            *  @param streamId SCTP流ID
            *  @param direction 流方向（INCOMING或OUTGOING）
            */
            void ResetSctpStream(uint16_t streamId, StreamDirection);
            
            /**
            *  @brief 添加出站流（Add Outgoing Streams）
            *  
            *  该方法用于动态添加出站流。
            *  
            *  @param force 是否强制添加
            */
            void AddOutgoingStreams(bool force = false);

        public:
            /* usrsctp事件回调（Callbacks fired by usrsctp events） */
            
            /**
            *  @brief usrsctp发送SCTP数据回调
            *  
            *  该虚拟方法用于处理usrsctp需要发送的SCTP数据包。
            *  
            *  @param buffer SCTP数据缓冲区指针
            *  @param len SCTP数据长度
            *  @note 此方法会触发OnSctpAssociationSendData()通知上层发送
            */
            virtual void OnUsrSctpSendSctpData(void* buffer, size_t len);
            
            /**
            *  @brief usrsctp接收SCTP数据回调
            *  
            *  该虚拟方法用于处理usrsctp接收到的SCTP数据。
            *  
            *  @param streamId SCTP流ID
            *  @param ssn 流序列号（Stream Sequence Number）
            *  @param ppid 负载协议标识符（PPID）
            *  @param flags 标志位（MSG_EOR等）
            *  @param data 数据指针
            *  @param len 数据长度
            *  @note 此方法会触发OnSctpAssociationMessageReceived()通知上层
            */
            virtual void OnUsrSctpReceiveSctpData(uint16_t streamId, uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len);
            
            /**
            *  @brief usrsctp接收SCTP通知回调
            *  
            *  该虚拟方法用于处理usrsctp的通知事件（连接建立、关闭等）。
            *  
            *  @param notification SCTP通知结构指针
            *  @param len 通知长度
            *  @note 通知类型包括SCTP_ASSOC_CHANGE、SCTP_STREAM_RESET_EVENT等
            */
            virtual void OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len);

            /**
            *  @brief usrsctp数据已发送回调
            *  
            *  该方法用于处理usrsctp的数据发送确认。
            *  
            *  @param freeBuffer 释放的缓冲区大小
            */
            void OnUsrSctpSentData(uint32_t freeBuffer);
        private:
            // Passed by argument.
            Listener* listener{ nullptr };               ///< SCTP事件监听器
            uint16_t os{ 1024u };                        ///< 出站流数量（Outbound Streams）
            uint16_t mis{ 1024u };                       ///< 入站流数量（Max Inbound Streams）
            size_t maxSctpMessageSize{ 262144u };        ///< 最大SCTP消息大小（256KB）
            bool isDataChannel{ false };                 ///< 是否为DataChannel模式
            // Allocated by this.
            uint8_t* messageBuffer{ nullptr };           ///< 消息缓冲区，用于接收分片消息
            // Others.
            SctpState state{ SctpState::NEW };           ///< SCTP连接状态
            struct socket* socket{ nullptr };            ///< usrsctp套接字
            uint16_t desiredOs{ 0u };                    ///< 期望的出站流数量
            size_t messageBufferLen{ 0u };               ///< 消息缓冲区当前长度
            uint16_t lastSsnReceived{ 0u };              ///< 最后接收的流序列号（因为不支持SCTP I-DATA，所以有效）
            std::shared_ptr<SctpEnv> _env;               ///< SCTP环境对象
        };

        //��֤�̰߳�ȫ
        class SctpAssociationImp : public SctpAssociation, public std::enable_shared_from_this<SctpAssociationImp> {
        public:
            using Ptr = std::shared_ptr<SctpAssociationImp>;
            //template<typename ... ARGS>
            //explicit SctpAssociationImp(rtc::Thread*  workder_thread, ARGS &&...args) 
            //    : SctpAssociation(std::forward<ARGS>(args)...) {
            //    worker_thread_ = workder_thread;
            //}
            explicit SctpAssociationImp(rtc::Thread* workder_thread,
                Listener* listener, uint16_t os, uint16_t mis, size_t maxSctpMessageSize, bool isDataChannel)
                : SctpAssociation(listener, os, mis, maxSctpMessageSize, isDataChannel)
                , worker_thread_(workder_thread)
            {

            }

            ~SctpAssociationImp() override = default;

        protected:
            void OnUsrSctpSendSctpData(void* buffer, size_t len) override;
            void OnUsrSctpReceiveSctpData(uint16_t streamId, uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len) override;
            void OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len) override;

        private:
            rtc::Thread* worker_thread_;
        };
    }
} // namespace RTC



#endif //  