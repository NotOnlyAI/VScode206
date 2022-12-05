#include "nms.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <numeric>
using namespace std;




static float s_af32ExpCoef[10][16] = {
    {1.0f, 1.00024f, 1.00049f, 1.00073f, 1.00098f, 1.00122f, 1.00147f, 1.00171f, 1.00196f, 1.0022f, 1.00244f, 1.00269f, 1.00293f, 1.00318f, 1.00342f, 1.00367f},
    {1.0f, 1.00391f, 1.00784f, 1.01179f, 1.01575f, 1.01972f, 1.02371f, 1.02772f, 1.03174f, 1.03578f, 1.03984f, 1.04391f, 1.04799f, 1.05209f, 1.05621f, 1.06034f},
    {1.0f, 1.06449f, 1.13315f, 1.20623f, 1.28403f, 1.36684f, 1.45499f, 1.54883f, 1.64872f, 1.75505f, 1.86825f, 1.98874f, 2.117f, 2.25353f, 2.39888f, 2.55359f},
    {1.0f, 2.71828f, 7.38906f, 20.0855f, 54.5981f, 148.413f, 403.429f, 1096.63f, 2980.96f, 8103.08f, 22026.5f, 59874.1f, 162755.0f, 442413.0f, 1.2026e+006f, 3.26902e+006f},
    {1.0f, 8.88611e+006f, 7.8963e+013f, 7.01674e+020f, 6.23515e+027f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f},
    {1.0f, 0.999756f, 0.999512f, 0.999268f, 0.999024f, 0.99878f, 0.998536f, 0.998292f, 0.998049f, 0.997805f, 0.997562f, 0.997318f, 0.997075f, 0.996831f, 0.996588f, 0.996345f},
    {1.0f, 0.996101f, 0.992218f, 0.98835f, 0.984496f, 0.980658f, 0.976835f, 0.973027f, 0.969233f, 0.965455f, 0.961691f, 0.957941f, 0.954207f, 0.950487f, 0.946781f, 0.94309f},
    {1.0f, 0.939413f, 0.882497f, 0.829029f, 0.778801f, 0.731616f, 0.687289f, 0.645649f, 0.606531f, 0.569783f, 0.535261f, 0.502832f, 0.472367f, 0.443747f, 0.416862f, 0.391606f},
    {1.0f, 0.367879f, 0.135335f, 0.0497871f, 0.0183156f, 0.00673795f, 0.00247875f, 0.000911882f, 0.000335463f, 0.00012341f, 4.53999e-005f, 1.67017e-005f, 6.14421e-006f, 2.26033e-006f, 8.31529e-007f, 3.05902e-007f},
    {1.0f, 1.12535e-007f, 1.26642e-014f, 1.42516e-021f, 1.60381e-028f, 1.80485e-035f, 2.03048e-042f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};

static float SVP_NNIE_QuickExp( int s32Value )
{
    if( s32Value & 0x80000000 )
    {
        s32Value = ~s32Value + 0x00000001;
        return s_af32ExpCoef[5][s32Value & 0x0000000F] * s_af32ExpCoef[6][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[7][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[8][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[9][(s32Value>>16) & 0x0000000F ];
    }
    else
    {
        return s_af32ExpCoef[0][s32Value & 0x0000000F] * s_af32ExpCoef[1][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[2][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[3][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[4][(s32Value>>16) & 0x0000000F ];
    }
}



int SVP_NNIE_SoftMax( float* pf32Src, uint32_t u32Num)
{
    float f32Max = 0;
    float f32Sum = 0;
    uint32_t i = 0;

    for(i = 0; i < u32Num; ++i)
    {
        if(f32Max < pf32Src[i])
        {
            f32Max = pf32Src[i];
        }
    }

    for(i = 0; i < u32Num; ++i)
    {
        pf32Src[i] = (float)SVP_NNIE_QuickExp((int)((pf32Src[i] - f32Max))*4096);
        f32Sum += pf32Src[i];
    }

    for(i = 0; i < u32Num; ++i)
    {
        pf32Src[i] /= f32Sum;
    }
    return 0;
}


int SVP_NNIE_SoftMaxV2( float &pf32Src1,float &pf32Src2)
{
    float f32Max = 0;
    float f32Sum = 0;
    uint32_t i = 0;

    if(f32Max <  pf32Src1) f32Max =  pf32Src1;
    if(f32Max <  pf32Src2) f32Max =  pf32Src2;
    pf32Src1 = (float)SVP_NNIE_QuickExp((int)((pf32Src1 - f32Max))*4096);
    pf32Src2 = (float)SVP_NNIE_QuickExp((int)((pf32Src2 - f32Max))*4096);
    f32Sum =pf32Src1+pf32Src2;
    pf32Src1 /= f32Sum;
    pf32Src2 /= f32Sum;
    return 0;
}



bool sort_score(float* box1,float* box2)
{
    return (box1[4] > box2[4]);
}


float IOU(float* box1,float* box2)
{
    float x1 = std::max(box1[0],box2[0]);
    float y1 = std::max(box1[1],box2[1]);
    float x2 = std::min((box1[0] + box1[2]),(box2[0] + box2[2]));
    float y2 = std::min((box1[1] + box1[3]),(box2[1] + box2[3]));
    float over_area = (x2 - x1) * (y2 - y1);
    float iou = over_area/(box1[2] * box1[3] + box2[2] * box2[3]-over_area);
    return iou;
}


template <typename T>
vector<size_t> sort_indexes_e(vector<T> &v)
{
    vector<size_t> idx(v.size());
    iota(idx.begin(), idx.end(),0);
    sort(idx.begin(), idx.end(),[&v](size_t i1, size_t i2) {return v[i1][4] > v[i2][4]; });
    return idx;
}


vector<float*> nms(std::vector<float*> vec_boxs,float score_threshold,float nms_threshold,std::vector<int> &nms_indexes)
{

   vector<bool> del(vec_boxs.size(), false);
   for(size_t i = 0; i < vec_boxs.size(); i++)
   {

      if(vec_boxs.at(i)[4] < score_threshold)
      {
         del[i] = true;
        //  cout<<i<<" "<<vec_boxs.at(i)[4]<<endl;
      }
   }

    vector<float*> nms_boxs_iou;
    vector<int> score_indexes;
   
    for(size_t i=0;i<vec_boxs.size();i++)
    {
        if(!del[i])
        {
            nms_boxs_iou.push_back(vec_boxs[i]);
            score_indexes.push_back(i);
        }
    }
//    for(size_t i=0;i<nms_boxs_iou.size();i++)
//    {
//        cout<<i<<" "<<nms_boxs_iou.at(i)[0]<<endl;
//        cout<<i<<" "<<nms_boxs_iou.at(i)[1]<<endl;
//        cout<<i<<" "<<nms_boxs_iou.at(i)[2]<<endl;
//        cout<<i<<" "<<nms_boxs_iou.at(i)[3]<<endl;
//        cout<<i<<" "<<nms_boxs_iou.at(i)[4]<<endl;
//    }
    vec_boxs.clear();
    vector<float*>().swap(vec_boxs);

   vector<bool> del_iou(nms_boxs_iou.size(), false);
   vector<int> score_indexes_sort;
   vector<size_t> idx;
   idx = sort_indexes_e(nms_boxs_iou);//注意vec中的内容不变，不是返回排序后的向量
   vector<float*> nms_boxs_iou_sort;
   for (int i = 0; i < nms_boxs_iou.size(); i++)
   {
        nms_boxs_iou_sort.push_back(nms_boxs_iou[idx[i]]);
        score_indexes_sort.push_back(score_indexes[idx[i]]);
    }

 //  sort(nms_boxs_iou.begin(),nms_boxs_iou.end(),sort_score);

//     for(size_t i=0;i<nms_boxs_iou.size();i++)
//    {
    //    cout<<i<<" "<<nms_boxs_iou.at(i)[0]<<endl;
    //    cout<<i<<" "<<nms_boxs_iou.at(i)[1]<<endl;
    //    cout<<i<<" "<<nms_boxs_iou.at(i)[2]<<endl;
    //    cout<<i<<" "<<nms_boxs_iou.at(i)[3]<<endl;
//        cout<<i<<" "<<nms_boxs_iou.at(i)[4]<<endl;
//    }
   for(size_t i = 0; i < nms_boxs_iou_sort.size(); i++)
   {
      if(!del_iou[i])
      {
         for(size_t j = i+1; j < nms_boxs_iou_sort.size(); j++)
         {  
            // cout<<i<<" iou"<<IOU(nms_boxs_iou[i], nms_boxs_iou[j])<<endl;
            if(!del_iou[j] && IOU(nms_boxs_iou_sort[i], nms_boxs_iou_sort[j]) > nms_threshold)
            {
               del_iou[j] = true;//IOU大于阈值，扔掉
            }
        }
     }
   }
    // for(size_t i=0;i<nms_boxs_iou.size();i++)
    // {
    //    cout<<i<<" "<<del_iou[i]<<endl;
    // }



    vector<float*> nms_boxs;
    for(size_t i=0;i<nms_boxs_iou_sort.size();i++)
    {
        if(!del_iou[i])
        {
            nms_boxs.push_back(nms_boxs_iou_sort[i]);
            nms_indexes.push_back(score_indexes_sort[i]);
        }
    }

    nms_boxs_iou.clear();
    vector<float*>().swap(nms_boxs_iou);
//    for(size_t i=0;i<nms_boxs.size();i++)
//    {
//         cout<<i<<" ssfsdfsd"<<nms_boxs.at(i)[0]<<endl;
//         cout<<i<<" "<<nms_boxs.at(i)[0]<<endl;
//         cout<<i<<" "<<nms_boxs.at(i)[1]<<endl;
//         cout<<i<<" "<<nms_boxs.at(i)[2]<<endl;
//         cout<<i<<" "<<nms_boxs.at(i)[3]<<endl;
//         cout<<i<<" "<<nms_boxs.at(i)[4]<<endl;
//     }
    return nms_boxs;

}


