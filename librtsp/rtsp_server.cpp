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

#include "libmedia_transfer_protocol/librtsp/rtsp_server.h"
#include "libmedia_transfer_protocol/libnetwork/connection.h"
#include "rtc_base/logging.h"
#include "rtc_base/string_encode.h"
#include "rtc_base/string_utils.h"
#include <sstream>
#include <random>
#include <ctime>
#include <algorithm>
#include <cctype>



namespace libmedia_transfer_protocol {
	namespace librtsp {
		namespace {
			// 辅助函数：检查字符串是否以指定前缀开头
			bool starts_with(const std::string& str, const std::string& prefix) {
				return str.size() >= prefix.size() &&
					str.compare(0, prefix.size(), prefix) == 0;
			}
		}
		RtspServer::RtspServer()
			: tcp_server_(std::make_unique<libmedia_transfer_protocol::libnetwork::TcpServer>())
		{
			tcp_server_->SignalOnNewConnection.connect(this, &RtspServer::OnNewConnection);
			tcp_server_->SignalOnRecv.connect(this, &RtspServer::OnRecv);
			tcp_server_->SignalOnDestory.connect(this, &RtspServer::OnDestroy);
		}

		RtspServer::~RtspServer()
		{
			Shutdown();
		}

		bool RtspServer::Startup(const std::string& ip, uint16_t port)
		{
			return tcp_server_->Startup(ip, port);
		}

		void RtspServer::Shutdown()
		{
			std::lock_guard<std::mutex> lock(sessions_lock_);
			sessions_.clear();
		}

		void RtspServer::SetStreamInfo(const std::string& stream_path, const std::string& sdp_content)
		{
			default_sdp_content_ = sdp_content;
		}

		void RtspServer::OnNewConnection(libmedia_transfer_protocol::libnetwork::Connection* conn)
		{
			RTC_LOG(LS_INFO) << "RTSP server new connection";
			std::lock_guard<std::mutex> lock(sessions_lock_);
			auto session = std::make_shared<RtspServerSession>(conn);
			sessions_[conn] = session;
		}

		void RtspServer::OnRecv(libmedia_transfer_protocol::libnetwork::Connection* conn, const rtc::CopyOnWriteBuffer& data)
		{
			std::lock_guard<std::mutex> lock(sessions_lock_);
			auto it = sessions_.find(conn);
			if (it != sessions_.end())
			{
				it->second->OnRecv(data);
			}
		}

		void RtspServer::OnDestroy(libmedia_transfer_protocol::libnetwork::Connection* conn)
		{
			RTC_LOG(LS_INFO) << "RTSP server connection destroyed";
			std::lock_guard<std::mutex> lock(sessions_lock_);
			sessions_.erase(conn);
		}

		// RtspServerSession implementation
		RtspServerSession::RtspServerSession(libmedia_transfer_protocol::libnetwork::Connection* conn)
			: connection_(conn)
			, cseq_(0)
			, recv_buffer_(4096)
			, recv_buffer_size_(0)
			, is_playing_(false)
		{
			session_id_ = GenerateSessionId();
		}

		RtspServerSession::~RtspServerSession()
		{
		}

		void RtspServerSession::OnRecv(const rtc::CopyOnWriteBuffer& data)
		{
			// 追加数据到接收缓冲区
			if (recv_buffer_size_ + data.size() > recv_buffer_.capacity())
			{
				RTC_LOG(LS_WARNING) << "RTSP recv buffer overflow";
				recv_buffer_size_ = 0;
			}

			memcpy(recv_buffer_.data() + recv_buffer_size_, data.data(), data.size());
			recv_buffer_size_ += data.size();

			// 查找完整的 RTSP 请求（以 \r\n\r\n 结尾）
			std::string request_str((char*)recv_buffer_.data(), recv_buffer_size_);
			size_t request_end = request_str.find("\r\n\r\n");

			while (request_end != std::string::npos)
			{
				std::string request = request_str.substr(0, request_end);
				HandleRequest(request);

				// 移除已处理的请求
				size_t next_start = request_end + 4;
				if (next_start < recv_buffer_size_)
				{
					memmove(recv_buffer_.data(), recv_buffer_.data() + next_start, recv_buffer_size_ - next_start);
					recv_buffer_size_ -= next_start;
					request_str = std::string((char*)recv_buffer_.data(), recv_buffer_size_);
					request_end = request_str.find("\r\n\r\n");
				}
				else
				{
					recv_buffer_size_ = 0;
					break;
				}
			}
		}

