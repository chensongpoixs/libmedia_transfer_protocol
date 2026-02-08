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

purpose:		FLV Encoder - FLV格式编码器实现
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
#include "libmedia_transfer_protocol/libflv/cflv_encoder.h"
#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <cstdalign>
#include "rtc_base/logging.h"
#include "libmedia_transfer_protocol/libflv/amf0.h"
#include "common_video/h264/h264_common.h"
#include "modules/video_coding/include/video_coding.h"
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
#include "libmedia_transfer_protocol/libmedia_transfer_protocol_log.h"
namespace libmedia_transfer_protocol
{
	namespace libflv
	{
		namespace {
			/**
			*  @brief 写入24位大端序整数（Write 24-bit Big-Endian Integer）
			*  
			*  将32位整数的低24位以大端序（Big-Endian）格式写入内存。
			*  FLV格式中的DataSize和Timestamp字段使用24位大端序。
			*  
			*  @param p 目标内存地址
			*  @param val 要写入的值（只使用低24位）
			*  
			*  示例：
			*  val = 0x123456 -> 内存布局: [0x12, 0x34, 0x56]
			*/
			static void set_be24(void *p, uint32_t val)
			{
				uint8_t *data = (uint8_t *)p;
				data[0] = val >> 16;  // 高8位
				data[1] = val >> 8;   // 中8位
				data[2] = val;        // 低8位
			}
			
			/**
			*  @brief 写入32位大端序整数（Write 32-bit Big-Endian Integer）
			*  
			*  将32位整数以大端序（Big-Endian）格式写入内存。
			*  FLV格式中的PreviousTagSize和NALU长度字段使用32位大端序。
			*  
			*  @param p 目标内存地址
			*  @param val 要写入的值
			*  
			*  示例：
			*  val = 0x12345678 -> 内存布局: [0x12, 0x34, 0x56, 0x78]
			*/
			static void set_be32(void *p, uint32_t val)
			{
				uint8_t *data = (uint8_t *)p;
				data[0] = val >> 24;  // 最高8位
				data[1] = val >> 16;  // 次高8位
				data[2] = val >> 8;   // 次低8位
				data[3] = val;        // 最低8位
			}
			
			/**
			*  @brief FLV编码器名称常量
			*  
			*  用于onMetaData中的encoder字段，标识编码器来源。
			*/
			static const char   kflv_muxer[] = "libflv_rtc";


			/**
			*  @brief FLV视频帧类型位偏移量
			*  
			*  FLV Video Tag的第一个字节高4位表示帧类型，需要左移4位。
			*/
#define FLV_VIDEO_FRAMETYPE_OFFSET   4

			/**
			*  @brief FLV视频帧类型枚举（FLV Video Frame Type）
			*  
			*  定义FLV视频帧的类型标识，用于Video Tag的第一个字节高4位。
			*/
			enum {
				FLV_FRAME_KEY = 1 << FLV_VIDEO_FRAMETYPE_OFFSET,           ///< 关键帧（可搜索帧，IDR帧）
				FLV_FRAME_INTER = 2 << FLV_VIDEO_FRAMETYPE_OFFSET,         ///< 非关键帧（不可搜索帧，P帧/B帧）
				FLV_FRAME_DISP_INTER = 3 << FLV_VIDEO_FRAMETYPE_OFFSET,    ///< 可丢弃的非关键帧（仅H.263）
				FLV_FRAME_GENERATED_KEY = 4 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< 生成的关键帧（服务器保留）
				FLV_FRAME_VIDEO_INFO_CMD = 5 << FLV_VIDEO_FRAMETYPE_OFFSET,///< 视频信息/命令帧
			};
			
