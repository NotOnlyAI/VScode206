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

enum ImageFormat {
    RGBA     = 0,
    RGB      = 1,
    BGR      = 2,
    GRAY     = 3,
    BGRA     = 4,
    YCrCb    = 5,
    YUV      = 6,
    HSV      = 7,
    XYZ      = 8,
    BGR555   = 9,
    BGR565   = 10,
    YUV_NV21 = 11,
    YUV_NV12 = 12,
    YUV_I420 = 13,
    HSV_FULL = 14,
};


typedef struct ImgData_T
{
	unsigned char* data;
	int width;             	
	int height;            	
	int stride; 
    int channel;
    int depth;             
	ImageFormat dataFormat;            
}STRU_ImgData_T,wsImg;




typedef struct Box
{
	int xmin;
	int ymin;
	int width;
	int height;
}STRU_Rect_T;


typedef struct Label
{
	int cls;
	float score;
}Label_s;


typedef struct DetectResult
{
	int nNum;
	Box boxes[MAXOBJECTCOUNT];
	Label labels[MAXOBJECTCOUNT];
}STRU_RectInfo_T;



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
	Point2f landmark[68];
}LandmarkInfo_T;




} //#namespace
#endif





