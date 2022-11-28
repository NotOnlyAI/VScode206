#include "nms.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <numeric>
using namespace std;

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