			/**
			*  @brief FLV视频编码格式ID枚举（FLV Video Codec ID）
			*  
			*  定义FLV支持的视频编码格式标识，用于Video Tag的第一个字节低4位。
			*/
			enum {
				FLV_CODECID_H263 = 2,      ///< H.263编码
				FLV_CODECID_SCREEN = 3,    ///< Screen video编码
				FLV_CODECID_VP6 = 4,       ///< VP6编码
				FLV_CODECID_VP6A = 5,      ///< VP6 with alpha编码
				FLV_CODECID_SCREEN2 = 6,   ///< Screen video v2编码
				FLV_CODECID_H264 = 7,      ///< H.264/AVC编码（最常用）
				FLV_CODECID_REALH263 = 8,  ///< Real H.263编码
				FLV_CODECID_MPEG4 = 9,     ///< MPEG-4编码
			};
			// const char   kflv_muxerl[] = "libflv_rtc";
		}
		/**
		*  @brief FlvEncoder构造函数实现
		*  
		*  初始化FLV编码器，分配缓冲区，打开输出文件，发送HTTP响应头。
		*  
		*  初始化步骤：
		*  1. 保存网络连接指针
		*  2. 分配8MB输出缓冲区（out_buffer_）
		*  3. 分配8MB发送缓冲区（send_buffer_）
		*  4. 初始化时间戳和标志位
		*  5. 如果指定文件名，打开文件用于输出
		*  6. 如果有网络连接，发送HTTP响应头
		*  
		*  HTTP响应头内容：
		*  - HTTP/1.1 200 OK
		*  - Access-Control-Allow-Origin: *（允许跨域访问）
		*  - Content-Type: video/x-flv; charset=utf-8
		*  - Connection: Keep-Alive（保持连接）
		*/
		FlvEncoder::FlvEncoder(  libnetwork::Connection* conn, const char * out_flv_file_name)
			:connection_(conn) 
			, out_file_ptr_(NULL)
			, out_buffer_(new uint8_t[1024 * 1024 * 8])  // 分配8MB输出缓冲区
			, start_timestamp_(0)
			, send_sps_(false)
			, sps_("" )
			, pps_( "")
			, send_buffer_(new uint8_t[1024 * 1024 * 8])  // 分配8MB发送缓冲区
			, send_size_(0)
		{
			LIBFLV_LOG_T_F(LS_INFO);
			current_ = out_buffer_;
			prev_packet_size_ = 0;
			
			// 构造HTTP响应头
			std::stringstream ss;
			ss << "HTTP/1.1 200 OK \r\n";
			ss << "Access-Control-Allow-Origin:*\r\n";  // 允许跨域
			ss << "Content-Type: video/x-flv; charset=utf-8\r\n";  // FLV MIME类型
			ss << "Connection: Keep-Alive\r\n";  // 保持连接
			ss << "\r\n";  // 响应头结束
			
		 	// 如果指定了输出文件名，打开文件
			if (out_flv_file_name)
			{
				out_file_ptr_ = fopen(out_flv_file_name, "wb+");
			}
			
 			// 如果有网络连接，发送HTTP响应头
			if (connection_)
			{
				connection_->AsyncSend(ss.str());
			}
		}

