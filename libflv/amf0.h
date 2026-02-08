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

 purpose:		AMF0 Format - AMF0格式编解码器
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

#ifndef _amf0_h_
#define _amf0_h_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
*  @brief AMF0数据类型枚举（AMF0 Data Type）
*  
*  AMF0（Action Message Format 0）是Adobe开发的一种数据序列化格式，
*  用于在Flash Player和服务器之间传输数据。FLV的onMetaData使用AMF0格式。
*  
*  数据类型说明：
*  - AMF_NUMBER (0x00)：双精度浮点数（8字节）
*  - AMF_BOOLEAN (0x01)：布尔值（1字节）
*  - AMF_STRING (0x02)：字符串（2字节长度 + 字符串内容）
*  - AMF_OBJECT (0x03)：对象（键值对集合）
*  - AMF_MOVIECLIP (0x04)：影片剪辑（保留，不使用）
*  - AMF_NULL (0x05)：空值
*  - AMF_UNDEFINED (0x06)：未定义值
*  - AMF_REFERENCE (0x07)：引用（指向已序列化的对象）
*  - AMF_ECMA_ARRAY (0x08)：ECMA数组（关联数组）
*  - AMF_OBJECT_END (0x09)：对象结束标记（0x00 0x00 0x09）
*  - AMF_STRICT_ARRAY (0x0A)：严格数组（索引数组）
*  - AMF_DATE (0x0B)：日期时间（8字节毫秒 + 2字节时区）
*  - AMF_LONG_STRING (0x0C)：长字符串（4字节长度 + 字符串内容）
*  - AMF_UNSUPPORTED (0x0D)：不支持的类型
*  - AMF_RECORDSET (0x0E)：记录集（保留，不使用）
*  - AMF_XML_DOCUMENT (0x0F)：XML文档
*  - AMF_TYPED_OBJECT (0x10)：类型化对象
*  - AMF_AVMPLUS_OBJECT (0x11)：AMF3对象
*  
*  @note FLV的onMetaData主要使用NUMBER、BOOLEAN、STRING、ECMA_ARRAY类型
*/
enum AMFDataType
{
	AMF_NUMBER = 0x00,          // 双精度浮点数
	AMF_BOOLEAN,                // 布尔值
	AMF_STRING,                 // 字符串
	AMF_OBJECT,                 // 对象
	AMF_MOVIECLIP,              // 影片剪辑（保留）
	AMF_NULL,                   // 空值
	AMF_UNDEFINED,              // 未定义值
	AMF_REFERENCE,              // 引用
	AMF_ECMA_ARRAY,             // ECMA数组
	AMF_OBJECT_END,             // 对象结束标记
	AMF_STRICT_ARRAY,           // 严格数组
	AMF_DATE,                   // 日期时间
	AMF_LONG_STRING,            // 长字符串
	AMF_UNSUPPORTED,            // 不支持的类型
	AMF_RECORDSET,              // 记录集（保留）
	AMF_XML_DOCUMENT,           // XML文档
	AMF_TYPED_OBJECT,           // 类型化对象
	AMF_AVMPLUS_OBJECT,         // AMF3对象
};

// ==================== AMF0写入函数（Write Functions）====================

/**
*  @brief 写入AMF0 Null值
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @return 更新后的写入位置指针，失败返回NULL
*/
uint8_t* AMFWriteNull(uint8_t* ptr, const uint8_t* end);

/**
*  @brief 写入AMF0 Undefined值
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @return 更新后的写入位置指针，失败返回NULL
*/
uint8_t* AMFWriteUndefined(uint8_t* ptr, const uint8_t* end);

/**
*  @brief 写入AMF0 Object标记（对象开始）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @return 更新后的写入位置指针，失败返回NULL
*/
uint8_t* AMFWriteObject(uint8_t* ptr, const uint8_t* end);

/**
*  @brief 写入AMF0 Object结束标记（0x00 0x00 0x09）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @return 更新后的写入位置指针，失败返回NULL
*/
uint8_t* AMFWriteObjectEnd(uint8_t* ptr, const uint8_t* end);

/**
*  @brief 写入AMF0 Typed Object标记（类型化对象开始）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @return 更新后的写入位置指针，失败返回NULL
*/
uint8_t* AMFWriteTypedObject(uint8_t* ptr, const uint8_t* end);

/**
*  @brief 写入AMF0 ECMA Array标记（ECMA数组开始）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @return 更新后的写入位置指针，失败返回NULL
*  @note ECMA数组用于存储键值对，类似于JavaScript的对象
*/
uint8_t* AMFWriteECMAArarry(uint8_t* ptr, const uint8_t* end);

/**
*  @brief 写入AMF0 Boolean值
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @param value 布尔值（0=false，非0=true）
*  @return 更新后的写入位置指针，失败返回NULL
*/
uint8_t* AMFWriteBoolean(uint8_t* ptr, const uint8_t* end, uint8_t value);

/**
*  @brief 写入AMF0 Number值（双精度浮点数）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @param value 双精度浮点数值
*  @return 更新后的写入位置指针，失败返回NULL
*  @note 使用大端序（Big-Endian）存储
*/
uint8_t* AMFWriteDouble(uint8_t* ptr, const uint8_t* end, double value);

/**
*  @brief 写入AMF0 String值
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @param string 字符串指针
*  @param length 字符串长度
*  @return 更新后的写入位置指针，失败返回NULL
*  @note 长度小于65536使用2字节长度，否则使用4字节长度
*/
uint8_t* AMFWriteString(uint8_t* ptr, const uint8_t* end, const char* string, size_t length);

