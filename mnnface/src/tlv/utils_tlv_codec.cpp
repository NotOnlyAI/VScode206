/******************************************************************************

                  ��Ȩ���� (C), 2001-2016, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : utils_tlv_codec.cpp
  �� �� ��   : ����
  ��    ��   : �� �S/chenyun c00193875
  ��������   : 2016��08��05�� 09:34:47
  ����޸�   :
  ��������   : tlv�����ʵ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��08��05�� 09:34:47
    ��    ��   : �� �S/chenyun c00193875
    �޸�����   : �����ļ�

******************************************************************************/

#include <string.h>
#include "utils_tlv_codec.h"
#include "sdc_def_ext.h"

/* -----------------------------------------------------------------------------
 * TLV�ຯ��
 *----------------------------------------------------------------------------- */
#define         LOGCOMMON              "LogCommon"            // ����ģ��ͷ�ļ�


TLV::~TLV()
{
    delete[] m_pucValue;
    m_pucValue = NULL;
}

template<class T> 
TLV::TLV(UINT32 nType, T Val):m_nType(nType), m_uLength(0), m_pucValue(NULL)
{
    Init(&Val, sizeof(T));
}

TLV::TLV(UINT32 nType, const CHAR *Val):m_nType(nType), m_uLength(0), m_pucValue(NULL)
{
    Init(Val, strlen(Val) + 1);
}

TLV::TLV(UINT32 nType, const UCHAR *Val, UINT32 uLength):m_nType(nType), m_uLength(0), m_pucValue(NULL)
{
    Init(Val, uLength);
}

TLV::TLV(UINT32 nType, const std::vector<BinaryPutData> &putdata):m_nType(nType), m_uLength(0),
    m_pucValue(NULL)
{
    INT64 lResult = ITGT_FAIL;

    UINT64 len = 0;

    for (size_t i = 0; i < putdata.size(); i++) {
        ITGT_RETURN_IF_FAIL(LOGCOMMON, putdata[i].len > 0,
                            "invalid %d len, index %d", putdata[i].len, (INT32)i);

        ITGT_RETURN_IF_FAIL(LOGCOMMON, putdata[i].data != nullptr,
                            "nullptr pointer, index %d", (INT32)i);

        len += putdata[i].len;
        ITGT_RETURN_IF_FAIL(LOGCOMMON, len == (UINT32)len,
                            "sum over flow, len %lld index %d", len, (INT32)i);
    }

    m_pucValue = new (std::nothrow) UCHAR[(UINT32)len];
    ITGT_RETURN_IF_FAIL(LOGCOMMON, m_pucValue != nullptr,
                        "new data failed len %lld", len);

    m_uLength = (UINT32)len;

    auto func = [&]() {
        if (m_pucValue != nullptr) {
            delete[] m_pucValue;
            m_pucValue = nullptr;
            m_uLength = 0;
        }
    };

    UINT32 offset = 0;
    for (auto &it : putdata) {
        ITGT_RETURN_DO_IF_FAIL(LOGCOMMON, offset < m_uLength, func,
                               "offset<%d> over max len<%d>", offset, m_uLength);

        lResult = memcpy_s(m_pucValue + offset, m_uLength - offset, it.data, it.len);
        ITGT_RETURN_DO_IF_FAIL(LOGCOMMON, lResult == EOK, func, "memcpy failed with %lld", lResult);

        offset += it.len;
    }
}

/*****************************************************************************
��ʼ��Len��Val�ֶ�
*****************************************************************************/
void TLV::Init(const void *pVal, UINT32 uLength)
{
    INT64 lResult = ITGT_FAIL;
    ITGT_LOG_IF_FAIL(LOGCOMMON, 0 != uLength, "<<%s>>", ITGT_GetErrString(ITGT_INVALID_PARAM));
    m_uLength = uLength;
    m_pucValue = new (std::nothrow) UCHAR[uLength];
    ITGT_LOG_IF_FAIL(LOGCOMMON, m_pucValue != NULL, "<<%s>>", ITGT_GetErrString(ITGT_OUT_OF_MEM));

    lResult = memcpy_s(m_pucValue, uLength, pVal, uLength);
    ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
}

TLV_Codec::~TLV_Codec()
{
    Clear();
}

