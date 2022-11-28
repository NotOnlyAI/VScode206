/******************************************************************************

                  ??????? (C), 2019-2029, SDC OS ???????��??????

 ******************************************************************************
  ?? ?? ??   : proto_buffer.h
  ?? ?? ??   : ????
  ??    ??   : athina
  ????????   : 2020??7??4??
  ??????   :
  ????????   : ?????????
  ?????��?   :
  ??????   :
  1.??    ??   : 2020??7??4??
    ??    ??   : athina
    ???????   : ???????

******************************************************************************/
#ifndef __PROTO_STREAM_H__
#define __PROTO_STREAM_H__

#include <string>
#include <array>


class ProtoBuffer
{
public:
    ProtoBuffer(void);
    virtual ~ProtoBuffer(void);
 
    size_t GetUsed(void) const;
    size_t GetSize(void) const;
    const uint8_t* GetData(void) const;
    bool   Full(void) const;
    bool   Empty(void) const;
    int32_t WriteBytes(const void *buf, size_t len);
    void   Clear(void);

    friend ProtoBuffer& operator<<(ProtoBuffer &is, uint8_t  i);
    friend ProtoBuffer& operator<<(ProtoBuffer &is, uint16_t i);
    friend ProtoBuffer& operator<<(ProtoBuffer &is, uint32_t i);
    friend ProtoBuffer& operator<<(ProtoBuffer &is, uint64_t i);
    friend ProtoBuffer& operator<<(ProtoBuffer &is, const std::string &s);

private:
    static const int MAX_BUF_SIZE = 10000;
    typedef std::array<uint8_t, ProtoBuffer::MAX_BUF_SIZE> Array;
    Array m_array;
    size_t m_used;
};

#endif /* __PROTO_STREAM_H__ */