		/**
		*  @brief FlvEncoder析构函数实现
		*  
		*  清理FLV编码器资源，释放缓冲区，关闭文件。
		*  
		*  清理步骤：
		*  1. 释放输出缓冲区（out_buffer_）
		*  2. 释放发送缓冲区（send_buffer_）
		*  3. 刷新并关闭输出文件
		*/
		FlvEncoder::~FlvEncoder()
		{
			LIBFLV_LOG_T_F(LS_INFO);
			// 释放输出缓冲区
			if (out_buffer_)
			{
				delete out_buffer_;
				out_buffer_ = NULL;
			}
			// 释放发送缓冲区
			if (send_buffer_)
			{
				delete[]send_buffer_;
				send_buffer_ = nullptr;
			}
			// 关闭输出文件
			if (out_file_ptr_)
			{
				fflush(out_file_ptr_);  // 刷新缓冲区
				fclose(out_file_ptr_);
				out_file_ptr_ = nullptr;
			}
		}

		
		/**
		*  @brief 发送FLV Header和onMetaData实现
		*  
		*  该方法发送FLV文件头和元数据Tag，是FLV流的第一个数据包。
		*  
		*  发送流程：
		*  1. 构造并发送FLV Header（9字节）
		*  2. 构造并发送onMetaData脚本数据Tag（使用AMF0格式）
		*  
		*  FLV Header结构：
		*  - 签名："FLV"（3字节）
		*  - 版本号：1（1字节）
		*  - 标志位：音频/视频标志（1字节）
		*  - 头长度：9（4字节，大端序）
		*  - PreviousTagSize0：0（4字节）
		*  
		*  onMetaData内容（AMF0格式）：
		*  - 字符串："onMetaData"
		*  - ECMA数组：包含视频和音频的元数据
		*    * 视频元数据：duration, videocodecid, videodatarate, framerate, width, height
		*    * 音频元数据：audiocodecid, audiodatarate, audiosamplerate, audiosamplesize, stereo
		*    * 编码器信息：encoder
		*/
		void FlvEncoder::SendFlvHeader(bool has_auido, bool has_video)
		{
			uint8_t * header = current_;
			int32_t  index = 0;
			
			// 构造FLV Header
			libflv::FLVHeader flv_header = { 0 };
			flv_header.flv[0] = 'F';
			flv_header.flv[1] = 'L';
			flv_header.flv[2] = 'V';
			flv_header.version = libflv::FLVHeader::kFlvVersion;  // 版本号=1
			flv_header.length = htonl(libflv::FLVHeader::kFlvHeaderLength);  // 头长度=9
			flv_header.have_video = has_video;  // 视频标志
			flv_header.have_audio = has_auido;  // 音频标志

			// 发送FLV Header
			Writer((const uint8_t *)&flv_header, sizeof(libflv::FLVHeader));
			
			// 构造onMetaData脚本数据Tag
			uint8_t * metadata = current_ ;
			uint8_t * ptr = metadata;
			uint8_t *end = ptr + (1024 * 1023);

			// 计算元数据项数量
			uint8_t count = (has_auido ? 5 : 0) + (has_video ? 7 : 0) + 1;
			
			// 写入"onMetaData"字符串
			ptr = AMFWriteString(ptr, end, "onMetaData", 10);
			
			// 写入ECMA数组类型和元素数量
			ptr[0] = AMF_ECMA_ARRAY;
			ptr[1] = (uint8_t)((count >> 24) & 0xFF);
			ptr[2] = (uint8_t)((count >> 16) & 0xFF);
			ptr[3] = (uint8_t)((count >> 8) & 0xFF);
			ptr[4] = (uint8_t)(count & 0xFF);
			ptr += 5;

			// 写入音频元数据
			if (has_auido)
			{
				ptr = AMFWriteNamedDouble(ptr, end, "audiocodecid", 12, 10);  // AAC编码ID=10
				ptr = AMFWriteNamedDouble(ptr, end, "audiodatarate", 13, 125);  // 音频码率=125kbps
				ptr = AMFWriteNamedDouble(ptr, end, "audiosamplerate", 15, 44100);  // 采样率=44.1kHz
				ptr = AMFWriteNamedDouble(ptr, end, "audiosamplesize", 15, 16);  // 采样精度=16bit
				ptr = AMFWriteNamedBoolean(ptr, end, "stereo", 6, (uint8_t)true);  // 立体声
			}
		
			// 写入视频元数据
			if (has_video)
			{
				ptr = AMFWriteNamedDouble(ptr, end, "duration", 8, 0);  // 视频时长（初始为0）
				ptr = AMFWriteNamedDouble(ptr, end, "videocodecid", 12, 7);  // H.264编码ID=7
				ptr = AMFWriteNamedDouble(ptr, end, "videodatarate", 13, 0);  // 视频码率（初始为0）
				ptr = AMFWriteNamedDouble(ptr, end, "framerate", 9, 25);  // 帧率=25fps
				ptr = AMFWriteNamedDouble(ptr, end, "height", 6, 2560);  // 视频高度=2560
				ptr = AMFWriteNamedDouble(ptr, end, "width", 5, 1440);  // 视频宽度=1440
			}
			
			// 写入编码器信息
			ptr = AMFWriteNamedString(ptr, end, "encoder", 7, kflv_muxer, strlen(kflv_muxer));
			
			// 写入对象结束标记
			ptr = AMFWriteObjectEnd(ptr, end);
			 
			// 发送onMetaData Tag（类型=18，时间戳=0）
			WriteFlvTag(libflv::kFlvMsgTypeAMFMeta, metadata, ptr - metadata, 0);
		}
		 
		//void FlvContext::SendFlvOnMetaHeader()
		//{
		//}