/**
*  @brief 写入AMF0 Date值（日期时间）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @param milliseconds 毫秒时间戳（自1970-01-01 00:00:00 UTC）
*  @param timezone 时区偏移（分钟）
*  @return 更新后的写入位置指针，失败返回NULL
*/
uint8_t* AMFWriteDate(uint8_t* ptr, const uint8_t* end, double milliseconds, int16_t timezone);

/**
*  @brief 写入AMF0命名字符串（对象属性）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @param name 属性名称
*  @param length 属性名称长度
*  @param value 字符串值
*  @param length2 字符串值长度
*  @return 更新后的写入位置指针，失败返回NULL
*  @note 用于写入对象的字符串属性，格式：名称长度 + 名称 + 类型标记 + 值长度 + 值
*/
uint8_t* AMFWriteNamedString(uint8_t* ptr, const uint8_t* end, const char* name, size_t length, const char* value, size_t length2);

/**
*  @brief 写入AMF0命名数值（对象属性）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @param name 属性名称
*  @param length 属性名称长度
*  @param value 双精度浮点数值
*  @return 更新后的写入位置指针，失败返回NULL
*  @note 用于写入对象的数值属性，格式：名称长度 + 名称 + 类型标记 + 值
*/
uint8_t* AMFWriteNamedDouble(uint8_t* ptr, const uint8_t* end, const char* name, size_t length, double value);

/**
*  @brief 写入AMF0命名布尔值（对象属性）
*  @param ptr 当前写入位置指针
*  @param end 缓冲区结束位置指针
*  @param name 属性名称
*  @param length 属性名称长度
*  @param value 布尔值
*  @return 更新后的写入位置指针，失败返回NULL
*  @note 用于写入对象的布尔属性，格式：名称长度 + 名称 + 类型标记 + 值
*/
uint8_t* AMFWriteNamedBoolean(uint8_t* ptr, const uint8_t* end, const char* name, size_t length, uint8_t value);

// ==================== AMF0读取函数（Read Functions）====================

/**
*  @brief 读取AMF0 Null值
*  @param ptr 当前读取位置指针
*  @param end 缓冲区结束位置指针
*  @return 更新后的读取位置指针，失败返回NULL
*/
const uint8_t* AMFReadNull(const uint8_t* ptr, const uint8_t* end);

/**
*  @brief 读取AMF0 Undefined值
*  @param ptr 当前读取位置指针
*  @param end 缓冲区结束位置指针
*  @return 更新后的读取位置指针，失败返回NULL
*/
const uint8_t* AMFReadUndefined(const uint8_t* ptr, const uint8_t* end);

/**
*  @brief 读取AMF0 Boolean值
*  @param ptr 当前读取位置指针
*  @param end 缓冲区结束位置指针
*  @param value 输出参数，存储读取的布尔值
*  @return 更新后的读取位置指针，失败返回NULL
*/
const uint8_t* AMFReadBoolean(const uint8_t* ptr, const uint8_t* end, uint8_t* value);

/**
*  @brief 读取AMF0 Number值（双精度浮点数）
*  @param ptr 当前读取位置指针
*  @param end 缓冲区结束位置指针
*  @param value 输出参数，存储读取的双精度浮点数
*  @return 更新后的读取位置指针，失败返回NULL
*/
const uint8_t* AMFReadDouble(const uint8_t* ptr, const uint8_t* end, double* value);

/**
*  @brief 读取AMF0 String值
*  @param ptr 当前读取位置指针
*  @param end 缓冲区结束位置指针
*  @param isLongString 是否为长字符串（0=短字符串，1=长字符串）
*  @param string 输出缓冲区，存储读取的字符串
*  @param length 输出缓冲区大小
*  @return 更新后的读取位置指针，失败返回NULL
*/
const uint8_t* AMFReadString(const uint8_t* ptr, const uint8_t* end, int isLongString, char* string, size_t length);

/**
*  @brief 读取AMF0 Date值（日期时间）
*  @param ptr 当前读取位置指针
*  @param end 缓冲区结束位置指针
*  @param milliseconds 输出参数，存储毫秒时间戳
*  @param timezone 输出参数，存储时区偏移（分钟）
*  @return 更新后的读取位置指针，失败返回NULL
*/
const uint8_t* AMFReadDate(const uint8_t* ptr, const uint8_t* end, double *milliseconds, int16_t *timezone);


/**
*  @brief AMF0对象项结构（AMF Object Item）
*  
*  用于描述AMF0对象的一个属性项，包含类型、名称、值和大小。
*/
struct amf_object_item_t
{
	enum AMFDataType type;  // 数据类型
	const char* name;       // 属性名称
	void* value;            // 属性值指针
	size_t size;            // 值的大小或数组元素个数
};

/**
*  @brief 读取AMF0对象项数组
*  @param data 数据起始位置指针
*  @param end 数据结束位置指针
*  @param items 对象项数组
*  @param count 对象项数量
*  @return 更新后的读取位置指针，失败返回NULL
*  @note 用于批量读取AMF0对象的多个属性
*/
const uint8_t* amf_read_items(const uint8_t* data, const uint8_t* end, struct amf_object_item_t* items, size_t count);

#ifdef __cplusplus
}
#endif
#endif /* !_amf0_h_ */