/*****************************************************************************
���TLV�����
*****************************************************************************/
void TLV_Codec::Clear(void)
{
    // ɾ��TLV����
    if (nullptr != m_pucTlvBuf) {
        delete[] m_pucTlvBuf;
        m_pucTlvBuf = nullptr;
    }

    m_ulDataLen = 0;

    std::multimap<INT32, TLV *>::iterator iter;

    for (iter = m_TLVMap.begin(); iter != m_TLVMap.end(); iter++) {
        // ɾ��TLV����
        delete iter->second;
        iter->second = nullptr;
        // ITGT_NOTICE_PRINT("delete tlv obj Success!\n");
    }

    // ���map
    m_TLVMap.clear();
}

/*****************************************************************************
��TLVָ�����Map
*****************************************************************************/
INT64 TLV_Codec::InsertValueToMap(TLV *pTlvObj)
{
    // ������TLV����
    m_TLVMap.insert(std::pair<INT32, TLV *>(pTlvObj->m_nType, pTlvObj));
    m_ulDataLen += (sizeof(INT32) + sizeof(UINT32) + pTlvObj->m_uLength);

    ITGT_RETURN_VAL_IF_FAIL(LOGCOMMON, (pTlvObj->m_uLength > 0) && (pTlvObj->m_pucValue != nullptr),
                            ITGT_FAIL, "type %x pTlvObj<%d,%p> is illegal", pTlvObj->m_nType, pTlvObj->m_uLength, pTlvObj->m_pucValue);

    return ITGT_SUCCESS;
}

// ���ݷ���
INT64 TLV_Encoder::PutValue(UINT32 nType, const UCHAR *data, INT32 len)
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "type %d \n", nType);

    TLV *pTlvObj = new(std::nothrow) TLV(nType, data, len);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, pTlvObj != NULL, ITGT_OUT_OF_MEM, "TLV new error. type %d \n", nType);

    return InsertValueToMap(pTlvObj);
}

// �ֶζ��������ݷ���
INT64 TLV_Encoder::PutValue(UINT32 nType, const std::vector<BinaryPutData> &putdata)
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "type %d \n", nType);

    TLV *pTlvObj = new(std::nothrow) TLV(nType, putdata);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, pTlvObj != NULL, ITGT_OUT_OF_MEM, "type %d \n", nType);

    return InsertValueToMap(pTlvObj);
}

/*****************************************************************************
���������ݽ���TLV����
*****************************************************************************/
INT64 TLV_Encoder::Encode(void)
{
    INT64 lResult = ITGT_FAIL;
    UINT32 nType;
    UINT32 uLength;
    std::multimap<INT32, TLV *>::iterator iter;
    UINT64 ulWoffset;  // дƫ��

    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "\n");

    m_pucTlvBuf = new (std::nothrow) UCHAR[m_ulDataLen];
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_pucTlvBuf != NULL, ITGT_OUT_OF_MEM, "\n");

    ulWoffset = 0;
    for (iter = m_TLVMap.begin(); iter != m_TLVMap.end(); iter++) {
        // ����TYPE�ֶ�
        nType = iter->second->m_nType;
        lResult = memcpy_s(m_pucTlvBuf + ulWoffset, (m_ulDataLen - ulWoffset), &nType, sizeof(INT32));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += sizeof(INT32);

        // ����LENGTH�ֶ�
        uLength = iter->second->m_uLength;
        lResult = memcpy_s(m_pucTlvBuf + ulWoffset, (m_ulDataLen - ulWoffset), &uLength, sizeof(UINT32));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += sizeof(UINT32);

        // ����VALUE
        lResult = memcpy_s(m_pucTlvBuf + ulWoffset, (m_ulDataLen - ulWoffset), iter->second->m_pucValue, uLength);
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += uLength;
    }

    return ITGT_SUCCESS;
}

/*****************************************************************************
���������ݽ���TLV����
*****************************************************************************/
INT64 TLV_Encoder::Encode(UCHAR* pucDataBuf, UINT64 ulDataLen)
{
    INT64 lResult = ITGT_FAIL;
    UINT32 nType;
    UINT32 uLength;
    std::multimap<INT32, TLV *>::iterator iter;
    UINT64 ulWoffset;  // дƫ��

    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "\n");

    ulWoffset = 0;
    for (iter = m_TLVMap.begin(); iter != m_TLVMap.end(); iter++) {
        // ����TYPE�ֶ�
        nType = iter->second->m_nType;
        lResult = memcpy_s(pucDataBuf + ulWoffset, (ulDataLen - ulWoffset), &nType, sizeof(INT32));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += sizeof(INT32);

        // ����LENGTH�ֶ�
        uLength = iter->second->m_uLength;
        lResult = memcpy_s(pucDataBuf + ulWoffset, (ulDataLen - ulWoffset), &uLength, sizeof(UINT32));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += sizeof(UINT32);

        // ����VALUE
        lResult = memcpy_s(pucDataBuf + ulWoffset, (ulDataLen - ulWoffset), iter->second->m_pucValue, uLength);
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += uLength;
    }

    return ITGT_SUCCESS;
}