		void RtspServerSession::HandleRequest(const std::string& request)
		{
			std::vector<std::string> lines;
			rtc::split(request, '\n', &lines);
			if (lines.empty())
			{
				return;
			}

			// 解析请求行
			std::vector<std::string> request_line;
			rtc::split(lines[0], ' ', &request_line);
			if (request_line.size() < 2)
			{
				SendResponse(400, "Bad Request");
				return;
			}

			std::string method = request_line[0];
			std::string url = request_line[1];

			// 解析 CSeq
			for (const auto& line : lines)
			{
				if (starts_with(line, "CSeq:"))
				{
					std::vector<std::string> parts;
					rtc::split(line, ':', &parts);
					if (parts.size() >= 2)
					{
						cseq_ = std::atoi(rtc::string_trim(parts[1]).c_str());
					}
				}
			}

			// 根据方法分发处理
			if (method == "OPTIONS")
			{
				HandleOptions(request);
			}
			else if (method == "DESCRIBE")
			{
				HandleDescribe(request);
			}
			else if (method == "SETUP")
			{
				HandleSetup(request);
			}
			else if (method == "PLAY")
			{
				HandlePlay(request);
			}
			else if (method == "TEARDOWN")
			{
				HandleTeardown(request);
			}
			else if (method == "GET_PARAMETER")
			{
				HandleGetParameter(request);
			}
			else
			{
				SendResponse(501, "Not Implemented");
			}
		}

		void RtspServerSession::HandleOptions(const std::string& request)
		{
			std::map<std::string, std::string> headers;
			headers["Public"] = "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN, GET_PARAMETER";
			SendResponse(200, "OK", headers);
		}

		void RtspServerSession::HandleDescribe(const std::string& request)
		{
			std::map<std::string, std::string> headers;
			headers["Content-Type"] = "application/sdp";
			
			std::string sdp = GenerateSdp(stream_path_);
			SendResponse(200, "OK", headers, sdp);
		}

		void RtspServerSession::HandleSetup(const std::string& request)
		{
			std::map<std::string, std::string> headers;
			headers["Transport"] = "RTP/AVP/TCP;unicast;interleaved=0-1";
			headers["Session"] = session_id_;
			SendResponse(200, "OK", headers);
		}

		void RtspServerSession::HandlePlay(const std::string& request)
		{
			is_playing_ = true;
			std::map<std::string, std::string> headers;
			headers["Session"] = session_id_;
			headers["Range"] = "npt=0.000-";
			SendResponse(200, "OK", headers);
		}

		void RtspServerSession::HandleTeardown(const std::string& request)
		{
			is_playing_ = false;
			std::map<std::string, std::string> headers;
			headers["Session"] = session_id_;
			SendResponse(200, "OK", headers);
		}

		void RtspServerSession::HandleGetParameter(const std::string& request)
		{
			std::map<std::string, std::string> headers;
			headers["Session"] = session_id_;
			SendResponse(200, "OK", headers);
		}

		void RtspServerSession::SendResponse(int status_code, const std::string& status_text,
			const std::map<std::string, std::string>& headers, const std::string& body)
		{
			std::stringstream response;
			response << "RTSP/1.0 " << status_code << " " << status_text << "\r\n";
			response << "CSeq: " << cseq_ << "\r\n";
			response << "Server: CRTC/1.0\r\n";

			for (const auto& header : headers)
			{
				response << header.first << ": " << header.second << "\r\n";
			}

			if (!body.empty())
			{
				response << "Content-Length: " << body.size() << "\r\n";
			}

			response << "\r\n";
			if (!body.empty())
			{
				response << body;
			}

			std::string response_str = response.str();
			rtc::CopyOnWriteBuffer buffer(response_str.data(), response_str.size());
			if (connection_)
			{
				connection_->Send(buffer);
			}
		}

		std::string RtspServerSession::GenerateSessionId()
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(1000000000, 9999999999);
			return std::to_string(dis(gen));
		}

		std::string RtspServerSession::GenerateSdp(const std::string& stream_path)
		{
			std::stringstream sdp;
			sdp << "v=0\r\n";
			sdp << "o=- 0 0 IN IP4 127.0.0.1\r\n";
			sdp << "s=CRTC RTSP Stream\r\n";
			sdp << "t=0 0\r\n";
			sdp << "m=video 0 RTP/AVP 96\r\n";
			sdp << "a=rtpmap:96 H264/90000\r\n";
			sdp << "a=fmtp:96 packetization-mode=1;profile-level-id=42001f;sprop-parameter-sets=Z0IAHpWoKA9puAgICBA=,aM48gA==\r\n";
			sdp << "a=control:trackID=0\r\n";
			return sdp.str();
		}
	}
}

