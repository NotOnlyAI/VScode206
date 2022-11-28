/*
* Copyright (c) Huawei Technologies Co., Ltd. 2020-2026. All rights reserved.
* Description: mempool 
* Author: l00427296
* Create: 2020-2-29
* Notes: ITGT TLV to common TLV
*/
#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <map>
#include <atomic> 
#include <memory>
#include <mutex> 
#include <vector> 
#include "securec.h"
#include "sdc_def_ext.h"
#include "utils_misc.h"
#include "sdc.h"

const uint32_t BASICBUFLEN = 1024;
const uint32_t BUFPOOLSIZE = 1;

#define TLVBUFMGR TlvBufMgr::GetInstance()

// 基础内存封装
class BaseBuffer {
public:
    explicit BaseBuffer(const size_t &size) : size_(size) {
        if (size_) {
            data_ = new(std::nothrow) uint8_t[size_]();
            if (data_ == nullptr)
            {
                return;
            }
        }
        else {
            data_ = nullptr;
        }
    }
    virtual ~BaseBuffer() {
        if (data_) {
            delete[] data_;
            data_ = nullptr;
        }
    }
    void resize(const size_t size) {
        uint8_t *newData = new(std::nothrow) uint8_t[size]();
        if (nullptr == newData)  //252971
        {
            size_ = 0;
            return;
        }

        auto func = [&newData]() {
            if (nullptr != newData)
            {
                delete[] newData;
                newData = nullptr;
            }
        };
        size_t copyLength = std::min(size, size_);
        INT64 lResult = memcpy_s(newData, copyLength, data_, copyLength);
        ITGT_RETURN_DO_IF_FAIL(DEMOYOLOLOG, (OK == lResult), func, "Copy Data Failed %lld", lResult);
        delete[] data_;
        data_ = newData;
        size_ = size;
    }
    const size_t size() const {
        return size_;
    }
    uint8_t *data() const {
        return data_;
    }

    uint8_t *data_ = nullptr;
protected:
    size_t size_ = 0;
private:
    BaseBuffer(const BaseBuffer & BaseBuffer);            // 不允许赋值构造
    BaseBuffer & operator=(const BaseBuffer & BaseBuffer);// 不允许赋值构造
};


// tlv内存 支持动态伸缩
class TlvBuf :public BaseBuffer {
public:
    explicit TlvBuf(size_t size = 1024) : BaseBuffer(size){ }

    ~TlvBuf() { 
    }

    int32_t AppandBufHead(uint32_t type, size_t len)
    {
        auto exceptLen = currentIndex_  + sizeof(type) + sizeof(uint32_t);
        // relloc buf is too small
        if (exceptLen > size_) {
            exceptLen = std::max(size_ * 2, exceptLen);
            resize(exceptLen);
        }
        // type
        auto ret = memcpy_s(data_ + currentIndex_, sizeof(type), &type, sizeof(type));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += sizeof(type);
        
        // len
        uint32_t valueLen = len;
        std::vector<uint32_t> emptyVecor(4);
        ret = memcpy_s(data_ + currentIndex_, sizeof(valueLen), emptyVecor.data(), sizeof(valueLen));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += len;

        return ITGT_SUCCESS;
    }

    int32_t AppandBufChannel(uint32_t type, size_t len)
    {
        auto exceptLen = currentIndex_  + sizeof(type) + sizeof(uint32_t);
        // relloc buf is too small
        if (exceptLen > size_) {
            exceptLen = std::max(size_ * 2, exceptLen);
            resize(exceptLen);
        }

        // type
        auto ret = memcpy_s(data_ + currentIndex_, sizeof(type), &type, sizeof(type));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += sizeof(type);

        // len
        uint32_t valueLen = sizeof(size_t);
        std::vector<uint32_t> emptyVecor(4);
        emptyVecor[0] = 0x01;
        ret = memcpy_s(data_ + currentIndex_, sizeof(valueLen), emptyVecor.data(), sizeof(valueLen));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += valueLen;

        // value
        std::vector<uint32_t> emptyVecor2(4);
        emptyVecor2[0] = 0x65;
        ret = memcpy_s(data_ + currentIndex_, 4, emptyVecor2.data(), 4);
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += 1;

        return ITGT_SUCCESS;
    }

    int32_t AppandBufStr(uint32_t type, const char* buf, size_t len)
    {
        auto exceptLen = currentIndex_ + len + sizeof(type) + sizeof(uint32_t);
        // relloc buf is too small
        if (exceptLen > size_) {
            exceptLen = std::max(size_ * 2, exceptLen);
            resize(exceptLen);
        }

        // type
        auto ret = memcpy_s(data_ + currentIndex_, sizeof(type), &type, sizeof(type));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += sizeof(type);

        // len
        // uint32_t valueLen = len + sizeof(uint32_t); // comment this line valueLen calculate wrong
        uint32_t valueLen = uint32_t(len);
        ret = memcpy_s(data_ + currentIndex_, sizeof(uint32_t), &valueLen, sizeof(uint32_t));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += sizeof(uint32_t);

        // value
        ret = memcpy_s(data_ + currentIndex_, len, buf, len);
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += len;
        return ITGT_SUCCESS;
    }

