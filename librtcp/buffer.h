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
 created: 		2025-11-09

 author:			chensong

 purpose:		Buffer Management Classes for RTCP
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


#ifndef LIBRTCP_RTCP_BUFFER_H_
#define LIBRTCP_RTCP_BUFFER_H_
#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <functional> 
#include <atomic>
#include <stdexcept>
#include <cstring>


namespace libmedia_transfer_protocol {
    namespace librtcp { 

        /**
        *  @author chensong
        *  @date 2025-11-09
        *  @brief 对象统计模板类（Object Statistics Template）
        *  
        *  该模板类用于统计特定类型对象的实例数量。通过继承该类，可以自动跟踪
        *  对象的创建和销毁，用于内存泄漏检测和性能分析。
        *  
        *  工作原理：
        *  - 构造函数中递增计数器
        *  - 析构函数中递减计数器
        *  - 使用原子操作保证线程安全
        *  
        *  @tparam C 需要统计的类类型
        *  @note 使用原子变量保证线程安全
        *  @note 需要配合StatisticImp宏定义静态计数器
        */
        template <class C>
        class ObjectStatistic {
        public:
            /**
            *  @brief 构造函数，递增对象计数器
            */
            ObjectStatistic() {
                ++getCounter();
            }

            /**
            *  @brief 析构函数，递减对象计数器
            */
            ~ObjectStatistic() {
                --getCounter();
            }

            /**
            *  @brief 获取当前对象实例数量
            *  @return 当前存活的对象数量
            */
            static size_t count() {
                return getCounter().load();
            }

        private:
            /**
            *  @brief 获取静态计数器引用
            *  @return 原子计数器的引用
            *  @note 需要在cpp文件中使用StatisticImp宏实现
            */
            static std::atomic<size_t>& getCounter();
        };

        /**
        *  @brief 对象统计实现宏
        *  
        *  该宏用于在cpp文件中实现ObjectStatistic模板类的静态计数器。
        *  为每个需要统计的类型生成独立的静态原子变量。
        *  
        *  使用示例：
        *  @code
        *  // 在cpp文件中
        *  StatisticImp(Buffer)
        *  StatisticImp(BufferRaw)
        *  @endcode
        */
#define StatisticImp(Type)  \
    template<> \
    std::atomic<size_t>& ObjectStatistic<Type>::getCounter(){ \
        static std::atomic<size_t> instance(0); \
        return instance; \
    }



        /**
        *  @brief 指针类型判断模板（Pointer Type Trait）
        *  
        *  该模板用于编译期判断类型是否为指针类型（包括原始指针和智能指针）。
        *  用于BufferOffset类的模板参数推导。
        */
template <typename T> struct is_pointer : public std::false_type {};
template <typename T> struct is_pointer<std::shared_ptr<T> > : public std::true_type {};
template <typename T> struct is_pointer<std::shared_ptr<T const> > : public std::true_type {};
template <typename T> struct is_pointer<T*> : public std::true_type {};
template <typename T> struct is_pointer<const T*> : public std::true_type {};

/**
*  @author chensong
*  @date 2025-11-09
*  @brief 缓存基类（Buffer Base Class）
*  
*  Buffer是所有缓存类的抽象基类，定义了缓存对象的基本接口。
*  该类提供了数据访问、大小查询、字符串转换等基本功能。
*  
*  设计模式：
*  - 使用抽象基类定义接口
*  - 派生类实现具体的内存管理策略
*  - 支持智能指针管理生命周期
*  
*  派生类：
*  - BufferOffset: 偏移缓存，支持零拷贝的数据视图
*  - BufferRaw: 原始指针缓存，手动管理内存
*  - BufferLikeString: 类字符串缓存，支持字符串操作
*  
*  @note 该类不可拷贝，只能通过智能指针传递
*  @note 使用ObjectStatistic统计对象数量
*/
class Buffer /*: public noncopyable*/ {
public:
    using Ptr = std::shared_ptr<Buffer>;

