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


		1. �Ự�������Ự�汾���Ự���ƣ��Ựʱ�����Ự������
		2. ý����Ϣ����������Ƶ��ʽ�б���֧�ֵ�Э���б�������˵����
		3. ���������������������ͣ���ַ���ͣ�IP��ַ
		4. ��ȫ������ice�û��������룬fingerprint
		5. ��������������RTCP��������


 ******************************************************************************/


#ifndef _C_RTC_SDP_H_
#define _C_RTC_SDP_H_

#include <cstddef>

#include "absl/types/optional.h"
#include "libmedia_transfer_protocol/librtc/dtls_certs.h"

namespace libmedia_transfer_protocol {
	namespace librtc {

		/**
		*  @brief SDP类型枚举（SDP Type Enum）
		*  
		*  该枚举定义了SDP的使用类型。在WebRTC中，SDP可以用于推流或拉流。
		*  
		*  类型说明：
		*  - kRtcSdpNone: 未指定类型
		*  - kRtcSdpPlay: 拉流（播放）模式，客户端接收媒体流
		*  - kRtcSdpPush: 推流模式，客户端发送媒体流
		*  
		*  @note 类型决定了SDP Answer中的媒体方向属性
		*  @note Play模式：a=sendonly（服务器发送，客户端接收）
		*  @note Push模式：a=recvonly（服务器接收，客户端发送）
		*/
		enum RtcSdpType
		{
			kRtcSdpNone = 0,    ///< 未指定类型
			kRtcSdpPlay,        ///< 拉流（播放）模式
			kRtcSdpPush         ///< 推流模式
			
		};
		 
		/**
		*  @brief 数据通道参数结构体（Data Channel Parameters）
		*  
		*  该结构体定义了WebRTC数据通道（Data Channel）的参数。
		*  数据通道使用SCTP协议在DTLS连接上传输非媒体数据。
		*  
		*  参数说明：
		*  - application: 是否启用数据通道
		*  - sctp_port: SCTP端口号，默认5000
		*  - max_message_size: 最大消息大小，默认256KB
		*  
		*  @note 数据通道在SDP中表示为 m=application 行
		*  @note SCTP端口用于区分不同的数据流
		*/
		struct DataChannelParams
		{
			bool    application = false;          ///< 是否启用数据通道
			int32_t sctp_port = 5000;             ///< SCTP端口号
			int32_t max_message_size = 262144;    ///< 最大消息大小（256KB）
		};
		/**
		*  @author chensong
		*  @date 2025-10-14
		*  @brief SDP协议处理类（SDP Protocol Handler）
		*  
		*  RtcSdp类用于处理WebRTC的SDP协议，包括SDP的解析（Decode）和生成（Encode）。
		*  它支持音频、视频和数据通道的SDP描述。
		*  
		*  主要功能：
		*  1. SDP解析：解析SDP Offer，提取媒体信息、网络信息、安全属性等
		*  2. SDP生成：生成SDP Answer，包含服务器选择的媒体格式和参数
		*  3. 参数管理：管理ICE参数、DTLS指纹、SSRC等
		*  4. 数据通道支持：支持SCTP数据通道的SDP描述
		*  
		*  工作流程：
		*  1. 接收客户端的SDP Offer
		*  2. 调用Decode()解析Offer
		*  3. 设置本地参数（ufrag、pwd、fingerprint、ssrc等）
		*  4. 调用Encode()生成Answer
		*  5. 将Answer发送给客户端
		*  
		*  @note 该类负责SDP的完整处理，不涉及媒体传输
		*  @note SDP格式遵循RFC 4566规范
		*  @note 支持WebRTC标准的SDP扩展
		*  
		*  使用示例：
		*  @code
		*  RtcSdp sdp;
		*  
		*  // 解析Offer
		*  std::string offer = "v=0\r\no=- ...";
		*  sdp.SetSdpType(kRtcSdpPlay);
		*  if (sdp.Decode(offer)) {
		*      // 设置本地参数
		*      sdp.SetLocalUFrag("abcd");
		*      sdp.SetLocalPasswd("1234567890abcdef");
		*      sdp.SetLocalFingerprint(fingerprints);
		*      sdp.SetVideoSsrc(12345678);
		*      
		*      // 生成Answer
		*      std::string answer = sdp.Encode();
		*  }
		*  @endcode
		*/
		class RtcSdp
		{
		public:
			/**
			*  @brief 构造函数（Constructor）
			*  
			*  该构造函数用于创建RtcSdp实例。它会初始化所有成员变量为默认值。
			*  
			*  @note 构造后需要调用SetSdpType()设置SDP类型
			*/
			RtcSdp();
			