    // notice 字符串需要自己加'0'
    int32_t AppandBuf(uint32_t type, const char* buf, size_t len)
    {
        auto exceptLen = currentIndex_ + len + sizeof(type) + sizeof(uint32_t);
        // relloc buf is too small
        if (exceptLen > size_) {
            exceptLen = std::max(size_ * 2, exceptLen);
            resize(exceptLen);
        }

        // type
        auto ret = memcpy_s(data_ + currentIndex_, sizeof(type), &type, sizeof(type));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += sizeof(type);

        // len
        // uint32_t valueLen = len + sizeof(uint32_t); // comment this line valueLen calculate wrong
        uint32_t valueLen = uint32_t(len); 
        ret = memcpy_s(data_ + currentIndex_, sizeof(uint32_t), &valueLen, sizeof(uint32_t));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += sizeof(uint32_t);

        // value
        ret = memcpy_s(data_ + currentIndex_, len, buf, len);
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += len;
        return ITGT_SUCCESS;
    }

    // 专门针对属性的
    int32_t AppandPair(uint32_t type, const char* firstBuf, size_t firstLen, const char* secBuf, size_t secLen) 
    {
        uint32_t valueLen = firstLen + secLen;

        auto exceptLen = currentIndex_ + valueLen  + sizeof(type) + 4;

        // relloc buf is too small
        if (exceptLen > size_) {
            exceptLen = std::max(size_ * 2, exceptLen);
            resize(exceptLen);
        }
        
        // type
        auto ret = memcpy_s(data_ + currentIndex_, sizeof(type), &type, sizeof(type));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += sizeof(type);

        valueLen += sizeof(valueLen);
        // len
        ret = memcpy_s(data_ + currentIndex_, sizeof(valueLen), &valueLen, sizeof(valueLen));
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += sizeof(valueLen);

        // value1
        ret = memcpy_s(data_ + currentIndex_, firstLen, firstBuf, firstLen);
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += firstLen;
        
        std::vector<uint32_t> emptyVecor1(1);
        std::vector<uint32_t> emptyVecor2(1);
        emptyVecor1[0] = 0x5c;
        emptyVecor2[0] = 0x30;
        ret = memcpy_s(data_ + currentIndex_, 1, emptyVecor1.data(), 1);
        ret = memcpy_s(data_ + currentIndex_ + 1, 1, emptyVecor2.data(), 1);
        currentIndex_ += 2;
        // value2
        ret = memcpy_s(data_ + currentIndex_, secLen, secBuf, secLen);
        if (ret != OK) {
            return ret;
        }
        currentIndex_ += secLen;
        ret = memcpy_s(data_ + currentIndex_, 1, emptyVecor1.data(), 1);
        ret = memcpy_s(data_ + currentIndex_ + 1, 1, emptyVecor2.data(), 1);
        
        currentIndex_ +=2;
        return OK;
    }

    size_t CurrentIndex() {
        return currentIndex_;
    }

    void ZeroIndex() {
        currentIndex_ = 0;
    }

private:
    size_t currentIndex_ = 0;
    uint32_t file_num = 0;
};

// 内存管理
class TlvBufMgr :public Singleton<TlvBufMgr>{
public:
    TlvBufMgr() {

    }
    ~TlvBufMgr() {
        LOG_ERROR("Destory TlvBufMgr Sucess");
    }

    // 初始化内存
    int32_t Init() {
        if (!bufPool_) {
            bufPool_ = std::make_shared<ObjPool<TlvBuf>>();
            ITGT_RETURN_VAL_IF_FAIL(DEMOYOLOLOG, bufPool_, ITGT_OUT_OF_MEM, "failed to malloc buf for ObjPool<TlvBuf>");
            for (uint32_t i = 0; i < BUFPOOLSIZE; i++) {
                auto ret = bufPool_->AddMemory(BASICBUFLEN);
                ITGT_RETURN_VAL_IF_FAIL(DEMOYOLOLOG, ret == 0, ITGT_OUT_OF_MEM, "failed to AddMemory len %u", BASICBUFLEN);
            }
        }
        ITGT_INFO_PRINT(DEMOYOLOLOG, "#### bufPool_ size is %zu this %p", bufPool_->size(), this);
        return OK;
    }

    std::shared_ptr<TlvBuf> GetBuf() {
        return bufPool_->GetObj();
    }

    int32_t StashBuf(const char* addr, const std::shared_ptr<TlvBuf>& bufPtr) {
        std::unique_lock<std::mutex> _(mapMtx_);
        // TIPS bufPool_的大小 get后会缩小的
        // ITGT_RETURN_VAL_IF_FAIL(DEMOYOLOLOG, bufMap_.size() < bufPool_->total(), ITGT_FAIL, "Invailed value current size %zu bufPool size %zu this is %p ", bufMap_.size(), bufPool_->total(), this);
        bufMap_.emplace(addr, bufPtr);
        LOG_DEBUG("stash addr %p", addr);
        return OK;
    }

    // release buf
    int32_t ReleaseBuf(const char* addr, size_t bufLen) {
        std::unique_lock<std::mutex> _(mapMtx_);

        // ITGT_RETURN_VAL_IF_FAIL(DEMOYOLOLOG, bufMap_.find(addr) != bufMap_.end(), ITGT_FAIL, "Buf addr %p  is not stash", addr);
        // ITGT_RETURN_VAL_IF_FAIL(DEMOYOLOLOG, bufMap_[addr]->CurrentIndex() == bufLen, ITGT_FAIL,
            // "buf len is not match in %zu stash %zu", bufLen, bufMap_[addr]->CurrentIndex());

        // 这里需要手动清理下index，不是很好  后面内存池可以加个注入接口
        bufMap_[addr]->ZeroIndex();

        bufMap_.erase(addr);
        return OK;
    }

private:
    std::map<const char *, std::shared_ptr<TlvBuf>> bufMap_;
    std::mutex mapMtx_;
    std::shared_ptr<ObjPool<TlvBuf>> bufPool_ = nullptr;
};

#endif //MEMPOOL_H