		/**
		*  @brief 发送FLV视频帧实现
		*  
		*  该方法将H.264编码的视频帧封装为FLV Video Tag并发送。
		*  
		*  处理流程：
		*  1. 解析H.264 NALU（使用起始码分割）
		*  2. 根据NALU类型进行处理：
		*     - SPS（7）：保存SPS数据
		*     - PPS（8）：保存PPS数据
		*     - IDR（5）：首次发送配置包，然后发送IDR帧
		*     - 非IDR（1）：发送P帧
		*  3. 将NALU封装为AVC格式（4字节长度前缀）
		*  4. 构造FLV Video Tag并发送
		*  
		*  NALU类型说明：
		*  - 7: SPS（Sequence Parameter Set）
		*  - 8: PPS（Picture Parameter Set）
		*  - 5: IDR（Instantaneous Decoder Refresh，关键帧）
		*  - 1: 非IDR（P帧或B帧）
		*  - 6: SEI（Supplemental Enhancement Information）
		*  - 9: AUD（Access Unit Delimiter）
		*/
		bool FlvEncoder::SendFlvVideoFrame(const rtc::CopyOnWriteBuffer & frame, uint64_t timestamp)
		{
			// 解析H.264 NALU（查找起始码00 00 00 01或00 00 01）
			std::vector<webrtc::H264::NaluIndex> nalus = webrtc::H264::FindNaluIndices(
				frame.data(), frame.size());
				
			// 遍历所有NALU
			for (int32_t nal_index = 0; nal_index < nalus.size(); ++nal_index)
			{
				webrtc::NaluInfo nalu;
				// 提取NALU类型（第一个字节的低5位）
				nalu.type = frame.data()[nalus[nal_index].payload_start_offset] & 0x1F;
				nalu.sps_id = -1;
				nalu.pps_id = -1;
				
				switch (nalu.type) {
				case webrtc::H264::NaluType::kSps: {
					// 保存SPS数据
					sps_ =  (std::string((const char *)(frame.data() + nalus[nal_index].payload_start_offset),
						nalus[nal_index].payload_size));
					break;
				}
				case webrtc::H264::NaluType::kPps: {
					// 保存PPS数据
					pps_ =  (std::string((char *)(frame.data() + nalus[nal_index].payload_start_offset),
						nalus[nal_index].payload_size));
					break;
				}
				case webrtc::H264::NaluType::kIdr:
				{
					// 处理IDR帧（关键帧）
					if (!send_sps_)
					{
						// 首次发送IDR帧前，先发送配置包（包含SPS/PPS）
						send_sps_ = true;
						LIBFLV_LOG_T_F(LS_INFO) << "send decoder config ...";
						WriteConfigPacket();
						start_timestamp_ = timestamp;  // 记录起始时间戳
					}
					
					uint8_t * buffer = out_buffer_;
					uint8_t *ptr = buffer;
					
					// 写入FrameType + CodecID（0x17 = 关键帧 + H.264）
					*ptr = FLV_CODECID_H264;
					*ptr++ |= FLV_FRAME_KEY;
					
					// 写入AVCPacketType（1 = NALU数据）
					*ptr++ = 1;
					
					// 写入CompositionTime（3字节，相对于DTS的偏移）
					set_be24(ptr, timestamp - start_timestamp_);
					ptr += 3;
					
					// 写入SPS（4字节长度 + SPS数据）
					set_be32(ptr, sps_.size());
					ptr += 4;
					memcpy(ptr, sps_.c_str(), sps_.size());
					ptr += sps_.size();
					
					// 写入PPS（4字节长度 + PPS数据）
					set_be32(ptr, pps_.size());
					ptr += 4;
					memcpy(ptr, pps_.c_str(), pps_.size());
					ptr += pps_.size();

					// 写入IDR NALU（4字节长度 + NALU数据）
					set_be32(ptr, nalus[nal_index].payload_size);
					ptr += 4;
					memcpy(ptr, frame.data() + nalus[nal_index].payload_start_offset, nalus[nal_index].payload_size);
					ptr += nalus[nal_index].payload_size;
					
					// 发送Video Tag（类型=9，时间戳相对于起始时间）
					WriteFlvTag(libflv::kFlvMsgTypeVideo, buffer,
						ptr - buffer, timestamp - start_timestamp_);
					break;
				}
				case webrtc::H264::NaluType::kSlice:  // P帧或B帧
				case webrtc::H264::NaluType::kAud:
				case webrtc::H264::NaluType::kEndOfSequence:
				case webrtc::H264::NaluType::kEndOfStream:
				case webrtc::H264::NaluType::kFiller:
				case webrtc::H264::NaluType::kSei:
				case webrtc::H264::NaluType::kStapA:
				case webrtc::H264::NaluType::kFuA:
				{
					if (!send_sps_)
					{
						// 必须先发送SPS/PPS/IDR，否则跳过非关键帧
						continue;
					}
					
					uint8_t * buffer = out_buffer_;
					uint8_t *ptr = buffer;
					
					// 写入FrameType + CodecID（0x27 = 非关键帧 + H.264）
					*ptr = FLV_CODECID_H264;
					*ptr++ |= FLV_FRAME_INTER;
					
					// 写入AVCPacketType（1 = NALU数据）
					*ptr++ = 1;
					
					// 写入CompositionTime（3字节）
					set_be24(ptr, timestamp - start_timestamp_);
					ptr += 3;

					// 写入NALU（4字节长度 + NALU数据）
					set_be32(ptr, nalus[nal_index].payload_size);
					ptr += 4;
					memcpy(ptr, frame.data() + nalus[nal_index].payload_start_offset, nalus[nal_index].payload_size);
					ptr += nalus[nal_index].payload_size;
					
					// 发送Video Tag
					WriteFlvTag(libflv::kFlvMsgTypeVideo, buffer,
						ptr - buffer, timestamp - start_timestamp_);
					break;
				}
				default: {
					break;
				}
				}
			}

			return true;
		}
		/**
		*  @brief 发送FLV音频帧实现
		*  
		*  该方法将AAC编码的音频帧封装为FLV Audio Tag并发送。
		*  
		*  FLV Audio Tag结构：
		*  - SoundFormat（4位）：10（AAC）
		*  - SoundRate（2位）：3（44kHz）
		*  - SoundSize（1位）：1（16-bit）
		*  - SoundType（1位）：1（立体声）
		*  - AACPacketType（1字节）：1（AAC raw数据）
		*  - AAC音频数据
		*/
		bool FlvEncoder::SendFlvAudioFrame(const rtc::CopyOnWriteBuffer & frame, uint64_t timestamp)
		{
			uint8_t * buffer = out_buffer_; 
			
			// 构造Audio Tag Header（2字节）
			// SoundFormat=10（AAC）, SoundRate=3（44kHz）, SoundSize=1（16-bit）, SoundType=1（立体声）
			buffer[0] = (10 << 4) | (3 << 2) | (1 << 1) | 1;
			
			// AACPacketType=1（AAC raw数据）
			buffer[1] = 1;
			
			// 复制AAC音频数据
			memcpy(buffer + 2, frame.data(), frame.size());
			
			// 发送Audio Tag（类型=8，时间戳相对于起始时间）
			WriteFlvTag(libflv::kFlvMsgTypeAudio, out_buffer_,
				frame.size() + 2, timestamp - start_timestamp_);
			
			return true;
		}

		