// ���ݷ���
INT64 TLV_Perf_Encoder::PutValue(UINT32 nType, TLV_Encoder *pTlvEncoder)
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "type %d \n", nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.find(nType) == m_TLVMap.end(), ITGT_TLV_RESULT_EXIST, "type %d \n", nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, pTlvEncoder != NULL, ITGT_NULL_PTR, "type %d \n", nType);

    return InsertValueToMap(nType, pTlvEncoder);
}

// ���ݷ���
INT64 TLV_Perf_Encoder::PutValue(UINT32 nType, TLV_Perf_Encoder *pTlvEncoder)
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "type %d \n", nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_PerfTLVMap.find(nType) == m_PerfTLVMap.end(), ITGT_TLV_RESULT_EXIST, "type %d \n", nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, pTlvEncoder != NULL, ITGT_NULL_PTR, "type %d \n", nType);

    return InsertValueToMap(nType, pTlvEncoder);
}

/*****************************************************************************
��TLV_Encoderָ�����Map
*****************************************************************************/
INT64 TLV_Perf_Encoder::InsertValueToMap(UINT32 nType, TLV_Encoder *pTlvEncoder)
{
    // ������TLV����
    m_TLVMap.insert(std::pair<INT32, TLV_Encoder *>((INT32)nType, pTlvEncoder));
    m_ulDataLen += (sizeof(INT32) + sizeof(UINT32) + pTlvEncoder->GetTlvDataLen());

    ITGT_RETURN_VAL_IF_FAIL(LOGCOMMON, (pTlvEncoder->GetTlvDataLen() > 0) && (pTlvEncoder->GetTlvBuf() == nullptr),
                            ITGT_FAIL, "type %x pTlvEncoder<%llu,%p> is illegal", nType, pTlvEncoder->GetTlvDataLen(), pTlvEncoder->GetTlvBuf());

    return ITGT_SUCCESS;
}

/*****************************************************************************
��TLV_Encoderָ�����Map
*****************************************************************************/
INT64 TLV_Perf_Encoder::InsertValueToMap(UINT32 nType, TLV_Perf_Encoder *pTlvEncoder)
{
    // ������TLV����
    m_PerfTLVMap.insert(std::pair<INT32, TLV_Perf_Encoder *>((INT32)nType, pTlvEncoder));
    m_ulDataLen += (sizeof(INT32) + sizeof(UINT32) + pTlvEncoder->GetTlvDataLen());

    ITGT_RETURN_VAL_IF_FAIL(LOGCOMMON, (pTlvEncoder->GetTlvDataLen() > 0) && (pTlvEncoder->GetTlvBuf() == nullptr),
                            ITGT_FAIL, "type %x pTlvEncoder<%llu,%p> is illegal", nType, pTlvEncoder->GetTlvDataLen(), pTlvEncoder->GetTlvBuf());

    return ITGT_SUCCESS;
}

TLV_Perf_Encoder::~TLV_Perf_Encoder()
{
    Clear();
}

void TLV_Perf_Encoder::Clear(void)
{
    m_TLVMap.clear();
    m_PerfTLVMap.clear();

    TLV_Encoder::Clear();
}

