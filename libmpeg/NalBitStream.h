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
 
 *	created: 		2025-05-02
 *
 *	author:			chensong
 *
 *	purpose:		video encoder
 *	输赢不重要，答案对你们有什么意义才重要。
 *
 *	光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。
 *
 *
 *	我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
 *	然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
 *	3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
 *	然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
 *	于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
 *	我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
 *	从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
 *	我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
 *	沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
 *	安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
 ***********************************************************************************************/

 #ifndef _C_NAL_BIT_STREAM_H_
 #define _C_NAL_BIT_STREAM_H_
 #include <cstdint>
 namespace libmedia_transfer_protocol
 {
     namespace libmpeg
     {
         /**
         *  @author chensong
         *  @date 2025-10-09
         *  @brief NAL位流读取器类（NAL Bit Stream Reader）
         *  
         *  NalBitStream类用于读取和解析H264/H265视频编码中的NAL（Network Abstraction Layer）
         *  位流数据。它支持按位读取、按字节读取以及Exp-Golomb编码的解析。
         *  
         *  NAL单元结构（NAL Unit Structure）：
         *  
         *    0                   1                   2                   3
         *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *   |  forbidden_bit | nal_ref_idc | nal_unit_type                  |
         *   |  (1 bit)       |  (2 bits)   |  (5 bits)                      |
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *   :                        RBSP (Raw Byte Sequence Payload)      :
         *   :                        (variable length)                       :
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  
         *  NAL单元类型（NAL Unit Types）：
         *  - 1-5: VCL NALU（Video Coding Layer）：包含视频编码数据
         *    - 1: 非IDR图像的编码条带
         *    - 2: 编码条带数据分区A
         *    - 3: 编码条带数据分区B
         *    - 4: 编码条带数据分区C
         *    - 5: IDR图像的编码条带
         *  - 6: SEI（Supplemental Enhancement Information）
         *  - 7: SPS（Sequence Parameter Set）
         *  - 8: PPS（Picture Parameter Set）
         *  - 9: AUD（Access Unit Delimiter）
         *  
         *  位流读取格式（Bit Stream Reading Format）：
         *  
         *    0                   1                   2                   3
         *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *   |  Byte 0 (8 bits)                                              |
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *   |  Byte 1 (8 bits)                                              |
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *   :                                                               :
         *   |  Byte N (8 bits)                                              |
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *   |  Bit reading order: MSB (Most Significant Bit) first          |
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  
         *  Exp-Golomb编码格式（Exp-Golomb Encoding Format）：
         *  
         *    0                   1                   2                   3
         *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *   |  Leading Zeros (variable) | 1 | Info Bits (variable)          |
         *   |  (k bits)                  |   |  (k bits)                     |
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *   |  Example: value = 5                                          |
         *   |  Leading Zeros: 2 bits (00)                                  |
         *   |  Separator: 1 bit (1)                                        |
         *   |  Info Bits: 2 bits (01) -> value = 5                          |
         *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  
         *  @note NalBitStream用于解析H264/H265视频编码中的NAL单元数据
         *  @note 支持按位读取、按字节读取以及Exp-Golomb编码的解析
         *  @note 位流按MSB（最高有效位）优先的顺序读取
         *  
         *  使用示例：
         *  @code
         *  const char* nal_data = ...;
         *  int nal_size = ...;
         *  NalBitStream stream(nal_data, nal_size);
         *  uint32_t value = stream.GetUE();  // 读取Exp-Golomb编码的无符号整数
         *  int32_t signed_value = stream.GetSE();  // 读取Exp-Golomb编码的有符号整数
         *  @endcode
         */
         class NalBitStream 
         {
         public:
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 构造函数（Constructor）
             *  
             *  该构造函数用于创建NalBitStream实例。初始化位流读取器，
             *  设置数据指针和长度。
             *  
             *  初始化流程：
             *  1. 设置data_为指定的数据指针
             *  2. 设置len_为指定的数据长度
             *  3. 初始化bits_count_为0，表示当前字节中尚未读取的位数
             *  4. 初始化byte_idx_为0，表示当前字节索引
             *  5. 初始化byte_为0，表示当前读取的字节
             *  
             *  @param data 指向NAL单元数据的指针，不能为空
             *  @param len NAL单元数据的长度，单位为字节
             *  @note 数据指针和数据长度在对象生命周期内必须保持有效
             *  @note 位流按MSB（最高有效位）优先的顺序读取
             */
             NalBitStream(const char *data, int len);
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 读取一位（Get One Bit）
             *  
             *  该方法用于从位流中读取一位数据。按MSB（最高有效位）优先的顺序读取。
             *  
             *  位读取流程（Bit Reading Process）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  1. Check if bits_count_ == 0 (need new byte)                |
             *   |     - If yes, read next byte: byte_ = GetByte()               |
             *   |     - Set bits_count_ = 8                                    |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  2. Extract MSB from current byte                            |
             *   |     - bit = (byte_ >> (bits_count_ - 1)) & 0x01             |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  3. Decrement bits_count_                                    |
             *   |     - bits_count_--                                          |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  4. Return extracted bit                                     |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  位读取格式（Bit Reading Format）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  Byte: [b7 b6 b5 b4 b3 b2 b1 b0]                            |
             *   |  Reading order: MSB (b7) first, then b6, b5, ...            |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @return 返回读取的一位数据，值为0或1
             *  @note 位流按MSB（最高有效位）优先的顺序读取
             *  @note 当当前字节的所有位都被读取后，自动读取下一个字节
             *  
             *  使用示例：
             *  @code
             *  uint8_t bit = stream.GetBit();
             *  // bit 为 0 或 1
             *  @endcode
             */
             uint8_t GetBit();
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 读取多位（Get Multiple Bits as Word）
             *  
             *  该方法用于从位流中读取指定位数的数据，返回为16位无符号整数。
             *  
             *  多位读取格式（Multi-Bit Reading Format）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  Read bits: [b(n-1) b(n-2) ... b1 b0]                        |
             *   |  Result: value = b(n-1)*2^(n-1) + b(n-2)*2^(n-2) + ...      |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @param bits 要读取的位数，范围1-16
             *  @return 返回读取的指定位数的数据，返回值为16位无符号整数
             *  @note 位流按MSB（最高有效位）优先的顺序读取
             *  @note 如果bits大于16，只读取低16位
             *  
             *  使用示例：
             *  @code
             *  uint16_t value = stream.GetWord(12);  // 读取12位数据
             *  @endcode
             */
             uint16_t GetWord(int bits);
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 读取多位长整数（Get Multiple Bits as Long Integer）
             *  
             *  该方法用于从位流中读取指定位数的数据，返回为32位无符号整数。
             *  
             *  多位读取格式（Multi-Bit Reading Format）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  Read bits: [b(n-1) b(n-2) ... b1 b0]                        |
             *   |  Result: value = b(n-1)*2^(n-1) + b(n-2)*2^(n-2) + ...      |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @param bits 要读取的位数，范围1-32
             *  @return 返回读取的指定位数的数据，返回值为32位无符号整数
             *  @note 位流按MSB（最高有效位）优先的顺序读取
             *  @note 如果bits大于32，只读取低32位
             *  
             *  使用示例：
             *  @code
             *  uint32_t value = stream.GetBitLong(24);  // 读取24位数据
             *  @endcode
             */
             uint32_t GetBitLong(int bits);
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 读取多位64位整数（Get Multiple Bits as 64-bit Integer）
             *  
             *  该方法用于从位流中读取指定位数的数据，返回为64位无符号整数。
             *  
             *  多位读取格式（Multi-Bit Reading Format）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  Read bits: [b(n-1) b(n-2) ... b1 b0]                        |
             *   |  Result: value = b(n-1)*2^(n-1) + b(n-2)*2^(n-2) + ...      |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @param bits 要读取的位数，范围1-64
             *  @return 返回读取的指定位数的数据，返回值为64位无符号整数
             *  @note 位流按MSB（最高有效位）优先的顺序读取
             *  @note 如果bits大于64，只读取低64位
             *  
             *  使用示例：
             *  @code
             *  uint64_t value = stream.GetBit64(32);  // 读取32位数据
             *  @endcode
             */
             uint64_t GetBit64(int bits);
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 读取Exp-Golomb编码的无符号整数（Get Unsigned Exp-Golomb）
             *  
             *  该方法用于从位流中读取Exp-Golomb编码的无符号整数。
             *  Exp-Golomb编码是H264/H265视频编码中常用的变长编码方式。
             *  
             *  Exp-Golomb编码格式（Exp-Golomb Encoding Format）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  Leading Zeros (variable) | 1 | Info Bits (variable)          |
             *   |  (k bits)                  |   |  (k bits)                     |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  Decoding formula: value = (1 << k) - 1 + info_bits           |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  Exp-Golomb编码示例（Exp-Golomb Encoding Examples）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  value = 0: 1                                                |
             *   |  Binary: [1]                                                 |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  value = 1: 010                                              |
             *   |  Binary: [0][1][0]                                           |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  value = 5: 00110                                            |
             *   |  Binary: [00][1][10] -> Leading Zeros=2, Info=2             |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  解码流程（Decoding Process）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  1. Count leading zeros (k bits)                              |
             *   |     - Read bits until first '1' is found                      |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  2. Read separator bit ('1')                                  |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  3. Read info bits (k bits)                                   |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  4. Decode: value = (1 << k) - 1 + info_bits                 |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @return 返回Exp-Golomb编码解码后的无符号整数值
             *  @note Exp-Golomb编码是H264/H265视频编码中的标准变长编码方式
             *  @note 该方法用于解析SPS、PPS等参数集中的Exp-Golomb编码数据
             *  
             *  使用示例：
             *  @code
             *  uint32_t value = stream.GetUE();  // 读取Exp-Golomb编码的无符号整数
             *  // value 例如: 0, 1, 5, 10, ...
             *  @endcode
             */
             uint32_t GetUE();
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 读取Exp-Golomb编码的有符号整数（Get Signed Exp-Golomb）
             *  
             *  该方法用于从位流中读取Exp-Golomb编码的有符号整数。
             *  有符号Exp-Golomb编码通过无符号Exp-Golomb编码转换得到。
             *  
             *  有符号Exp-Golomb编码格式（Signed Exp-Golomb Encoding Format）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  1. Read unsigned Exp-Golomb value (code)                     |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  2. Convert to signed value:                                  |
             *   |     - If code is even: value = -(code / 2)                    |
             *   |     - If code is odd: value = (code + 1) / 2                  |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  有符号Exp-Golomb编码示例（Signed Exp-Golomb Encoding Examples）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  code=0: value = 0                                            |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  code=1: value = 1  (odd: (1+1)/2 = 1)                        |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  code=2: value = -1 (even: -2/2 = -1)                         |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  code=3: value = 2  (odd: (3+1)/2 = 2)                        |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  code=4: value = -2 (even: -4/2 = -2)                         |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @return 返回Exp-Golomb编码解码后的有符号整数值
             *  @note 有符号Exp-Golomb编码通过无符号Exp-Golomb编码转换得到
             *  @note 转换公式：偶数 -> 负值，奇数 -> 正值
             *  @note 该方法用于解析SPS、PPS等参数集中的有符号Exp-Golomb编码数据
             *  
             *  使用示例：
             *  @code
             *  int32_t value = stream.GetSE();  // 读取Exp-Golomb编码的有符号整数
             *  // value 例如: 0, 1, -1, 2, -2, ...
             *  @endcode
             */
             int32_t GetSE();
         private:
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 读取一个字节（Get One Byte）
             *  
             *  该方法用于从位流中读取一个字节数据。当按位读取需要新的字节时，
             *  会自动调用此方法。
             *  
             *  字节读取流程（Byte Reading Process）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  1. Check if byte_idx_ < len_ (has more data)                |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  2. Read byte: byte_ = data_[byte_idx_]                      |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  3. Increment byte_idx_                                      |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  4. Return byte_                                             |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @return 返回读取的一个字节数据
             *  @note 该方法在按位读取需要新字节时自动调用
             *  @note 如果已到达数据末尾，行为取决于实现
             */
             char GetByte();
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 数据指针（Data Pointer）
             *  
             *  该成员变量用于存储指向NAL单元数据的指针。数据指针指向
             *  要解析的NAL单元数据的开始位置。
             *  
             *  数据布局（Data Layout）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  data_[0]  |  data_[1]  |  data_[2]  |  ...  |  data_[len_-1]|
             *   |  (1 byte)  |  (1 byte)  |  (1 byte)  |       |  (1 byte)     |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  <-- data_ points here                                        |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @note 数据指针在构造函数中设置，指向NAL单元数据
             *  @note 数据指针在对象生命周期内必须保持有效
             */
             const char * data_;
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 数据长度（Data Length）
             *  
             *  该成员变量用于存储NAL单元数据的长度。长度表示可以读取的
             *  最大字节数。
             *  
             *  @note 数据长度在构造函数中设置，单位为字节
             *  @note 数据长度限制可以读取的最大字节数
             */
             int len_;
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 位计数器（Bit Counter）
             *  
             *  该成员变量用于存储当前字节中尚未读取的位数。当所有位都被
             *  读取后（bits_count_ == 0），需要读取下一个字节。
             *  
             *  位计数器状态（Bit Counter States）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  bits_count_ = 8: All bits available                          |
             *   |  bits_count_ = 0: Need new byte                               |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  bits_count_ range: 0-8                                       |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @note 初始值为0，表示尚未读取任何位
             *  @note 当bits_count_为0时，表示当前字节的所有位都已被读取
             *  @note 每读取一位，bits_count_会递减
             */
             int bits_count_;
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 字节索引（Byte Index）
             *  
             *  该成员变量用于存储当前读取位置的字节索引。索引从0开始，
             *  指向当前正在读取的字节。
             *  
             *  字节索引格式（Byte Index Format）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  data_[0]  |  data_[1]  |  data_[2]  |  ...  |  data_[len_-1]|
             *   |  byte_idx_ = 0          |  byte_idx_ = 1      |               |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @note 初始值为0，表示从第一个字节开始读取
             *  @note 字节索引从0开始，最大为len_-1
             *  @note 每读取一个字节，byte_idx_会递增
             */
             int byte_idx_;
 
             /**
             *  @author chensong
             *  @date 2025-10-09
             *  @brief 当前字节（Current Byte）
             *  
             *  该成员变量用于存储当前正在读取的字节。当需要读取新的字节时，
             *  会通过GetByte()方法更新此变量。
             *  
             *  当前字节格式（Current Byte Format）：
             *  
             *    0                   1                   2                   3
             *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *   |  byte_: [b7 b6 b5 b4 b3 b2 b1 b0]                            |
             *   |  Reading order: MSB (b7) first, then b6, b5, ...            |
             *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
             *  
             *  @note 初始值为0，表示尚未读取任何字节
             *  @note 当前字节通过GetByte()方法更新
             *  @note 位从MSB（最高有效位）开始读取
             */
             char byte_;
         };
     }
 }
 
 
 #endif // _C_NAL_BIT_STREAM_H_