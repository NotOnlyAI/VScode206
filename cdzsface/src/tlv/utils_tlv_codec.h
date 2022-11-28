/******************************************************************************

                  ��Ȩ���� (C), 2001-2016, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : utils_tlv_codec.h
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
#ifndef __UTILS_TLV_CODEC_H__
#define __UTILS_TLV_CODEC_H__

#include <algorithm>
#include <map>
#include <vector>
#include <typeindex>  // std::type_index

#include "securec.h"
#include "intelligent_alg_metadata.h"

#include "utils_err_code.h"
#include <type_traits>
#include "sdc.h"


// ��ȡ������Ҫ���Ƶ�λ��
#define METADTAT_TYPE_POS (24)

// ��������������
struct BinaryPutData {
    UCHAR *data;
    INT32 len;

    BinaryPutData(UCHAR *d = nullptr, INT32 l = 0)
    {
        data = d;
        len = l;
    }
};

class TLV_Encoder;
class TLV_Perf_Encoder;

// TLV������������
// �������Ͷ��壬������
// ������Ҫ�����Ƶ��ػ�����

template<UINT32 TAG, class T>
struct TlvCheckType {
private:
    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x02ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, CHAR> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x02)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x03ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, UCHAR> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x03)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x04ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, INT16> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x04)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x05ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, UINT16> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x05)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x06ö�������ػ����� //0x01ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, INT32> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x06) ||
    ((TAG >> METADTAT_TYPE_POS) == 0x01)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x07ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, UINT32> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x07)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x08ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, INT64> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x08)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x09ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, UINT64> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x09)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x0A/0x20/0x21ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, UCHAR *> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x0A) || 
    ((TAG >> METADTAT_TYPE_POS) == 0x20) || 
    ((TAG >> METADTAT_TYPE_POS) == 0x21)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x0Bö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, META_RECT_S> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x0B)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x0Cö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, META_POINT_S> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x0C)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x0Dö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, META_LINE_S> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x0D)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x0Eö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, META_POLYGON_S> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x0E)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x0Fö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, META_COLOR_S> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x0F)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x10ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, HUMAN_ATTRIBUTES> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x10)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x11ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, FACE_ATTRIBUTES> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x11)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x12ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, FACE_INFO_S> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x12)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// 0x13ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, RIDERMAN_ATTRIBUTES> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x13)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

/*
// 0x22ö�������ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, UINT128> {
    typename std::enable_if<
    ((TAG >> METADTAT_TYPE_POS) == 0x22)
    , INT32>::type
    GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};
*/

// TLV_Encoder�����ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, TLV_Encoder *> {
    typename std::enable_if<
    (TAG == (UINT32)COMMON) ||
    (TAG == (UINT32)TARGET) ||
    (TAG == (UINT32)RULE) ||
    (TAG == (UINT32)TALARM) ||
    (TAG == (UINT32)TRECORD) ||
    (TAG == (UINT32)TRAFFIC_LIGHT) ||
    (TAG == (UINT32)METADATA_TYPE)
    , INT32>::type 
	GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// TLV_Perf_Encoder�����ػ�����
