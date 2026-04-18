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
				   date:  2025-10-14

single NAL Unit Mode:
当 packetization-mode 媒体类型参数的值等于 0，或者 packetization-mode 参数未出现时，使用此模式。所有接收端 必须 支持此模式。该模式主要用于与 ITU-T 推荐标准 H.241（见第 12.1 节）兼容的、低延迟应用场景。在此模式下，只能使用单一 NAL 单元包（Single NAL Unit Packets）。禁止使用 STAP（单时间聚合包）、MTAP（多时间聚合包）和 FU（分片单元）。发送时，单一 NAL 单元包的传输顺序必须遵循 NAL 单元的解码顺序。

non-interleaved mode:
当 packetization-mode（分包模式）这个可选的媒体类型参数的值为 1 时，使用此模式。此模式建议被支持。它主要面向低延迟应用场景。 在该模式下，允许使用以下类型的数据包：

● 单一 NAL 单元包（Single NAL Unit Packets）

● STAP-A（单时间聚合包 A）

● FU-A（分片单元 A）

而禁止使用以下类型的数据包：

● STAP-B（单时间聚合包 B）

● MTAP（多时间聚合包，包括 MTAP16 和 MTAP24）

● FU-B（分片单元 B）

同时，NAL 单元的发送顺序必须遵循 NAL 单元的解码顺序。

Interleaved Mode:
当 packetization-mode（分包模式）这个可选的媒体类型参数的值为 2 时，使用此模式。

部分接收端可以选择（MAY）支持此模式。

在该模式下，允许使用：

● STAP-B（单时间聚合包 B）

● MTAP（多时间聚合包，包括 MTAP16 和 MTAP24）

● FU-A（分片单元 A）

● FU-B（分片单元 B）

而禁止使用：

● STAP-A（单时间聚合包 A）

● 单一 NAL 单元包（Single NAL Unit Packets）
 

 ******************************************************************************/
#include "libmedia_transfer_protocol/librtc/rtc_sdp.h"
#include <vector>
#include "rtc_base/string_utils.h"
#include "rtc_base/string_encode.h"
#include "rtc_base/logging.h"
#include "libmedia_transfer_protocol/string_utils.h"
#include <iostream>
#include "libmedia_transfer_protocol/librtc/dtls_certs.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
namespace libmedia_transfer_protocol {
	namespace librtc
	{

		namespace {
			static const std::string rtpmap_token = "a=rtpmap:";
			static const std::string ice_ufrag_token = "a=ice-ufrag:";
			static const std::string ice_pwd_token = "a=ice-pwd:";
			static const std::string  fingerprint_token = "a=fingerprint:";
			static const std::string   role_token = "a=setup:";
			static const std::string   ssrc				= "a=ssrc:";
			static const std::string   ssrc_group	= "a=ssrc-group:";
			static const std::string   fmtp = "a=fmtp:";
			static const std::string application_token = "m=application";
			static const std::string  sctp_port_token = "a=sctp-port:";
			static const std::string  max_message_size_token = "a=max-mesage-size:";
		}
		/*
		
		audio : 
		
			a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
			a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
			a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
			a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid
		video:
			a=extmap:14 urn:ietf:params:rtp-hdrext:toffset
			a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
			a=extmap:13 urn:3gpp:video-orientation
			a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
			a=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
			a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
			a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
			a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space
			a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid
			a=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
			a=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id

		datachannel
			m=application 9 UDP/DTLS/SCTP webrtc-datachannel
			c=IN IP4 0.0.0.0
			a=ice-ufrag:QMlp
			a=ice-pwd:flokPS0swUVGfjizysa3zuL4
			a=ice-options:trickle
			a=fingerprint:sha-256 11:3D:8D:D7:E7:86:7E:4B:9D:0C:75:AF:60:CF:7D:88:AB:F2:5D:7E:15:A3:E5:A3:5E:C0:C4:B8:62:1F:44:EC
			a=setup:actpass
			a=mid:2
			a=sctp-port:5000
			a=max-message-size:262144
		*/
		RtcSdp::RtcSdp()
		: rtc_sdp_type_(kRtcSdpPlay)
		, data_channel_params_(){}
		RtcSdp::  ~RtcSdp() {}