    Buffer() = default;
    virtual ~Buffer() = default;

    /**
    *  @brief 获取数据指针
    *  @return 指向缓存数据的指针
    */
    virtual char *data() const = 0;

    /**
    *  @brief 获取数据大小
    *  @return 有效数据的字节数
    */
    virtual size_t size() const = 0;

    /**
    *  @brief 转换为字符串
    *  @return 包含缓存数据的字符串对象
    */
    virtual std::string toString() const {
        return std::string(data(), size());
    }

    /**
    *  @brief 获取容量大小
    *  @return 缓存的总容量（字节数）
    *  @note 默认实现返回size()，派生类可以重写
    */
    virtual size_t getCapacity() const {
        return size();
    }

private:
    // 对象个数统计
    ObjectStatistic<Buffer> _statistic;
};

template <typename C>
class BufferOffset : public  Buffer {
public:
    using Ptr = std::shared_ptr<BufferOffset>;

    BufferOffset(C data, size_t offset = 0, size_t len = 0) : _data(std::move(data)) {
        setup(offset, len);
    }

    ~BufferOffset() override = default;

    char *data() const override {
        return const_cast<char *>(getPointer<C>(_data)->data()) + _offset;
    }

    size_t size() const override {
        return _size;
    }

    std::string toString() const override {
        return std::string(data(), size());
    }

private:
    void setup(size_t offset = 0, size_t size = 0) {
        auto max_size = getPointer<C>(_data)->size();
        assert(offset + size <= max_size);
        if (!size) {
            size = max_size - offset;
        }
        _size = size;
        _offset = offset;
    }

    template<typename T>
    static typename std::enable_if<::libmedia_transfer_protocol::librtcp::is_pointer<T>::value, const T &>::type
    getPointer(const T &data) {
        return data;
    }

    template<typename T>
    static typename std::enable_if<!::libmedia_transfer_protocol::librtcp::is_pointer<T>::value, const T *>::type
    getPointer(const T &data) {
        return &data;
    }

private:
    C _data;
    size_t _size;
    size_t _offset;
};

using BufferString = BufferOffset<std::string>;

//指针式缓存对象，  [AUTO-TRANSLATED:c8403290]
//Pointer-style cache object,
class BufferRaw : public Buffer {
public:
    using Ptr = std::shared_ptr<BufferRaw>;

    static Ptr create();

    ~BufferRaw() override {
        if (_data) {
            delete[] _data;
        }
    }

    //在写入数据时请确保内存是否越界  [AUTO-TRANSLATED:5602043e]
    //When writing data, please ensure that the memory does not overflow
    char *data() const override {
        return _data;
    }

    //有效数据大小  [AUTO-TRANSLATED:b8dcbda7]
    //Effective data size
    size_t size() const override {
        return _size;
    }

    //分配内存大小  [AUTO-TRANSLATED:cce87adf]
    //Allocated memory size
    void setCapacity(size_t capacity) {
        if (_data) {
            do {
                if (capacity > _capacity) {
                    //请求的内存大于当前内存，那么重新分配  [AUTO-TRANSLATED:65306424]
                    //If the requested memory is greater than the current memory, reallocate
                    break;
                }

                if (_capacity < 2 * 1024) {
                    //2K以下，不重复开辟内存，直接复用  [AUTO-TRANSLATED:056416c0]
                    //Less than 2K, do not repeatedly allocate memory, reuse directly
                    return;
                }

                if (2 * capacity > _capacity) {
                    //如果请求的内存大于当前内存的一半，那么也复用  [AUTO-TRANSLATED:c189d660]
                    //If the requested memory is greater than half of the current memory, also reuse
                    return;
                }
            } while (false);

            delete[] _data;
        }
        _data = new char[capacity];
        _capacity = capacity;
    }

    //设置有效数据大小  [AUTO-TRANSLATED:efc4fb3e]
    //Set valid data size
    virtual void setSize(size_t size) {
        if (size > _capacity) {
            throw std::invalid_argument("Buffer::setSize out of range");
        }
        _size = size;
    }