template<UINT32 TAG>
struct TlvCheckType<TAG, TLV_Perf_Encoder *> {
    typename std::enable_if<
    (TAG == (UINT32)COMMON) ||
    (TAG == (UINT32)TARGET) ||
    (TAG == (UINT32)RULE) ||
    (TAG == (UINT32)TALARM) ||
    (TAG == (UINT32)TRECORD) ||
    (TAG == (UINT32)TRAFFIC_LIGHT) ||
    (TAG == (UINT32)METADATA_TYPE)
    , INT32>::type 
	GetTag()
    {
        return TAG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// HOTMAP_ACCUM_IMG��ǰ�������
// ��ǰ���ܼ���
template<>
struct TlvCheckType<HOTMAP_ACCUM_IMG, UCHAR *> {
    INT32 GetTag()
    {
        return HOTMAP_ACCUM_IMG;
    }

    TlvCheckType(){}
    ~TlvCheckType(){}
};

// TLV����
class TLV {
public:
    ~TLV();  // ���麯�� �������̳�

    // ���캯���б�
    template<class T> 
	TLV(UINT32 nType, T Val);

    // �ַ�������
    TLV(UINT32 nType, const CHAR *Val);

    // ������buf����
    TLV(UINT32 nType, const UCHAR *Val, UINT32 uLength);

    // �ֶζ���������buf
    TLV(UINT32 nType, const std::vector<BinaryPutData> &putdata);

    friend class TLV_Codec;
    friend class TLV_Encoder;
    friend class TLV_Decoder;

private:
    /*****************************************************************************
     �� �� ��  : Init
     ��������  : ��ʼ��Len��Val�ֶ�
     �������  : const void * pVal,UINT32 uLength
     �������  : void
     �� �� ֵ  : void
     ���ú���  : 
     ��������  : 
     
     �޸���ʷ      :
      1.��    ��   : 2016��08��05�� 09:49:31
        ��    ��   : �� �S/chenyun c00193875
        �޸�����   : �����ɺ���
    
    *****************************************************************************/
    void Init(const void *pVal, UINT32 uLength);

    // ��ֹ��ֵ����
    TLV &operator=(const TLV &Val);
    TLV(const TLV &);

private:
    INT32 m_nType;      // ����
    UINT32 m_uLength;   // ����
    UCHAR *m_pucValue;  // ֵ
};

// TLV �����������
class TLV_Codec {
public:
    TLV_Codec() : m_pucTlvBuf(NULL), m_ulDataLen(0)
        {
    }
    virtual ~TLV_Codec();

    /*****************************************************************************
     �� �� ��  : Clear
     ��������  : ���TLV�������
     �������  : 
     �������  : void
     �� �� ֵ  : void
     ���ú���  : 
     ��������  : 
     
     �޸���ʷ      :
      1.��    ��   : 2016��08��05�� 11:28:36
        ��    ��   : �� �S/chenyun c00193875
        �޸�����   : �����ɺ���
    
    *****************************************************************************/
    void Clear(void);

    // ��ȡ����
    const UCHAR *GetTlvBuf(void)const
    {
        return m_pucTlvBuf;
    }

    UINT64 GetTlvDataLen(void)const
    {
        return m_ulDataLen;
    }

protected:
    /*****************************************************************************
     �� �� ��  : InsertValueToMap
     ��������  : ��TLVָ�����Map
     �������  : TLV * pTlvObj
     �������  : void
     �� �� ֵ  : INT64
     ���ú���  : 
     ��������  : 
     
     �޸���ʷ      :
      1.��    ��   : 2016��08��05�� 11:10:36
        ��    ��   : �� �S/chenyun c00193875
        �޸�����   : �����ɺ���
    
    *****************************************************************************/
    INT64 InsertValueToMap(TLV *pTlvObj);

protected:
    // std::map<INT32,TLV*> m_TLVMap; // type��TLV����ӳ���
    std::multimap<INT32, TLV *> m_TLVMap;  // type��TLV����ӳ���
    UCHAR *m_pucTlvBuf;                    // TLV����buf
    UINT64 m_ulDataLen;                    // TLV���ݳ���
private:
    TLV_Codec(const TLV_Codec &codec);             // ���������ƹ���
    TLV_Codec &operator=(const TLV_Codec &codec);  // ��������ֵ����
};

// TLV ������
class TLV_Encoder : public TLV_Codec {
public:
    TLV_Encoder(){}
    virtual ~TLV_Encoder(){}

    // �����������������
    template<UINT32 TAG>
    INT64 PutValue(const std::vector<BinaryPutData> &putdata)
    {
        // ITGT_RETURN_VAL_IF_FAIL(LOGCOMMON, putdata.size() != 0, ITGT_FAIL, "putdata is empty for tag %x", TAG);
        // ���ͼ��
        TlvCheckType<TAG, UCHAR *> _;

        return PutValue(TAG, putdata);
    }

    // ���������ݷ���
    template<UINT32 TAG>
    INT64 PutValue(UCHAR *val, INT32 len)
    {
        // ���ͼ��
        TlvCheckType<TAG, UCHAR *> _;

        return PutValue(TAG, val, len);
    }

    // ���������������
    template<UINT32 TAG, class T>
    typename std::enable_if<
    // ��֧��ָ������
    (!std::is_pointer<T>::value)
    , INT64>::type 
	PutValue(const T &val)
    {
        // ���ͼ��
        TlvCheckType<TAG, T> _;

        return PutValue (TAG, (const UCHAR *)(&val), sizeof(T));
    }

    // ����TLV����
    template<UINT32 TAG>
    INT64 PutValue(const TLV_Encoder &Val)
    {
        // ���ͼ��
        TlvCheckType<TAG, TLV_Encoder *> _;

        return PutValue (TAG, (const UCHAR *)(Val.GetTlvBuf()), Val.GetTlvDataLen());
    }

    /*****************************************************************************
     �� �� ��  : Encode
     ��������  : ���������ݽ���TLV����
     �������  : void
     �������  : void
     �� �� ֵ  : INT64
     ���ú���  : 
     ��������  : 
     
     �޸���ʷ      :
      1.��    ��   : 2016��08��05�� 10:55:14
        ��    ��   : �� �S/chenyun c00193875
        �޸�����   : �����ɺ���
    
    *****************************************************************************/
    INT64 Encode(void);

    INT64 Encode(UCHAR* pucDataBuf, UINT64 ulDataLen);
    
private:
    // ���ݷ���
    INT64 PutValue(UINT32 nType, const UCHAR *data, INT32 len);

    // ���ݷ���
    INT64 PutValue(UINT32 nType, const std::vector<BinaryPutData> &putdata);

private:
    TLV_Encoder(const TLV_Encoder &Enc);             // ���������ƹ���
    TLV_Encoder &operator=(const TLV_Encoder &Enc);  // ��������ֵ����
};

// TLV������perf��
class TLV_Perf_Encoder : public TLV_Encoder {
public:
    TLV_Perf_Encoder(){}
    ~TLV_Perf_Encoder();

    // ����TLV����
    template<UINT32 TAG>
    INT64 PutValue(TLV_Encoder *Val)
    {
        // ���ͼ��
        TlvCheckType<TAG, TLV_Encoder *> _;

        return PutValue(TAG, Val);
    }

    // ����TLV����
    template<UINT32 TAG>
    INT64 PutValue(TLV_Perf_Encoder *Val)
    {
        // ���ͼ��
        TlvCheckType<TAG, TLV_Perf_Encoder *> _;

        return PutValue(TAG, Val);
    }

    // ���������ݽ���TLV����
    INT64 Encode(void);

    INT64 Encode(UCHAR* pucDataBuf, UINT64 ulDataLen);

    // �����ͷ�
    void Clear(void);
    
protected:        
    // ���ݷ���
    INT64 PutValue(UINT32 nType, TLV_Encoder *pTlvEncoder);

    // ���ݷ���
    INT64 PutValue(UINT32 nType, TLV_Perf_Encoder *pTlvEncoder);

    // ��TLV_Encoderָ�����Map
    INT64 InsertValueToMap(UINT32 nType, TLV_Encoder *pTlvEncoder);

    // ��TLV_Perf_Encoderָ�����Map
    INT64 InsertValueToMap(UINT32 nType, TLV_Perf_Encoder *pTlvEncoder);

private:
    std::multimap<INT32, TLV_Encoder *> m_TLVMap;  // type��TLV_Encoder����ӳ���
    std::multimap<INT32, TLV_Perf_Encoder *> m_PerfTLVMap;  // type��TLV_Perf_Encoder����ӳ���
};

// TLV ������
class TLV_Decoder : public TLV_Codec {
public:
    TLV_Decoder(){}
    ~TLV_Decoder(){}

    // ��ȡ���͸���
    UINT64 GetValueCount(UINT32 nType) const;

    UINT64 GetCount() const;

    // ��ȡֵ���� ��Ҫ���ڶ�����������
    INT64 GetValueLen(UINT32 nType, UINT32 *puLength, UINT64 ulBufCnt) const;

    // ��ȡ������ֵ
    INT64 GetValue(UINT32 nType, bool *Val, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, CHAR *Val, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, INT16 *Val, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, INT32 *Val, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, INT64 *Val, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, UCHAR *Val, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, UINT16 *Val, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, UINT32 *Val, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, UINT64 *Val, UINT64 ulBufCnt) const;

    INT64 GetValue(UINT32 nType, CHAR *Val[], UINT64 *pulLength, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, UCHAR *Val[], UINT64 *pulLength, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, TLV_Decoder *Val, UINT64 ulBufCnt) const;

    /*****************************************************************************
     �� �� ��  : Decode
     ��������  : TLV����
     �������  : UCHAR * pucTlvBuf��ULONG ulDataLen
     �������  : void
     �� �� ֵ  : INT64
     ���ú���  : 
     ��������  : 
     
     �޸���ʷ      :
      1.��    ��   : 2016��08��05�� 14:24:45
        ��    ��   : �� �S/chenyun c00193875
        �޸�����   : �����ɺ���
    
    *****************************************************************************/
    INT64 Decode(UCHAR *pucTlvBuf, UINT64 ulDataLen);

private:
    TLV_Decoder(const TLV_Decoder &Enc);             // ���������ƹ���
    TLV_Decoder &operator=(const TLV_Decoder &Enc);  // ��������ֵ����
};

// EffieientTLV ���� ���������� ֻ����ָ������
struct EffieientTLV {
    CHAR *data;
    UINT32 length;
    INT32 type;

    EffieientTLV(CHAR *tmpData = NULL, INT32 tmpLen = 0, INT32 tmpType = 0) :
        data(tmpData), length(tmpLen), type(tmpType){}

    ~EffieientTLV(){}
};

// TLV ���ٽ����� ��ȡ���ڴ�����ʱռ��ԭ������buf�ڴ� ����������
// ʹ��ʱԭ������buffer��Ҫһֱ���� ���� ��������ƿ����ʹ��
class EffieientTLVDecoder {
public:
    EffieientTLVDecoder(){}
    ~EffieientTLVDecoder(){}

    // ���ݽ���
    INT64 Decode(UCHAR *pucTlvBuf, UINT64 ulDataLen);

    // ��ȡ���͸���
    UINT64 GetValueCount(UINT32 nType) const;
    UINT64 GetCount() const;

    // ��ȡֵ���� ��Ҫ���ڶ����������ͣ�������ǰд����������ʹ��
    INT64 GetValueLen(UINT32 nType, UINT32 *puLength, UINT64 ulBufCnt) const;

    /**********************************************************
     * ��ȡ��ֵ���ݣ���ֵ������Ҫ�������ݿ���
     * Ӧ�þ�������Ҫ��������A��A������INT/�ṹ�壬��type��Ӧ����������
     * 1����ȡ��Ŀ count = GetValueCount(type);
     * 2���������� A val[count] = {0};
     * 3��������ֵ GetValue(type, val, count)
    ***********************************************************/
    template<typename T>
    INT64 GetValue(UINT32 nType, T *Val, UINT64 ulBufCnt) const
    {
        // ITGT_RETURN_VAL_SHOW_ERR_IF_FAIL(LOGCOMMON, m_TLVMap.count(nType) <= ulBufCnt, ITGT_BUF_TOO_SMALL,
        //                                  "Type Cnt %u BufCnt %llu\n", (UINT32)m_TLVMap.count(nType), ulBufCnt);
        std::multimap<INT32, EffieientTLV>::const_iterator iter_beg = m_TLVMap.lower_bound(nType);
        std::multimap<INT32, EffieientTLV>::const_iterator iter_end = m_TLVMap.upper_bound(nType);
        if (iter_beg == iter_end) {
            return ITGT_OBJ_NOT_FIND;
        }

        UINT32 i = 0;
        while (iter_beg != iter_end) {
            // ITGT_RETURN_VAL_IF_FAIL(LOGCOMMON, iter_beg->second.length >= sizeof(T), ITGT_FAIL, "T is too large for type %d",
            //                         nType);
            Val[i] = (*(T *)(iter_beg->second.data));
            iter_beg++;
            i++;
        }

        return ITGT_SUCCESS;
    }

    /*****************************************************************************
     * ��ȡ���������ݣ�typeΪ0x0AXXXXXX��ʽ�����������ݽϴ󣬲���������
     * Ӧ�þ�������Ҫ�������������ݣ���ӦTagΪtype
     * 1����ȡ��Ŀ count = GetValueCount(type);
     * 2���������� UCHAR* val[count] = {nullptr}; UINT64 len[count] = {0};
     * 3��������ֵ GetValue(type, val, len, count)
     * ��Ҫע�����val������ֻ����ָ�룬ָ��ָ����ڴ治�ǵ�������������ǣ���ԭ��TLV buffer
     * ���ݵ��ڴ棬ʹ��ʱ��Ҫ��֤ԭ��buffer���ݲ����ͷ�
     * ***************************************************************************/
    INT64 GetValue(UINT32 nType, CHAR *Val[], UINT64 *pulLength, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, UCHAR *Val[], UINT64 *pulLength, UINT64 ulBufCnt) const;
    INT64 GetValue(UINT32 nType, EffieientTLVDecoder *Val, UINT64 ulBufCnt) const;

private:
    std::multimap<INT32, EffieientTLV> m_TLVMap;
};

#endif /* __UTILS_TLV_CODEC_H__ */