		void RtcSdp::SetSdpType(RtcSdpType  rtc_sdp_type)
		{
			rtc_sdp_type_ = rtc_sdp_type;
		}
		RtcSdpType RtcSdp::GetSdpType() const
		{
			return rtc_sdp_type_;
		}
		bool RtcSdp::Decode(const std::string &sdp)
		{
			std::vector<std::string>    list;
			rtc::split(sdp, '\n', &list);
			if (list.size() < 0)
			{
				LIBRTC_LOG_T_F(LS_WARNING) << "parse sdp  failed !!! , line : " << list.size() << ", sdp : " << sdp;
				return false;
			}
			for (auto line : list)
			{
				//rtc::tokenize_first
				if (StringUtils::StartsWith(line, ice_ufrag_token))
				{
					remote_ufrag_ = line.substr(ice_pwd_token.size());
					LIBRTC_LOG(LS_INFO) << "remote ufrage:" << remote_ufrag_;
				}
				else if (StringUtils::StartsWith(line, ice_pwd_token))
				{
					remote_passwd_ = line.substr(ice_pwd_token.size());
					LIBRTC_LOG(LS_INFO) << "remote passwd:" << remote_passwd_;
				}
				else if (StringUtils::StartsWith(line, role_token))
				{
					remote_role_ = line.substr(role_token.size(), line.size() - role_token.size()  -1);
					LIBRTC_LOG(LS_INFO) << "remote role:" << remote_role_;
				}
				else if (StringUtils::StartsWith(line, fingerprint_token))
				{
					std::string remote_fingerprint = line.substr(fingerprint_token.size());
					
					
					LIBRTC_LOG(LS_INFO) << "remote_fingerprint:" << remote_fingerprint;
				
					auto pos = remote_fingerprint.find_first_of(" ");
					if (pos == std::string::npos)
					{
						continue;
					}
					std::string   alg = remote_fingerprint.substr(0, pos);
					std::string   sha = remote_fingerprint.substr(pos + 1, remote_fingerprint.size() - pos-2);
					LIBRTC_LOG(LS_INFO) << "alg:" << alg << ", sha: << " << sha ;

					// audio video 
					remote_fingerprint_.algorithm = libssl::DtlsCerts::GetFingerprintAlgorithm(alg);
						remote_fingerprint_.value = sha;
				}
				else if (StringUtils::StartsWith(line, rtpmap_token))
				{
					std::string content = line.substr(rtpmap_token.size());
					auto pos = content.find_first_of(" ");
					if (pos == std::string::npos)
					{
						continue;
					}

					int32_t pt = std::atoi(content.substr(0, pos).c_str());
					auto pos1 = content.find_first_of("/", pos + 1);
					if (pos1 == std::string::npos)
					{
						continue;
					}
					std::string name = content.substr(pos + 1, pos1 - pos - 1);
					if (audio_payload_type_ == -1 && name == "opus")
					{
						audio_payload_type_ = pt;
						LIBRTC_LOG(LS_INFO) << "audio_payload_type:" << audio_payload_type_;
					}
					else if (video_payload_type_ == -1 && name == "H264")
					{
						video_payload_type_ = pt;
						LIBRTC_LOG(LS_INFO) << "video_payload_type:" << video_payload_type_;
					}
					else if (video_payload_rtx_type_ == -1 && name == "rtx/90000")
					{
						//video_payload_rtc_type_ = pt;
						LIBRTC_LOG(LS_INFO) << "pt:" << pt;
					}
					// @date 2025-11-12  识别音频 RTX 提议（rtx/48000），pt 延后在 fmtp apt=audio_pt 再绑定
					else if (audio_payload_rtx_type_ == -1 && name == "rtx/48000")
					{
						LIBRTC_LOG(LS_INFO) << "audio rtx rtpmap candidate pt:" << pt;
					}
				}
				else if (StringUtils::StartsWith(line, fmtp))
				{
					// a=fmtp:122 apt=102
					std::string content = line.substr(ssrc.size());
					auto pos = content.find_first_of(" ");
					if (pos == std::string::npos)
					{
						continue;
					}
					int32_t fmtp = std::atoi(content.substr(0, pos).c_str());

					//ssrc_group_ = content.substr(0, pos);
					//LIBRTC_LOG(LS_INFO) << "fmtp:" << fmtp;
					  pos = content.find_first_of("=");
					int32_t apt = std::atoi(content.substr(pos +1, content.size()).c_str());
					
					if (apt == video_payload_type_)
					{
						LIBRTC_LOG(LS_INFO) << "fmtp:" << fmtp << ", apt : " << apt;
						video_payload_rtx_type_ = fmtp;
					}
					// @date 2025-11-12  音频 RTX apt 绑定
					else if (audio_payload_type_ != -1 && apt == audio_payload_type_)
					{
						LIBRTC_LOG(LS_INFO) << "audio rtx fmtp:" << fmtp << ", apt:" << apt;
						audio_payload_rtx_type_ = fmtp;
					}

				}
				else if (StringUtils::StartsWith(line, ssrc_group))
				{
					//if (rtc_sdp_type_ == kRtcSdpPush)
					{
						//a=ssrc-group:FID 3094518028 3431722997

					}


					std::string content = line.substr(ssrc.size());
					auto pos = content.find_first_of(" ");
					if (pos == std::string::npos)
					{
						continue;
					}

					ssrc_group_ = content.substr(0, pos);
					LIBRTC_LOG(LS_INFO) << "ssrc_group:" << ssrc_group_;


				}
				else if (StringUtils::StartsWith(line, ssrc))
				{
					std::string content = line.substr(ssrc.size());
					auto pos = content.find_first_of(" ");
					if (pos == std::string::npos)
					{
						continue;
					}

					int32_t type_ssrc = std::atol(content.substr(0, pos).c_str());
					 
					if (0 == audio_ssrc_ || type_ssrc == audio_ssrc_)
					{
						// a=ssrc:2334762070 cname:PC7pg9AJa4C7K8dZ
						audio_ssrc_ = type_ssrc;
						LIBRTC_LOG(LS_INFO) << "audio ssrc:" << audio_ssrc_;
					}
					else if (0 == video_ssrc_ || video_ssrc_ == type_ssrc)
					{
					/* 
						a=ssrc:3094518028 cname:PC7pg9AJa4C7K8dZ
						a=ssrc:3094518028 msid:qKdR1YckS50GVYklSxB3XsCGIAw1neUbWeza c997d2f0-0d7a-4572-aaef-facc21099943
						a=ssrc:3431722997 cname:PC7pg9AJa4C7K8dZ
						a=ssrc:3431722997 msid:qKdR1YckS50GVYklSxB3XsCGIAw1neUbWeza c997d2f0-0d7a-4572-aaef-facc21099943
					*/
						video_ssrc_ = type_ssrc;
						LIBRTC_LOG(LS_INFO) << "video ssrc:" << video_ssrc_;
					}
					else if (0 == video_rtx_ssrc_ && video_ssrc_!= type_ssrc)
					{
						video_rtx_ssrc_ = type_ssrc;
						LIBRTC_LOG(LS_INFO) << "video rtx ssrc:" << video_rtx_ssrc_;
					}
					// @date 2025-11-12  第四个不同的 ssrc 视作音频 RTX ssrc（仅当 Offer 声明音频 RTX 时）
					else if (audio_payload_rtx_type_ != -1
						&& 0 == audio_rtx_ssrc_
						&& type_ssrc != audio_ssrc_
						&& type_ssrc != video_ssrc_
						&& type_ssrc != video_rtx_ssrc_)
					{
						audio_rtx_ssrc_ = type_ssrc;
						LIBRTC_LOG(LS_INFO) << "audio rtx ssrc:" << audio_rtx_ssrc_;
					}
					else
					{
						LIBRTC_LOG_T_F(LS_WARNING) << "not  type  ssrc: " << line;
					}
				}
				else if (StringUtils::StartsWith(line, application_token))
				{

					data_channel_params_.application = true;
					LIBRTC_LOG(LS_INFO) << "open  data channel  OK !!! ";
				}
				else if (StringUtils::StartsWith(line, sctp_port_token))
				{
					if (data_channel_params_.application)
					{
						std::string port = line.substr(sctp_port_token.size());
						//auto pos = content.find_first_of(" ");
						data_channel_params_.sctp_port = std::atoi(port.c_str());
						LIBRTC_LOG(LS_INFO) << " data channel sctp port: " << data_channel_params_.sctp_port;

					}
				}
				else if (StringUtils::StartsWith(line, sctp_port_token))
				{
					if (data_channel_params_.application)
					{
						std::string max_mesage_size = line.substr(sctp_port_token.size());
				 
						data_channel_params_.max_message_size = std::atoi(max_mesage_size.c_str());
						LIBRTC_LOG(LS_INFO) << " data channel max message size: " << data_channel_params_.max_message_size;
					}
				}
			}
			return true;
		}
		const std::string &RtcSdp::GetRemoteUFrag() const
		{
			return remote_ufrag_;
		}
		const std::vector<libssl::Fingerprint> &RtcSdp::GetLocalFingerprints()const
		{
			return finger_prints_;
		}
		const  libssl::Fingerprint  &RtcSdp::GetRemoteFingerprint()const
		{
			return remote_fingerprint_;
		}
		const std::string & RtcSdp::GetRemoteRole() const
		{
			// TODO: insert return statement here
			return remote_role_;
		}
		int32_t RtcSdp::GetVideoPayloadType() const
		{
			return video_payload_type_;
		}
		uint32_t RtcSdp::GetVideoPayloadRtxType() const
		{
			return video_payload_rtx_type_;
		}
		// @date 2025-11-12  音频 RTX accessor
		int32_t RtcSdp::GetAudioPayloadRtxType() const
		{
			return audio_payload_rtx_type_;
		}
		int32_t RtcSdp::GetAudioPayloadType() const
		{
			return  audio_payload_type_;
		}

