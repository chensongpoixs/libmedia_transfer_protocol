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
 /*****************************************************************************
				   Author: chensong
				   date:  2025-10-18

 ******************************************************************************/

#ifndef _C_LIBRTSP_RTSP_SERVER_H_
#define _C_LIBRTSP_RTSP_SERVER_H_

#include <string>
#include <memory>
#include <unordered_map>
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "libmedia_transfer_protocol/libnetwork/tcp_server.h"
#include "libp2p_peerconnection/connection_context.h"
#include <mutex>
namespace libmedia_transfer_protocol {
	namespace librtsp {

		class RtspServerSession;
		class RtspServer : public sigslot::has_slots<>
		{
		public:
			explicit RtspServer();
			virtual ~RtspServer();

		public:
			bool Startup(const std::string& ip, uint16_t port);
			void Shutdown();

		public:
			// 设置流 URL 和 SDP 信息
			void SetStreamInfo(const std::string& stream_path, const std::string& sdp_content);

		private:
			void OnNewConnection(libmedia_transfer_protocol::libnetwork::Connection* conn);
			void OnRecv(libmedia_transfer_protocol::libnetwork::Connection* conn, const rtc::CopyOnWriteBuffer& data);
			void OnDestroy(libmedia_transfer_protocol::libnetwork::Connection* conn);

		private:
			std::unique_ptr<libmedia_transfer_protocol::libnetwork::TcpServer> tcp_server_;
			std::unordered_map<libmedia_transfer_protocol::libnetwork::Connection*, 
				std::shared_ptr<RtspServerSession>> sessions_;
			std::mutex sessions_lock_;
			std::string default_sdp_content_;
		};

		// RTSP 服务器会话，处理单个客户端连接
		class RtspServerSession : public sigslot::has_slots<>
		{
		public:
			explicit RtspServerSession(libmedia_transfer_protocol::libnetwork::Connection* conn);
			virtual ~RtspServerSession();

		public:
			void OnRecv(const rtc::CopyOnWriteBuffer& data);
			void SendResponse(int status_code, const std::string& status_text, 
				const std::map<std::string, std::string>& headers = {},
				const std::string& body = "");

		private:
			void HandleRequest(const std::string& request);
			void HandleOptions(const std::string& request);
			void HandleDescribe(const std::string& request);
			void HandleSetup(const std::string& request);
			void HandlePlay(const std::string& request);
			void HandleTeardown(const std::string& request);
			void HandleGetParameter(const std::string& request);

			std::string GenerateSessionId();
			std::string GenerateSdp(const std::string& stream_path);

		private:
			libmedia_transfer_protocol::libnetwork::Connection* connection_;
			std::string session_id_;
			int32_t cseq_;
			rtc::Buffer recv_buffer_;
			int32_t recv_buffer_size_;
			std::string stream_path_;
			bool is_playing_;
		};
	}
}

#endif // _C_LIBRTSP_RTSP_SERVER_H_