			/**
			*  @brief 析构函数（Destructor）
			*  
			*  该析构函数用于清理RtcSdp实例。
			*/
			virtual ~RtcSdp();

		public:
			/** 设置SDP类型 */
			void SetSdpType(RtcSdpType  rtc_sdp_type);
			
			/** 解析SDP Offer */
			bool Decode(const std::string &sdp);
			
			/** 获取远程ICE用户名片段 */
			const std::string &GetRemoteUFrag() const;
			
			/** 获取本地证书指纹列表 */
			const std::vector<libssl::Fingerprint> &GetLocalFingerprints()const;
			
			/** 获取远程证书指纹 */
			const libssl::Fingerprint  &  GetRemoteFingerprint() const;
			
			/** 获取远程DTLS角色 */
			const std::string  &GetRemoteRole() const;
			
			/** 获取视频负载类型 */
			int32_t GetVideoPayloadType() const;
			

			uint32_t GetVideoPayloadRtxType() const;


			/** 获取音频负载类型 */
			int32_t GetAudioPayloadType() const;

			/** 设置本地证书指纹 */
			void SetLocalFingerprint(const std::vector<libssl::Fingerprint> &fps);
			
			/** 设置流名称 */
			void SetStreamName(const std::string &name);
			
			/** 设置本地ICE用户名片段 */
			void SetLocalUFrag(const std::string &frag);
			
			/** 设置本地ICE密码 */
			void SetLocalPasswd(const std::string &pwd);
			
			/** 设置服务器端口 */
			void SetServerPort(uint16_t port);
			
			/** 设置服务器地址 */
			void SetServerAddr(const std::string &addr);
			
			/** 设置视频SSRC */
			void SetVideoSsrc(uint32_t ssrc);
			
			/** 设置视频RTX SSRC */
			void SetVideoRtxSsrc(uint32_t ssrc);
			
			/** 设置音频SSRC */
			void SetAudioSsrc(int32_t ssrc);
			
			/** 设置数据通道参数 */
			void SetDataChannelParams(const DataChannelParams& params);
			
			/** 获取本地ICE密码 */
			const std::string &GetLocalPasswd()const;
			
			/** 获取本地ICE用户名片段 */
			const std::string &GetLocalUFrag()const;
			
			/** 获取视频SSRC */
			uint32_t VideoSsrc() const;
			
			/** 获取视频RTX SSRC */
			uint32_t VideoRtxSsrc() const;

			/** 获取音频SSRC */
			uint32_t AudioSsrc() const;
			
			/** 获取数据通道参数 */
			const DataChannelParams& GetDataChannelParams() const;
			
			/** 编码生成SDP Answer */
			std::string Encode();
		private:
			int32_t audio_payload_type_{ -1 };
			int32_t video_payload_type_{ -1 };
			int32_t video_payload_rtx_type_{128};
			// Զ�˵��û���������
			std::string remote_ufrag_;
			/*  a = setup ��Ҫ�Ǳ�ʾdtls��Э�̹����н�ɫ�����⣬˭�ǿͻ��ˣ�˭�Ƿ�����
				a = setup:actpass �ȿ����ǿͻ��ˣ�Ҳ�����Ƿ�����
				a = setup : active �ͻ���
				a = setup : passive ������
				�ɿͻ����ȷ���client hello*/
			std::string remote_role_;// role = "active" / "passive" / "actpass" / "holdconn"
			std::string remote_passwd_;
			std::string local_ufrag_;
			std::string local_passwd_;
			//std::string remote_fingerprint_;
			libssl::Fingerprint  remote_fingerprint_;
			std::vector<libssl::Fingerprint>   finger_prints_;
			int32_t video_ssrc_{ 0 };
			int32_t video_rtx_ssrc_{ 0 };
			int32_t audio_ssrc_{ 0 };
			int16_t server_port_{ 0 };
			std::string server_addr_;
			std::string stream_name_;


			RtcSdpType      rtc_sdp_type_{ kRtcSdpPlay };
			std::string     ssrc_group_{ "FID" }; // video rtx 

			DataChannelParams   data_channel_params_;
		};
	}
	

}


#endif // 