		void RtcSdp::SetLocalFingerprint(const std::vector<libssl::Fingerprint> &fps)
		{
			finger_prints_ = fps;
		}
		void RtcSdp::SetStreamName(const std::string &name)
		{
			//stream_name_ = name;
			// 
			std::vector<std::string>  list;
			rtc::split(name, '/', &list);
			if (list.size() == 3)
			{
				stream_name_ = list[2];
			}
			else
			{
				stream_name_ = name;
			}
		}
		void RtcSdp::SetLocalUFrag(const std::string &frag)
		{
			local_ufrag_ = frag;
		}
		void RtcSdp::SetLocalPasswd(const std::string &pwd)
		{
			local_passwd_ = pwd;
		}
		void RtcSdp::SetServerPort(uint16_t port)
		{
			server_port_ = port;
		}
		void RtcSdp::SetServerAddr(const std::string &addr)
		{
			server_addr_ = addr;
		}

		void RtcSdp::SetServerExternPort(uint16_t port)
		{
			server_extern_port_ = port;
		}
		void RtcSdp::SetServerExternAddr(const std::string& addr)
		{
			server_extern_addr_ = addr;
		}
		void RtcSdp::SetVideoSsrc(uint32_t ssrc)
		{
			video_ssrc_ = ssrc;
		}
		void RtcSdp::SetVideoRtxSsrc(uint32_t ssrc)
		{
			video_rtx_ssrc_ = ssrc;
		}
		// @date 2025-11-12  音频 RTX ssrc setter
		void RtcSdp::SetAudioRtxSsrc(uint32_t ssrc)
		{
			audio_rtx_ssrc_ = ssrc;
		}
		void RtcSdp::SetAudioSsrc(int32_t ssrc)
		{
			audio_ssrc_ = ssrc;
		}
		const std::string &RtcSdp::GetLocalPasswd()const
		{
			return local_passwd_;
		}
		const std::string &RtcSdp::GetLocalUFrag()const
		{
			return local_ufrag_;
		}
		uint32_t RtcSdp::VideoSsrc() const
		{
			return video_ssrc_;
		}
		uint32_t RtcSdp::VideoRtxSsrc() const
		{
			return video_rtx_ssrc_;
		}
		// @date 2025-11-12  音频 RTX ssrc getter
		uint32_t RtcSdp::AudioRtxSsrc() const
		{
			return audio_rtx_ssrc_;
		}
		uint32_t RtcSdp::AudioSsrc() const
		{
			return audio_ssrc_;
		}
		void RtcSdp::SetDataChannelParams(const DataChannelParams& params)
		{
			data_channel_params_ = params;
		}
		const DataChannelParams& RtcSdp::GetDataChannelParams() const
		{
			return data_channel_params_;
		}
		std::string RtcSdp::Encode()
		{
			std::ostringstream ss;
			int32_t mid = 0;
			ss << "v=0\n";
			ss << "o=rtc 11111111111360111 2 IN IP4 0.0.0.0\n";
			ss << "s=" << stream_name_ << "\n";
			ss << "c=IN IP4 0.0.0.0\n";
			ss << "t=0 0\n";




			ss << "a=group:BUNDLE";
			if (0 != audio_payload_type_)
			{
				ss << " " << mid++ ;
			}
			if (0 != video_payload_type_)
			{
				ss << " " << mid++  ;
			}
			if (data_channel_params_.application)
			{
				ss << " " << mid++;
			}
			ss << "\n";
			ss << "a=msid-semantic: WMS " << stream_name_ << "\n";

			std::stringstream finger_prints;
			for (auto finger_print : finger_prints_)
			{
				finger_prints << "a=fingerprint:" << libssl::DtlsCerts::GetFingerprintAlgorithmString(finger_print.algorithm)
					<< " " << finger_print.value << "\n";
			}
			
			std::stringstream candidate_prints;
			candidate_prints << "a=candidate:0 1 udp 2130706431 " << server_addr_ << " " << server_port_ << " typ host generation 0\n";
			// a=candidate:4234997325 1 udp 2043278322 192.0.2.172 44323 typ host
			// 通过 STUN 服务器获取的、经 NAT 映射后的公网地址。它必须包含 raddr 和 rport 字段，用于指明映射前的本地基础地址
			// a=candidate:842163049 1 udp 1677729535 203.0.113.5 62005 typ srflx raddr 192.0.2.172 rport 44323
			if (server_extern_addr_.size() > 1)
			{
				candidate_prints << "a=candidate:842163049 1 udp 1677729535 " << server_extern_addr_ << " " << server_extern_port_ << " typ srflx raddr "<< server_addr_ << " rport " << server_port_ << "\n";
			}
			
			// 由对等端在连通性检查过程中发现的地址，它的格式与 srflx 类似，也包含基础地址映射信息
			// a=candidate:2156732508 1 udp 1686052607 198.51.100.8 51987 typ prflx raddr 192.0.2.172 rport 44323
			// 通过 TURN 服务器中继流量的地址。它必须包含 raddr 和 rport 字段，且 raddr 通常是 TURN 服务器的公网 IP
			// a=candidate:3156894721 1 udp 41885439 192.0.2.88 50011 typ relay raddr 203.0.113.1 rport 3478
			if (video_payload_type_ != -1 && audio_payload_type_ != -1)
			{
				// @date 2025-11-12  如协商音频 RTX，m= 行追加 rtx_pt
				if (audio_payload_rtx_type_ > 0)
				{
					ss << "m=audio 9 UDP/TLS/RTP/SAVPF " << audio_payload_type_
						<< " " << audio_payload_rtx_type_ << "\n";
				}
				else
				{
					ss << "m=audio 9 UDP/TLS/RTP/SAVPF " << audio_payload_type_ << "\n";
				}
				ss << "c=IN IP4 0.0.0.0\n";
				
				ss << "a=mid:0\n";
				ss << "a=ice-ufrag:" << local_ufrag_ << "\n";
				ss << "a=ice-pwd:" << local_passwd_ << "\n";

				//ss << "a=candidate:0 1 udp 2130706431 " << server_addr_ << " " << server_port_ << " typ host generation 0\n";
				//ss << "a=fingerprint:sha-256 " << fingerprint_ << "\n";
				ss << candidate_prints.str();
				ss << finger_prints.str();
				ss << "a=setup:passive\n"; 
				
				if (rtc_sdp_type_ == kRtcSdpPlay)
				{
					ss << "a=sendonly\n";
				}
				else if (rtc_sdp_type_ == kRtcSdpPush)
				{
					ss << "a=sendrecv\n";
				}
				else
				{
					LIBRTC_LOG_T_F(LS_WARNING) << "rtc_sdp_type:" << rtc_sdp_type_;
				}
				ss << "a=rtcp-mux\n";
				ss << "a=rtcp-rsize\n";
				ss << "a=rtpmap:" << audio_payload_type_ << " opus/48000/2\n";
				ss << "a=fmtp:" << audio_payload_type_ << " minptime=10;stereo=1;useinbandfec=1\n";
				ss << "a=rtcp-fb:" << audio_payload_type_ << " transport-cc\n";
				ss << "a=rtcp-fb:" << audio_payload_type_ << " nack\n";

				// @date 2025-11-12  音频 RTX 声明（rtx/48000 + apt=audio_pt），仅在 Offer 协商到时启用
				if (audio_payload_rtx_type_ > 0)
				{
					ss << "a=rtpmap:" << audio_payload_rtx_type_ << " rtx/48000\n";
					ss << "a=fmtp:" << audio_payload_rtx_type_ << " apt=" << audio_payload_type_ << "\n";
				}

				// twcc 
				/*
				* audio : 
					a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level
					a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
					a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
					a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid
				*/
				//ss << "a=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\n";
				ss << "a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\n";
				ss << "a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\n";

				if (rtc_sdp_type_ == kRtcSdpPlay)
				{
					// @date 2025-11-12  音频 RTX：启用时追加 ssrc-group FID 与 rtx ssrc 描述
					if (audio_payload_rtx_type_ > 0 && audio_rtx_ssrc_ != 0)
					{
						ss << "a=ssrc-group:FID " << audio_ssrc_ << " " << audio_rtx_ssrc_ << "\n";
					}

					ss << "a=ssrc:" << audio_ssrc_ << " cname:" << stream_name_ << "\n";
					ss << "a=ssrc:" << audio_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_audio\n";
					ss << "a=ssrc:" << audio_ssrc_ << " mslabel:" << stream_name_ << "\n";
					ss << "a=ssrc:" << audio_ssrc_ << " label:" << stream_name_ << "_audio\n";

					if (audio_payload_rtx_type_ > 0 && audio_rtx_ssrc_ != 0)
					{
						ss << "a=ssrc:" << audio_rtx_ssrc_ << " cname:" << stream_name_ << "\n";
						ss << "a=ssrc:" << audio_rtx_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_audio\n";
					}
				}
			}
			if (video_payload_type_ != -1)
			{
				//std::vector<int32_t>
#if 1
				ss << "m=video 9 UDP/TLS/RTP/SAVPF " << video_payload_type_ << " " << video_payload_rtx_type_ << "\n";
#else
				ss << "m=video 9 UDP/TLS/RTP/SAVPF 96 97 98 99 100 101 35 36 37 38 103 104 107 108 109 114 115 116 117 118 39 40 41 42 43 44 45 46 47 48 119 120 121 122 49 50 51 52 123 124 125 53\n";

#endif //
				ss << "c=IN IP4 0.0.0.0\n";
				if (mid > 1)
				{
					ss << "a=mid:1\n";
				}
				else
				{
					ss << "a=mid:0\n";
				}
				
				ss << "a=ice-ufrag:" << local_ufrag_ << "\n";
				ss << "a=ice-pwd:" << local_passwd_ << "\n";
				//ss << "a=fingerprint:sha-256 " << fingerprint_ << "\n";
				ss << finger_prints.str();
				ss << "a=setup:passive\n";
				//ss << "a=candidate:0 1 udp 2130706431 " << server_addr_ << " " << server_port_ << " typ host generation 0\n";
				ss << candidate_prints.str();
				// a=candidate:4234997325 1 udp 2043278322 192.0.2.172 44323 typ host
				// 通过 STUN 服务器获取的、经 NAT 映射后的公网地址。它必须包含 raddr 和 rport 字段，用于指明映射前的本地基础地址
				// a=candidate:842163049 1 udp 1677729535 203.0.113.5 62005 typ srflx raddr 192.0.2.172 rport 44323
				// 由对等端在连通性检查过程中发现的地址，它的格式与 srflx 类似，也包含基础地址映射信息
				// a=candidate:2156732508 1 udp 1686052607 198.51.100.8 51987 typ prflx raddr 192.0.2.172 rport 44323
				// 通过 TURN 服务器中继流量的地址。它必须包含 raddr 和 rport 字段，且 raddr 通常是 TURN 服务器的公网 IP
				// a=candidate:3156894721 1 udp 41885439 192.0.2.88 50011 typ relay raddr 203.0.113.1 rport 3478
				if (rtc_sdp_type_ == kRtcSdpPlay)
				{
					ss << "a=sendonly\n";
				}
				else if (rtc_sdp_type_ == kRtcSdpPush)
				{
					ss << "a=sendrecv\n";
				}
				else
				{
					LIBRTC_LOG_T_F(LS_WARNING) << "rtc_sdp_type:" << rtc_sdp_type_;
				}
				ss << "a=rtcp-mux\n";
				ss << "a=rtcp-rsize\n";
#if 1
				ss << "a=rtpmap:" << video_payload_type_ << " H264/90000\n";
				/*
				 a=fmtp:123 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=64001f​

				level-asymmetry-allowed=1指明通信双方使用的H264Level是否要保持一致，0必须一致，1可以不一致。

				packetization-mode指明经H264编码后的视频数据如何打包：0单包、1非交错包、2交错包。三种打包模式中，模式0和模式1用于低延迟的实时通信领域。

				模式0的含义是每个包就是一帧视频数据。

				模式1是可以将视频帧拆分成多个顺序的RTP包发送，接收端收到数据包后再按顺序将其还原。

				profile-level-id由三部分组成，即profile_idc、profile_iop以及level_idc，每个组成占8位，
				因此可以推测出profile_idc=64、profile_iop=00、level-idc=1f

				 a=rtpmap:114 red/90000，red是一种在webrtc中使用的FEC（引入前向纠错）算法，用于防止丢包；
				 a=fmtp:103 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f
				*/
				// a=fmtp:41 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=f4001f
				ss << "a=fmtp:"<< video_payload_type_ <<" level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f\n";

				ss << "a=rtpmap:" << video_payload_rtx_type_ << " rtx/90000\n";
				ss << "a=fmtp:" << video_payload_rtx_type_ << " apt=" << video_payload_type_ << "\n";
				//ss << "a=fmtp:" << video_payload_type_ << " x-google-min-bitrate=8000; x-google-max-bitrate=10000" << "\n";
				ss << "a=fmtp:" << video_payload_type_ << " x-google-min-bitrate=6000; x-google-start-bitrate=4000; x-google-max-bitrate=10000\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " ccm fir\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " goog-remb\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " nack\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " nack pli\n";
				ss << "a=rtcp-fb:" << video_payload_type_ << " transport-cc\n";
				/*
				  video : 
					a=extmap:14 urn:ietf:params:rtp-hdrext:toffset
					a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time
					a=extmap:13 urn:3gpp:video-orientation
					a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01
					a=extmap:5 http://www.webrtc.org/experiments/rtp-hdrext/playout-delay
					a=extmap:6 http://www.webrtc.org/experiments/rtp-hdrext/video-content-type
					a=extmap:7 http://www.webrtc.org/experiments/rtp-hdrext/video-timing
					a=extmap:8 http://www.webrtc.org/experiments/rtp-hdrext/color-space
					a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid
					a=extmap:10 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
					a=extmap:11 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id
				
				*/

				ss << "a=extmap:2 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\n";
				ss << "a=extmap:3 http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\n";
#else 
				ss << "a=rtpmap:96 VP8/90000\n";
				ss <<"a=rtcp-fb:96 goog-remb\n";
				ss <<"a=rtcp-fb:96 transport-cc\n";
				ss <<"a=rtcp-fb:96 ccm fir\n";
				ss <<"a=rtcp-fb:96 nack\n";
				ss <<"a=rtcp-fb:96 nack pli\n";
				ss <<"a=rtpmap:97 rtx/90000\n";
				ss <<"a=fmtp:97 apt=96\n";
				ss <<"a=rtpmap:98 VP9/90000\n";
				ss <<"a=rtcp-fb:98 goog-remb\n";
				ss <<"a=rtcp-fb:98 transport-cc\n";
				ss <<"a=rtcp-fb:98 ccm fir\n";
				ss <<"a=rtcp-fb:98 nack\n";
				ss <<"a=rtcp-fb:98 nack pli\n";
				ss <<"a=fmtp:98 profile-id=0\n";
				ss <<"a=rtpmap:99 rtx/90000\n";
				ss <<"a=fmtp:99 apt=98\n";
				ss <<"a=rtpmap:100 VP9/90000\n";
				ss <<"a=rtcp-fb:100 goog-remb\n";
				ss <<"a=rtcp-fb:100 transport-cc\n";
				ss <<"a=rtcp-fb:100 ccm fir\n";
				ss <<"a=rtcp-fb:100 nack\n";
				ss <<"a=rtcp-fb:100 nack pli\n";
				ss <<"a=fmtp:100 profile-id=2\n";
				ss <<"a=rtpmap:101 rtx/90000\n";
				ss <<"a=fmtp:101 apt=100\n";
				ss <<"a=rtpmap:35 VP9/90000\n";
				ss <<"a=rtcp-fb:35 goog-remb\n";
				ss <<"a=rtcp-fb:35 transport-cc\n";
				ss <<"a=rtcp-fb:35 ccm fir\n";
				ss <<"a=rtcp-fb:35 nack\n";
				ss <<"a=rtcp-fb:35 nack pli\n";
				ss <<"a=fmtp:35 profile-id=1\n";
				ss <<"a=rtpmap:36 rtx/90000\n";
				ss <<"a=fmtp:36 apt=35\n";
				ss <<"a=rtpmap:37 VP9/90000\n";
				ss <<"a=rtcp-fb:37 goog-remb\n";
				ss <<"a=rtcp-fb:37 transport-cc\n";
				ss <<"a=rtcp-fb:37 ccm fir\n";
				ss <<"a=rtcp-fb:37 nack\n";
				ss <<"a=rtcp-fb:37 nack pli\n";
				ss <<"a=fmtp:37 profile-id=3\n";
				ss <<"a=rtpmap:38 rtx/90000\n";
				ss <<"a=fmtp:38 apt=37\n";













				ss << "a=rtpmap:103 H264/90000\n";
				ss << "a=rtcp-fb:103 goog-remb\n";
				ss << "a=rtcp-fb:103 transport-cc\n";
				ss << "a=rtcp-fb:103 ccm fir\n";
				ss << "a=rtcp-fb:103 nack\n";
				ss << "a=rtcp-fb:103 nack pli\n";
				ss << "a=fmtp:103 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f\n";
				ss << "a=rtpmap:104 rtx/90000\n";
				ss << "a=fmtp:104 apt=103\n";
				ss << "a=rtpmap:107 H264/90000\n";
				ss << "a=rtcp-fb:107 goog-remb\n";
				ss << "a=rtcp-fb:107 transport-cc\n";
				ss << "a=rtcp-fb:107 ccm fir\n";
				ss << "a=rtcp-fb:107 nack\n";
				ss << "a=rtcp-fb:107 nack pli\n";
				ss << "a=fmtp:107 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42001f\n";
				ss << "a=rtpmap:108 rtx/90000\n";
				ss << "a=fmtp:108 apt=107\n";
				ss << "a=rtpmap:109 H264/90000\n";
				ss << "a=rtcp-fb:109 goog-remb\n";
				ss << "a=rtcp-fb:109 transport-cc\n";
				ss << "a=rtcp-fb:109 ccm fir\n";
				ss << "a=rtcp-fb:109 nack\n";
				ss << "a=rtcp-fb:109 nack pli\n";
				ss << "a=fmtp:109 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42e01f\n";
				ss << "a=rtpmap:114 rtx/90000\n";
				ss << "a=fmtp:114 apt=109\n";
				ss << "a=rtpmap:115 H264/90000\n";
				ss << "a=rtcp-fb:115 goog-remb\n";
				ss << "a=rtcp-fb:115 transport-cc\n";
				ss << "a=rtcp-fb:115 ccm fir\n";
				ss << "a=rtcp-fb:115 nack\n";
				ss << "a=rtcp-fb:115 nack pli\n";
				ss << "a=fmtp:115 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=42e01f\n";
				ss << "a=rtpmap:116 rtx/90000\n";
				ss << "a=fmtp:116 apt=115\n";
				ss << "a=rtpmap:117 H264/90000\n";
				ss << "a=rtcp-fb:117 goog-remb\n";
				ss << "a=rtcp-fb:117 transport-cc\n";
				ss << "a=rtcp-fb:117 ccm fir\n";
				ss << "a=rtcp-fb:117 nack\n";
				ss << "a=rtcp-fb:117 nack pli\n";
				ss << "a=fmtp:117 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=4d001f\n";
				ss << "a=rtpmap:118 rtx/90000\n";
				ss << "a=fmtp:118 apt=117\n";
				ss << "a=rtpmap:39 H264/90000\n";
				ss << "a=rtcp-fb:39 goog-remb\n";
				ss << "a=rtcp-fb:39 transport-cc\n";
				ss << "a=rtcp-fb:39 ccm fir\n";
				ss << "a=rtcp-fb:39 nack\n";
				ss << "a=rtcp-fb:39 nack pli\n";
				ss << "a=fmtp:39 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=4d001f\n";
				ss << "a=rtpmap:40 rtx/90000\n";
				ss << "a=fmtp:40 apt=39\n";
				ss << "a=rtpmap:41 H264/90000\n";
				ss << "a=rtcp-fb:41 goog-remb\n";
				ss << "a=rtcp-fb:41 transport-cc\n";
				ss << "a=rtcp-fb:41 ccm fir\n";
				ss << "a=rtcp-fb:41 nack\n";
				ss << "a=rtcp-fb:41 nack pli\n";
				ss << "a=fmtp:41 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=f4001f\n";
				ss << "a=rtpmap:42 rtx/90000\n";
				ss << "a=fmtp:42 apt=41\n";
				ss << "a=rtpmap:43 H264/90000\n";
				ss << "a=rtcp-fb:43 goog-remb\n";
				ss << "a=rtcp-fb:43 transport-cc\n";
				ss << "a=rtcp-fb:43 ccm fir\n";
				ss << "a=rtcp-fb:43 nack\n";
				ss << "a=rtcp-fb:43 nack pli\n";
				ss << "a=fmtp:43 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=f4001f\n";
				ss << "a=rtpmap:44 rtx/90000\n";
				ss << "a=fmtp:44 apt=43\n";
				ss << "a=rtpmap:45 AV1/90000\n";
				ss << "a=rtcp-fb:45 goog-remb\n";
				ss << "a=rtcp-fb:45 transport-cc\n";
				ss << "a=rtcp-fb:45 ccm fir\n";
				ss << "a=rtcp-fb:45 nack\n";
				ss << "a=rtcp-fb:45 nack pli\n";
				ss << "a=fmtp:45 level-idx=5;profile=0;tier=0\n";
				ss << "a=rtpmap:46 rtx/90000\n";
				ss << "a=fmtp:46 apt=45\n";
				ss << "a=rtpmap:47 AV1/90000\n";
				ss << "a=rtcp-fb:47 goog-remb\n";
				ss << "a=rtcp-fb:47 transport-cc\n";
				ss << "a=rtcp-fb:47 ccm fir\n";
				ss << "a=rtcp-fb:47 nack\n";
				ss << "a=rtcp-fb:47 nack pli\n";
				ss << "a=fmtp:47 level-idx=5;profile=1;tier=0\n";
				ss << "a=rtpmap:48 rtx/90000\n";
				ss << "a=fmtp:48 apt=47\n";
				ss << "a=rtpmap:119 H264/90000\n";
				ss << "a=rtcp-fb:119 goog-remb\n";
				ss << "a=rtcp-fb:119 transport-cc\n";
				ss << "a=rtcp-fb:119 ccm fir\n";
				ss << "a=rtcp-fb:119 nack\n";
				ss << "a=rtcp-fb:119 nack pli\n";
				ss << "a=fmtp:119 level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=64001f\n";
				ss << "a=rtpmap:120 rtx/90000\n";
				ss << "a=fmtp:120 apt=119\n";
				ss << "a=rtpmap:121 H264/90000\n";
				ss << "a=rtcp-fb:121 goog-remb\n";
				ss << "a=rtcp-fb:121 transport-cc\n";
				ss << "a=rtcp-fb:121 ccm fir\n";
				ss << "a=rtcp-fb:121 nack\n";
				ss << "a=rtcp-fb:121 nack pli\n";
				ss << "a=fmtp:121 level-asymmetry-allowed=1;packetization-mode=0;profile-level-id=64001f\n";
				ss << "a=rtpmap:122 rtx/90000\n";
				ss << "a=fmtp:122 apt=121\n";
				ss << "a=rtpmap:49 H265/90000\n";
				ss << "a=rtcp-fb:49 goog-remb\n";
				ss << "a=rtcp-fb:49 transport-cc\n";
				ss << "a=rtcp-fb:49 ccm fir\n";
				ss << "a=rtcp-fb:49 nack\n";
				ss << "a=rtcp-fb:49 nack pli\n";
				ss << "a=fmtp:49 level-id=180;profile-id=1;tier-flag=0;tx-mode=SRST\n";
				ss << "a=rtpmap:50 rtx/90000\n";
				ss << "a=fmtp:50 apt=49\n";
				ss << "a=rtpmap:51 H265/90000\n";
				ss << "a=rtcp-fb:51 goog-remb\n";
				ss << "a=rtcp-fb:51 transport-cc\n";
				ss << "a=rtcp-fb:51 ccm fir\n";
				ss << "a=rtcp-fb:51 nack\n";
				ss << "a=rtcp-fb:51 nack pli\n";
				ss << "a=fmtp:51 level-id=180;profile-id=2;tier-flag=0;tx-mode=SRST\n";
				ss << "a=rtpmap:52 rtx/90000\n";
				ss << "a=fmtp:52 apt=51\n";
				ss << "a=rtpmap:123 red/90000\n";
				ss << "a=rtpmap:124 rtx/90000\n";
				ss << "a=fmtp:124 apt=123\n";
				ss << "a=rtpmap:125 ulpfec/90000\n";
				ss << "a=rtpmap:53 flexfec-03/90000\n";
				ss << "a=rtcp-fb:53 goog-remb\n";
				ss << "a=rtcp-fb:53 transport-cc\n";
				ss << "a=fmtp:53 repair-window=10000000\n";

























#endif 

				if (rtc_sdp_type_ == kRtcSdpPlay)
				{
					ss << "a=ssrc:" << video_ssrc_ << " cname:" << stream_name_ << "\n";
					ss << "a=ssrc:" << video_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_video\n";
					ss << "a=ssrc:" << video_rtx_ssrc_ << " cname:" << stream_name_ << "\n";
					ss << "a=ssrc:" << video_rtx_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_video\n";

				}
			}


			if (data_channel_params_.application)
			{
				ss << "m=application 9 UDP/DTLS/SCTP webrtc-datachannel\n";
				ss << "c=IN IP4 0.0.0.0\n";
				//ss << "a=ice-ufrag:QMlp\n";
				//ss << "a=ice-pwd:flokPS0swUVGfjizysa3zuL4\n";
				ss << "a=ice-ufrag:" << local_ufrag_ << "\n";
				ss << "a=ice-pwd:" << local_passwd_ << "\n";
				ss << "a=ice-options:trickle\n";
				//ss << "a=fingerprint:sha-256 11:3D:8D:D7:E7:86:7E:4B:9D:0C:75:AF:60:CF:7D:88:AB:F2:5D:7E:15:A3:E5:A3:5E:C0:C4:B8:62:1F:44:EC\n";
				//ss << "a=setup:actpass\n";
				ss << finger_prints.str();
				ss << "a=setup:passive\n";
				 
				ss << "a=mid:"<< (mid-1) <<"\n";
				ss << "a=sctp-port:5000\n";
				ss << "a=max-message-size:262144\n";
			}


			return ss.str();
		}
	}
	
 
}