    //赋值数据  [AUTO-TRANSLATED:0b91b213]
    //Assign data
    void assign(const char *data, size_t size = 0) {
        if (size <= 0) {
            size = strlen(data);
        }
        setCapacity(size + 1);
        memcpy(_data, data, size);
        _data[size] = '\0';
        setSize(size);
    }

    size_t getCapacity() const override {
        return _capacity;
    }

protected:
     

    BufferRaw(size_t capacity = 0) {
        if (capacity) {
            setCapacity(capacity);
        }
    }

    BufferRaw(const char *data, size_t size = 0) {
        assign(data, size);
    }

private:
    size_t _size = 0;
    size_t _capacity = 0;
    char *_data = nullptr;
    //对象个数统计  [AUTO-TRANSLATED:3b43e8c2]
    //Object count statistics
    ObjectStatistic<BufferRaw> _statistic;
};

class BufferLikeString : public Buffer {
public:
    ~BufferLikeString() override = default;

    BufferLikeString() {
        _erase_head = 0;
        _erase_tail = 0;
    }

    BufferLikeString(std::string str) {
        _str = std::move(str);
        _erase_head = 0;
        _erase_tail = 0;
    }

    BufferLikeString &operator=(std::string str) {
        _str = std::move(str);
        _erase_head = 0;
        _erase_tail = 0;
        return *this;
    }

    BufferLikeString(const char *str) {
        _str = str;
        _erase_head = 0;
        _erase_tail = 0;
    }

    BufferLikeString &operator=(const char *str) {
        _str = str;
        _erase_head = 0;
        _erase_tail = 0;
        return *this;
    }

    BufferLikeString(BufferLikeString &&that) {
        _str = std::move(that._str);
        _erase_head = that._erase_head;
        _erase_tail = that._erase_tail;
        that._erase_head = 0;
        that._erase_tail = 0;
    }

    BufferLikeString &operator=(BufferLikeString &&that) {
        _str = std::move(that._str);
        _erase_head = that._erase_head;
        _erase_tail = that._erase_tail;
        that._erase_head = 0;
        that._erase_tail = 0;
        return *this;
    }

    BufferLikeString(const BufferLikeString &that) {
        _str = that._str;
        _erase_head = that._erase_head;
        _erase_tail = that._erase_tail;
    }

    BufferLikeString &operator=(const BufferLikeString &that) {
        _str = that._str;
        _erase_head = that._erase_head;
        _erase_tail = that._erase_tail;
        return *this;
    }

    char *data() const override {
        return (char *) _str.data() + _erase_head;
    }

    size_t size() const override {
        return _str.size() - _erase_tail - _erase_head;
    }

    BufferLikeString &erase(size_t pos = 0, size_t n = std::string::npos) {
        if (pos == 0) {
            //移除前面的数据  [AUTO-TRANSLATED:b025d3c5]
            //Remove data from the front
            if (n != std::string::npos) {
                //移除部分  [AUTO-TRANSLATED:a650bef2]
                //Remove part
                if (n > size()) {
                    //移除太多数据了  [AUTO-TRANSLATED:64460d15]
                    //Removed too much data
                    throw std::out_of_range("BufferLikeString::erase out_of_range in head");
                }
                //设置起始便宜量  [AUTO-TRANSLATED:7a0250bd]
                //Set starting offset
                _erase_head += n;
                data()[size()] = '\0';
                return *this;
            }
            //移除全部数据  [AUTO-TRANSLATED:3d016f79]
            //Remove all data
            _erase_head = 0;
            _erase_tail = _str.size();
            data()[0] = '\0';
            return *this;
        }

        if (n == std::string::npos || pos + n >= size()) {
            //移除末尾所有数据  [AUTO-TRANSLATED:efaf1165]
            //Remove all data from the end
            if (pos >= size()) {
                //移除太多数据  [AUTO-TRANSLATED:dc9347c3]
                //Removed too much data
                throw std::out_of_range("BufferLikeString::erase out_of_range in tail");
            }
            _erase_tail += size() - pos;
            data()[size()] = '\0';
            return *this;
        }

        //移除中间的  [AUTO-TRANSLATED:fd25344c]
        //Remove the middle
        if (pos + n > size()) {
            //超过长度限制  [AUTO-TRANSLATED:9ae84929]
            //Exceeds the length limit
            throw std::out_of_range("BufferLikeString::erase out_of_range in middle");
        }
        _str.erase(_erase_head + pos, n);
        return *this;
    }