/*****************************************************************************
���������ݽ���TLV����
*****************************************************************************/
INT64 TLV_Perf_Encoder::Encode(void)
{
    INT64 lResult = ITGT_FAIL;
    UINT32 nType;
    UINT32 uLength;
    std::multimap<INT32, TLV_Perf_Encoder *>::iterator iter;
    UINT64 ulWoffset;  // дƫ��

    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "\n");

    m_pucTlvBuf = new (std::nothrow) UCHAR[m_ulDataLen];
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_pucTlvBuf != NULL, ITGT_OUT_OF_MEM, "\n");

    ulWoffset = 0;
    for (iter = m_PerfTLVMap.begin(); iter != m_PerfTLVMap.end(); iter++) {
        // ����TYPE�ֶ�
        nType = iter->first;
        lResult = memcpy_s(m_pucTlvBuf + ulWoffset, (m_ulDataLen - ulWoffset), &nType, sizeof(INT32));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += sizeof(INT32);

        // ����LENGTH�ֶ�
        uLength = iter->second->GetTlvDataLen();
        lResult = memcpy_s(m_pucTlvBuf + ulWoffset, (m_ulDataLen - ulWoffset), &uLength, sizeof(UINT32));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += sizeof(UINT32);

        // ����VALUE
        lResult = iter->second->Encode(m_pucTlvBuf + ulWoffset, (m_ulDataLen - ulWoffset));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += uLength;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Perf_Encoder::Encode(UCHAR* pucDataBuf, UINT64 ulDataLen)
{
    INT64 lResult = ITGT_FAIL;
    UINT32 nType;
    UINT32 uLength;
    std::multimap<INT32, TLV_Encoder *>::iterator iter;
    UINT64 ulWoffset;  // дƫ��

    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "\n");

    ulWoffset = 0;
    for (iter = m_TLVMap.begin(); iter != m_TLVMap.end(); iter++) {
        // ����TYPE�ֶ�
        nType = iter->first;
        lResult = memcpy_s(pucDataBuf + ulWoffset, (ulDataLen - ulWoffset), &nType, sizeof(INT32));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += sizeof(INT32);

        // ����LENGTH�ֶ�
        uLength = iter->second->GetTlvDataLen();
        lResult = memcpy_s(pucDataBuf + ulWoffset, (ulDataLen - ulWoffset), &uLength, sizeof(UINT32));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += sizeof(UINT32);

        // ����VALUE
        lResult = iter->second->Encode(pucDataBuf + ulWoffset, (ulDataLen - ulWoffset));
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        ulWoffset += uLength;
    }

    return ITGT_SUCCESS;
}

UINT64 TLV_Decoder::GetValueCount(UINT32 nType) const
{
    return m_TLVMap.count(nType);
}

UINT64 TLV_Decoder::GetCount() const
{
    return m_TLVMap.size();
}

