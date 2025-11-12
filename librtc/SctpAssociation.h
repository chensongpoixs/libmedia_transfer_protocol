#ifndef MS_RTC_SCTP_ASSOCIATION_HPP
#define MS_RTC_SCTP_ASSOCIATION_HPP



/*
rtc_transfer 接口：

    1. dtls transfer application data received  接受 dtls的转发的数据 的处理 调用 sctp -> Process Sctp Data  数据
    2.  sctp assoication connecting 连接中的状态
    3. sctp assonication  connected 连接成功状态
    4. sctp  assonication failed 连接失败的状态
    5. sctp assonication closed  连接关闭状态
    6. sctp assonication  Send Data   Sctp 发送数据的接口  调用 dtls transfer 发送 SendApplicationData
    7. sctp assonication  Message Receivered 接受数据   1. 流id  2. ppid   3. 数据包
    8. sctp assonication  发送 dataChannel 数据  1. 流id 2.  ppid ， 3. 数据包



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

        class SctpStreamParameters
        {
        public:

             uint16_t streamId{ 0u };
            bool ordered{ true };
            uint16_t maxPacketLifeTime{ 0u };
             uint16_t maxRetransmits{ 0u };
        };

        class SctpAssociation
        {
        public:
            enum class SctpState
            {
                NEW = 1,
                CONNECTING,
                CONNECTED,
                FAILED,
                CLOSED
            };

        private:
            enum class StreamDirection
            {
                INCOMING = 1,
                OUTGOING
            };

        public:
            class Listener
            {
            public:
                virtual void OnSctpAssociationConnecting( SctpAssociation* sctpAssociation) = 0;
                virtual void OnSctpAssociationConnected( SctpAssociation* sctpAssociation) = 0;
                virtual void OnSctpAssociationFailed( SctpAssociation* sctpAssociation) = 0;
                virtual void OnSctpAssociationClosed( SctpAssociation* sctpAssociation) = 0;
                virtual void OnSctpAssociationSendData(
                     SctpAssociation* sctpAssociation, const uint8_t* data, size_t len) = 0;
                virtual void OnSctpAssociationMessageReceived(
                     SctpAssociation* sctpAssociation,
                     uint16_t streamId,
                    uint32_t ppid,
                    const uint8_t* msg,
                    size_t len) = 0;
            };

        public:
            static bool IsSctp(const uint8_t* data, size_t len)
            {
                // clang-format off
                return (
                    (len >= 12) &&
                    // Must have Source Port Number and Destination Port Number set to 5000 (hack).
                    ( Byte::Get2Bytes(data, 0) == 5000) &&
                    ( Byte::Get2Bytes(data, 2) == 5000)
                    );
                // clang-format on
            }

        public:
           explicit SctpAssociation(
                Listener* listener, uint16_t os,  uint16_t mis, size_t maxSctpMessageSize, bool isDataChannel);
            virtual ~SctpAssociation();

        public:
            void TransportConnected();
            size_t GetMaxSctpMessageSize() const
            {
                return this->maxSctpMessageSize;
            }
            SctpState GetState() const
            {
                return this->state;
            }
            void ProcessSctpData(const uint8_t* data, size_t len);
            void SendSctpMessage(const  SctpStreamParameters& params, uint32_t ppid, const uint8_t* msg, size_t len);
            void HandleDataConsumer(const  SctpStreamParameters& params);
            void DataProducerClosed(const  SctpStreamParameters& params);
            void DataConsumerClosed(const  SctpStreamParameters& params);

        private:
            void ResetSctpStream( uint16_t streamId, StreamDirection);
            void AddOutgoingStreams(bool force = false);

        public:
            /* Callbacks fired by usrsctp events. */
            virtual void OnUsrSctpSendSctpData(void* buffer, size_t len);
            virtual void OnUsrSctpReceiveSctpData( uint16_t streamId,  uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len);
            virtual void OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len);

            void OnUsrSctpSentData(uint32_t freeBuffer);
        private:
            // Passed by argument.
            Listener* listener{ nullptr };
            uint16_t os{ 1024u };
            uint16_t mis{ 1024u };
            size_t maxSctpMessageSize{ 262144u };
            bool isDataChannel{ false };
            // Allocated by this.
            uint8_t* messageBuffer{ nullptr };
            // Others.
            SctpState state{ SctpState::NEW };
            struct socket* socket{ nullptr };
            uint16_t desiredOs{ 0u };
            size_t messageBufferLen{ 0u };
            uint16_t lastSsnReceived{ 0u }; // Valid for us since no SCTP I-DATA support.
            std::shared_ptr<SctpEnv> _env;
        };

        //保证线程安全
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
            void OnUsrSctpReceiveSctpData( uint16_t streamId,  uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len) override;
            void OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len) override;

        private:
            rtc::Thread* worker_thread_;
        };
    }
} // namespace RTC
 
#endif //MS_RTC_SCTP_ASSOCIATION_HPP