    BufferLikeString &append(const BufferLikeString &str) {
        return append(str.data(), str.size());
    }

    BufferLikeString &append(const std::string &str) {
        return append(str.data(), str.size());
    }

    BufferLikeString &append(const char *data) {
        return append(data, strlen(data));
    }

    BufferLikeString &append(const char *data, size_t len) {
        if (len <= 0) {
            return *this;
        }
        if (_erase_head > _str.capacity() / 2) {
            moveData();
        }
        if (_erase_tail == 0) {
            _str.append(data, len);
            return *this;
        }
        _str.insert(_erase_head + size(), data, len);
        return *this;
    }

    void push_back(char c) {
        if (_erase_tail == 0) {
            _str.push_back(c);
            return;
        }
        data()[size()] = c;
        --_erase_tail;
        data()[size()] = '\0';
    }

    BufferLikeString &insert(size_t pos, const char *s, size_t n) {
        _str.insert(_erase_head + pos, s, n);
        return *this;
    }

    BufferLikeString &assign(const char *data) {
        return assign(data, strlen(data));
    }

    BufferLikeString &assign(const char *data, size_t len) {
        if (len <= 0) {
            return *this;
        }
        if (data >= _str.data() && data < _str.data() + _str.size()) {
            _erase_head = data - _str.data();
            if (data + len > _str.data() + _str.size()) {
                throw std::out_of_range("BufferLikeString::assign out_of_range");
            }
            _erase_tail = _str.data() + _str.size() - (data + len);
            return *this;
        }
        _str.assign(data, len);
        _erase_head = 0;
        _erase_tail = 0;
        return *this;
    }

    void clear() {
        _erase_head = 0;
        _erase_tail = 0;
        _str.clear();
    }

    char &operator[](size_t pos) {
        if (pos >= size()) {
            throw std::out_of_range("BufferLikeString::operator[] out_of_range");
        }
        return data()[pos];
    }

    const char &operator[](size_t pos) const {
        return (*const_cast<BufferLikeString *>(this))[pos];
    }

    size_t capacity() const {
        return _str.capacity();
    }

    void reserve(size_t size) {
        _str.reserve(size);
    }

    void resize(size_t size, char c = '\0') {
        _str.resize(size, c);
        _erase_head = 0;
        _erase_tail = 0;
    }

    bool empty() const {
        return size() <= 0;
    }

    std::string substr(size_t pos, size_t n = std::string::npos) const {
        if (n == std::string::npos) {
            //获取末尾所有的  [AUTO-TRANSLATED:8a0b92b6]
            //Get all at the end
            if (pos >= size()) {
                throw std::out_of_range("BufferLikeString::substr out_of_range");
            }
            return _str.substr(_erase_head + pos, size() - pos);
        }

        //获取部分  [AUTO-TRANSLATED:d01310a4]
        //Get part
        if (pos + n > size()) {
            throw std::out_of_range("BufferLikeString::substr out_of_range");
        }
        return _str.substr(_erase_head + pos, n);
    }

private:
    void moveData() {
        if (_erase_head) {
            _str.erase(0, _erase_head);
            _erase_head = 0;
        }
    }

private:
    size_t _erase_head;
    size_t _erase_tail;
    std::string _str;
    //对象个数统计  [AUTO-TRANSLATED:3b43e8c2]
    //Object count statistics
    ObjectStatistic<BufferLikeString> _statistic;
};
}
}//namespace  
#endif // 
