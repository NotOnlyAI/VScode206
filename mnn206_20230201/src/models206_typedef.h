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
#define MAXPTS		        500		//最大landmark�?


using namespace std;
namespace M2 {



struct DMSState{
  int eye_state;      // 0 open  ;      1 close ;
  int mouth_state;    // 0 close ;      1 open;
  int face_state;     // 0 good_face;   1 bad face or no face;
  int smoking_state;  // 0 no smoking;  1 smoking;
  int phone_state;    // 0 no phone;    1 phone;
};

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
  float z=0;
}Point2f_T;


typedef struct LandmarkInfo
{
	Point2f landmark[MAXPTS];
  int numPoints;
}LandmarkInfo_T;




} //#namespace
#endif






