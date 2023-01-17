#ifndef _FD_FACE_TYPEDEF_H_
#define _FD_FACE_TYPEDEF_H_


#include <stdint.h>
#include <vector>
#include <cmath>
#include <cfloat>
#include <opencv2/opencv.hpp>


#define TFL_CAPI_EXPORT __attribute__((visibility("default")))

#define MAXOBJECTCOUNT		200		//����ͼ�����֧�ּ��������
#define MAXLANDMARKNUM		5		//最大landmark�?
#define MAXPTS		        98		//最大landmark�?


using namespace std;
namespace M2 {



struct Object {
  cv::Rect_<float> rect;
  int label;
  float prob;
};


struct ObjectInfo{
	int ObjectNum;
	Object objects[MAXOBJECTCOUNT];
};




struct lane_DECODE
{
    float prob;
    float start_pos;
    float end_pos;
    float ax;
    float ay;
    std::vector<cv::Point> Lane;
    int LeftRightType; 
    int dis;
};



typedef struct Point2f
{
	float x;
	float y;
}Point2f_T;


typedef struct LandmarkInfo
{
	Point2f landmark[98];
    int numPoints;
}LandmarkInfo_T;




} //#namespace
#endif