INT64 TLV_Decoder::GetValueLen(UINT32 nType, UINT32 *puLength, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        puLength[i] = iter_beg->second->m_uLength;
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, bool *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(bool *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, CHAR *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(CHAR *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, INT16 *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(INT16 *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, INT32 *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(INT32 *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }
    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, INT64 *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(INT64 *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, UCHAR *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(UCHAR *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, UINT16 *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(UINT16 *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, UINT32 *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    if (iter_beg == iter_end) {
        return ITGT_OBJ_NOT_FIND;
    }

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(UINT32 *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, UINT64 *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    if (iter_beg == iter_end) {
        return ITGT_OBJ_NOT_FIND;
    }

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        Val[i] = (*(UINT64 *)(iter_beg->second->m_pucValue));
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, CHAR *Val[], UINT64 *pulLength, UINT64 ulBufCnt) const
{
    INT64 lResult = ITGT_FAIL;

    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT64 ulBufLen;
    UINT32 i = 0;
    while (iter_beg != iter_end) {
        ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, pulLength[i] >= iter_beg->second->m_uLength, ITGT_BUF_TOO_SMALL,
                                         "\n");

        // �����ϼ�ֱ����pulLength��ΪINOUT����
        // IN:Val��Buffer�ĳ���
        // OUT:������ValռBuffer�ĳ���
        ulBufLen = pulLength[i];
        pulLength[i] = iter_beg->second->m_uLength;

        lResult = memset_s(Val[i], ulBufLen, 0, ulBufLen);
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);
        lResult = memcpy_s(Val[i], ulBufLen, iter_beg->second->m_pucValue, pulLength[i]);
        ITGT_LOG_IF_FAIL(LOGCOMMON, (EOK == lResult), "Return with error 0x%llx", lResult);

        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

INT64 TLV_Decoder::GetValue(UINT32 nType, UCHAR *Val[], UINT64 *pulLength, UINT64 ulBufCnt) const
{
    return GetValue(nType, (CHAR **)Val, pulLength, ulBufCnt);
}

INT64 TLV_Decoder::GetValue(UINT32 nType, TLV_Decoder *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, TLV *>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, TLV *>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    if (iter_beg == iter_end) {
        ITGT_ERROR_PRINT(LOGCOMMON, "ErrCode Description:Obj Not Find nType:%0x", nType);
        for (std::multimap<INT32, TLV *>::const_iterator iter = m_TLVMap.begin(); iter != m_TLVMap.end(); iter++) {
            ITGT_INFO_PRINT(LOGCOMMON, "m_TLVMap:%d", iter->first);
        }
        return ITGT_FAIL;
    }

    INT64 lResult = ITGT_FAIL;
    UINT32 i = 0;
    while (iter_beg != iter_end) {
        lResult = Val[i].Decode(iter_beg->second->m_pucValue,
                                iter_beg->second->m_uLength);
        ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, ITGT_SUCCESS == lResult, lResult, "\n");

        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

/*****************************************************************************
TLV����
*****************************************************************************/
INT64 TLV_Decoder::Decode(UCHAR *pucTlvBuf, UINT64 ulDataLen)
{
    INT64 lResult = ITGT_FAIL;
    UINT32 nType;
    UINT32 uLength;
    UINT64 ulWoffset;  // дƫ��
    TLV *pTlvObj = nullptr;

    // �������
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, pucTlvBuf != NULL, ITGT_NULL_PTR, "\n");

    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, NULL == m_pucTlvBuf, ITGT_TLV_RESULT_EXIST, "\n");

    m_pucTlvBuf = new (std::nothrow) UCHAR[ulDataLen];
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_pucTlvBuf != NULL, ITGT_OUT_OF_MEM, "\n");

    m_ulDataLen = ulDataLen;

    lResult = memcpy_s(m_pucTlvBuf, ulDataLen, pucTlvBuf, ulDataLen);
    ITGT_RETURN_VAL_IF_FAIL(LOGCOMMON, (EOK == lResult), ITGT_FAIL, "Return with error 0x%llx", lResult);

    ulWoffset = 0;
    while (ulWoffset < ulDataLen) {
        nType = (*(INT32 *)(m_pucTlvBuf + ulWoffset));
        ulWoffset += sizeof(INT32);

        // ITGT_NOTICE_PRINT("Tlv Decode : nType 0x%x\n",nType);

        uLength = (*(UINT32 *)(m_pucTlvBuf + ulWoffset));
        if (0 == uLength) {
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : uLength:%d\n", uLength);
            return ITGT_FAIL;
        }
        ulWoffset += sizeof(UINT32);
        ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, uLength <= (ulDataLen - ulWoffset), ITGT_INVALID_PARAM,
                                         "Invalid Tlv Obj Length %u!,Buf Remain Length %llu\n",
                                         uLength, (ulDataLen - ulWoffset));

        // ITGT_NOTICE_PRINT("Tlv Decode : uLength %llu\n",uLength);

        // UCHAR * pucVal = m_pucTlvBuf + ulWoffset;

        // ITGT_NOTICE_PRINT("Tlv Decode : Val 0x%x\n",*pucVal);

        if (ulWoffset >= ulDataLen) {
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : ulWoffset:%llu\n", ulWoffset);
            return ITGT_FAIL;
        }

        pTlvObj = new(std::nothrow) TLV(nType, m_pucTlvBuf + ulWoffset, uLength);
        ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, pTlvObj != NULL, ITGT_OUT_OF_MEM, "pTlvobj is null");

        (void)InsertValueToMap(pTlvObj);
        // InsertValueToMap(new TLV(nType,m_pucTlvBuf + ulWoffset,uLength));

        ulWoffset += uLength;
    }

    return ITGT_SUCCESS;
}
/*****************************************************************
                    EffieientTLVDecoder ��Ա����
*******************************************************************/
// ���ݽ���
INT64 EffieientTLVDecoder::Decode(UCHAR *pucTlvBuf, UINT64 ulDataLen)
{
    UINT32 nType;
    UINT32 uLength;
    UINT64 ulWoffset;  // дƫ��

    // �������
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, pucTlvBuf != NULL, ITGT_NULL_PTR, "\n");

    ulWoffset = 0;
    while (ulWoffset < ulDataLen) {
        nType = (*(INT32 *)(pucTlvBuf + ulWoffset));
        ulWoffset += sizeof(INT32);
        if (ulWoffset >= ulDataLen) {
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : Head T:%d ,Head L:%d\n",
                                             (*(INT32 *)(pucTlvBuf)), (*(INT32 *)(pucTlvBuf + sizeof(INT32))));
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : ulDataLen:%llu ,ulWoffset:%llu\n", ulDataLen, ulWoffset);
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : nType: %u\n", nType);
            return ITGT_FAIL;
        }

        uLength = (*(UINT32 *)(pucTlvBuf + ulWoffset));
        if (0 == uLength) {
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : Head T:%d ,Head L:%d\n",
                                             (*(INT32 *)(pucTlvBuf)), (*(INT32 *)(pucTlvBuf + sizeof(INT32))));
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : ulDataLen:%llu, uLength:%d\n", ulDataLen, uLength);
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : nType: %u\n", nType);
            return ITGT_FAIL;
        }

        ulWoffset += sizeof(UINT32);
        ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, uLength <= (ulDataLen - ulWoffset), ITGT_INVALID_PARAM,
                                         "Invalid Tlv Obj Length %u!,Buf Remain Length %llu\n",
                                         uLength, (ulDataLen - ulWoffset));

        if (ulWoffset >= ulDataLen) {
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : Head T:%d ,Head L:%d\n",
                                             (*(INT32 *)(pucTlvBuf)), (*(INT32 *)(pucTlvBuf + sizeof(INT32))));
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : ulDataLen:%llu, ulWoffset:%llu\n",ulDataLen, ulWoffset);
            ITGT_ERROR_PRINT(LOGCOMMON, "Tlv Decode : nType: %u\n", nType);
            return ITGT_FAIL;
        }

        EffieientTLV tmp ((CHAR *)(pucTlvBuf + ulWoffset), uLength, nType);

        // �������ݵ�map
        m_TLVMap.insert(std::pair<INT32, EffieientTLV>(tmp.type, tmp));

        ulWoffset += uLength;
    }

    return ITGT_SUCCESS;
}

