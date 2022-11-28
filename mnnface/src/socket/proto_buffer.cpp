/******************************************************************************

                  ��Ȩ���� (C), 2019-2029, SDC OS ��Դ����С������

 ******************************************************************************
  �� �� ��   : proto_buffer.cpp
  �� �� ��   : ����
  ��    ��   : athina
  ��������   : 2020��7��4��
  ����޸�   :
  ��������   : ��Ϣ�����װ
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��7��4��
    ��    ��   : athina
    �޸�����   : �����ļ�

******************************************************************************/
#include "proto_buffer.h"
#include <memory.h>


ProtoBuffer::ProtoBuffer(void) : m_used(0)
{
    memset(m_array.data(), 0, m_array.size());
}

ProtoBuffer::~ProtoBuffer(void)
{
    m_used = 0;
}

size_t ProtoBuffer::GetUsed(void) const
{
    return m_used;
}

size_t ProtoBuffer::GetSize(void) const
{
    return m_array.size();
}

const uint8_t* ProtoBuffer::GetData(void) const
{
    return m_array.data();
}

bool ProtoBuffer::Full(void) const
{
    return m_used == m_array.size() ? true : false;
}

bool ProtoBuffer::Empty(void) const
{
    return m_used == 0 ? true : false;
}

int32_t ProtoBuffer::WriteBytes(const void *buf, size_t len)
{
    if (m_used + len <= m_array.size()) {
        memcpy(m_array.data() + m_used, buf, len);
        m_used += len;
        return len;
    }
    return -1;
}


void ProtoBuffer::Clear(void)
{
    m_used = 0;
    memset(m_array.data(), 0, m_array.size());
}

ProtoBuffer& operator<<(ProtoBuffer &is, uint8_t i)
{
    is.WriteBytes(&i, sizeof(uint8_t));
    return is;
}

ProtoBuffer& operator<<(ProtoBuffer &is, uint16_t i)
{
    is.WriteBytes(&i, sizeof(uint16_t));
    return is;
}

ProtoBuffer& operator<<(ProtoBuffer &is, uint32_t i)
{
    is.WriteBytes(&i, sizeof(uint32_t));
    return is;
}

ProtoBuffer& operator<<(ProtoBuffer &is, uint64_t i)
{
    is.WriteBytes(&i, sizeof(uint64_t));
    return is;
}

ProtoBuffer& operator<<(ProtoBuffer &is, const std::string &s)
{
    is.WriteBytes(s.c_str(), s.length());
    return is;
}