		/**
		*  @brief 写入AVC配置包实现
		*  
		*  该方法生成并发送AVCDecoderConfigurationRecord，包含SPS和PPS。
		*  
		*  AVCDecoderConfigurationRecord结构：
		*  - FrameType + CodecID（1字节）：0x17（关键帧 + AVC）
		*  - AVCPacketType（1字节）：0（AVC序列头）
		*  - CompositionTime（3字节）：0
		*  - configurationVersion（1字节）：1
		*  - AVCProfileIndication（1字节）：SPS[1]
		*  - profile_compatibility（1字节）：SPS[2]
		*  - AVCLevelIndication（1字节）：SPS[3]
		*  - lengthSizeMinusOne（1字节）：0xFF（NALU长度为4字节）
		*  - numOfSequenceParameterSets（1字节）：0xE1（1个SPS）
		*  - sequenceParameterSetLength（2字节）：SPS长度
		*  - sequenceParameterSetNALUnit：SPS数据
		*  - numOfPictureParameterSets（1字节）：1（1个PPS）
		*  - pictureParameterSetLength（2字节）：PPS长度
		*  - pictureParameterSetNALUnit：PPS数据
		*/
		void FlvEncoder::WriteConfigPacket()
		{
			uint8_t * buffer = out_buffer_;
			uint8_t *ptr = buffer;
			
			// 写入FrameType + CodecID（0x17 = 关键帧 + H.264）
			*ptr = FLV_CODECID_H264;
			*ptr++ |= FLV_FRAME_KEY;
			
			// 写入AVCPacketType（0 = AVC序列头）
			*ptr++ = 0;
			
			// 写入CompositionTime（3字节，配置包为0）
			*ptr++ = 0;
			*ptr++ = 0;
			*ptr++ = 0;
			
			// 构造AVCDecoderConfigurationRecord
			std::string extra_data;
			{
				// configurationVersion
				extra_data.push_back(1);
				
				// AVCProfileIndication（Profile）
				extra_data.push_back(sps_[1]);
				
				// profile_compatibility（兼容性）
				extra_data.push_back(sps_[2]);
				
				// AVCLevelIndication（Level）
				extra_data.push_back(sps_[3]);
				
				// lengthSizeMinusOne（0xFF表示NALU长度为4字节）
				extra_data.push_back((char)0xff);
				
				// numOfSequenceParameterSets（0xE1表示1个SPS）
				extra_data.push_back((char)0xe1);
				
				// SPS长度（2字节，大端序）
				uint16_t size = (uint16_t)sps_.size();
				size = htons(size);
				extra_data.append((char *)&size, 2);
				
				// SPS数据
				extra_data.append(sps_);
				
				// numOfPictureParameterSets（1个PPS）
				extra_data.push_back(1);
				
				// PPS长度（2字节，大端序）
				size = (uint16_t)pps_.size();
				size = htons(size);
				extra_data.append((char *)&size, 2);
				
				// PPS数据
				extra_data.append(pps_);
			}

			// 复制AVCDecoderConfigurationRecord到缓冲区
			memcpy(ptr, extra_data.c_str(), extra_data.size());
			ptr += extra_data.size();
			
			// 发送配置包（类型=9，时间戳=0）
			WriteFlvTag(libflv::kFlvMsgTypeVideo, buffer, ptr - buffer, 0);
		}


