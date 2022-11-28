#ifndef _BLAZE_FACE_NMS_H_
#define _BLAZE_FACE_NMS_H_

#include <stdint.h>
#include <vector>
using namespace std;

bool sort_score(float* box1,float* box2);
float IOU(float* box1,float* box2);
vector<float*> nms(std::vector<float*> vec_boxs,float score_threshold,float nms_threshold,std::vector<int> &nms_indexes);

#endif