// ��ȡ���͸���
UINT64 EffieientTLVDecoder::GetValueCount(UINT32 nType) const
{
    return m_TLVMap.count(nType);
}

UINT64 EffieientTLVDecoder::GetCount() const
{
    return m_TLVMap.size();
}

// ��ȡֵ���� ��Ҫ���ڶ�����������
INT64 EffieientTLVDecoder::GetValueLen(UINT32 nType, UINT32 *puLength, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, EffieientTLV>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, EffieientTLV>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        puLength[i] = iter_beg->second.length;
        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

// ��ȡ���������� ֻ��ȡָ�� ����������
INT64 EffieientTLVDecoder::GetValue(UINT32 nType, CHAR *Val[], UINT64 *pulLength, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, EffieientTLV>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, EffieientTLV>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    UINT32 i = 0;
    while (iter_beg != iter_end) {
        pulLength[i] = iter_beg->second.length;

        // ֻ��ֵָ��
        Val[i] = (CHAR *)iter_beg->second.data;

        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}

// ��ȡ���������� ֻ��ȡָ�� ����������
INT64 EffieientTLVDecoder::GetValue(UINT32 nType, UCHAR *Val[], UINT64 *pulLength, UINT64 ulBufCnt) const
{
    return GetValue(nType, (CHAR **)Val, pulLength, ulBufCnt);
}

// �²����ݽ���
INT64 EffieientTLVDecoder::GetValue(UINT32 nType, EffieientTLVDecoder *Val, UINT64 ulBufCnt) const
{
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
                                     "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
    std::multimap<INT32, EffieientTLV>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
    std::multimap<INT32, EffieientTLV>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
    ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, iter_beg != iter_end, ITGT_OBJ_NOT_FIND, "\n");

    if (iter_beg == iter_end) {
        ITGT_ERROR_PRINT(LOGCOMMON, "ErrCode Description:Obj Not Find nType:%0x", nType);
        for (std::multimap<INT32, EffieientTLV>::const_iterator iter = m_TLVMap.begin(); iter != m_TLVMap.end(); iter++) {
            ITGT_INFO_PRINT(LOGCOMMON, "m_TLVMap:%d", iter->first);
        }
        return ITGT_FAIL;
    }

    INT64 lResult = ITGT_FAIL;
    UINT32 i = 0;
    while (iter_beg != iter_end) {
        lResult = Val[i].Decode((UCHAR *)iter_beg->second.data,
                                iter_beg->second.length);
        ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, ITGT_SUCCESS == lResult, lResult, "\n");

        iter_beg++;
        i++;
    }

    return ITGT_SUCCESS;
}