		/**
		*  @brief 写入FLV Tag实现
		*  
		*  该方法构造完整的FLV Tag结构并发送。
		*  
		*  FLV Tag结构：
		*  1. Tag Header（11字节）
		*  2. Tag Data（size字节）
		*  3. PreviousTagSize（4字节）
		*/
		void FlvEncoder::WriteFlvTag(uint8_t type, const uint8_t * data, int32_t size, int64_t time_stamp)
		{
			// 构造Tag Header
			libflv::FlvTagHeader header;
			header.type = type;  // Tag类型（8=音频，9=视频，18=脚本数据）
			set_be24(header.data_size, (uint32_t)size);  // Tag数据大小（24位大端序）
			header.timestamp_ex = (time_stamp >> 24) & 0xff;  // 时间戳高8位
			set_be24(header.timestamp, time_stamp & 0xFFFFFF);  // 时间戳低24位

			// 发送Tag Header
			std::string tag_header;
			tag_header.append((char *)&header, sizeof(header));
			Writer((const uint8_t *)tag_header.c_str(), tag_header.size());

			// 发送Tag Data
			Writer(data, size);

			// 发送PreviousTagSize（Tag Header + Tag Data的总大小）
			uint32_t PreviousTag_Size = htonl((uint32_t)(size + sizeof(header)));
			std::string PreviousTagSize;
			PreviousTagSize.append((char *)&PreviousTag_Size, 4);
			Writer((const uint8_t *)PreviousTagSize.c_str(), PreviousTagSize.size(), true);  // fflsh=true，立即发送
		}

		/**
		*  @brief 写入数据到输出实现
		*  
		*  该方法将数据写入文件和网络连接。
		*  使用发送缓冲区进行批量发送，减少网络调用次数。
		*  
		*  @param data 要写入的数据指针
		*  @param size 数据大小
		*  @param fflsh 是否立即刷新发送缓冲区
		*/
		void FlvEncoder::Writer(const uint8_t * data, int32_t size, bool fflsh)
		{
			// 写入文件（如果有）
			if (out_file_ptr_)
			{
				fwrite(data, 1, size, out_file_ptr_);
				fflush(out_file_ptr_);
			}
			
			// 追加到发送缓冲区
			memcpy(send_buffer_ + send_size_, data, size);
			send_size_ += size;
			
			// 如果需要刷新，发送缓冲区数据
			if (fflsh)
			{
				if (connection_)
				{
					connection_->AsyncSend(rtc::CopyOnWriteBuffer(send_buffer_, send_size_));
				}
				send_size_ = 0;  // 重置缓冲区大小
			}
		}
		 
	}
}