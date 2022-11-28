#include "quality.h"
#include <algorithm>
#include <math.h>
#ifndef Face_PI
#define Face_PI 3.14159265358979323846
#endif


// int evaluate_facesize(int w,int h,int &size);
// {   
    
//     size=std::min(w,h);
//     return 0;
// }



int evaluate_position(int x1,int y1,int x2,int y2,int origin_image_width, int origin_image_height,int &border)
{
	int dx1=x1-1.0;
    int dy1=y1-1.0;
    int dx2=origin_image_width-x2;
    int dy2=origin_image_height-y2;
	int tem1=std::min(dx1,dy1);
	int tem2=std::min(tem1,dx2);
    border=std::min(tem2,dy2);
    return 0;
}

int evaluate_pose(int x0,int y0,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4, float &roll, float &yaw, float &pitch)
{

    // static const float nose_center = 0.5f;
    // static const float roll0 = 1 / 6.0f;
    // static const float yaw0 = 0.2f;
    // static const float pitch0 = 0.2f;
    _STRU_PointF_T point0(1.0f*x0,1.0f*y0);
    _STRU_PointF_T point1(1.0f*x1,1.0f*y1);
    _STRU_PointF_T point2(1.0f*x2,1.0f*y2);
    _STRU_PointF_T point3(1.0f*x3,1.0f*y3);
    _STRU_PointF_T point4(1.0f*x4,1.0f*y4);



    _STRU_PointF_T point_center_eye = (point0 + point1) / 2.0;
    _STRU_PointF_T point_center_mouth = (point3 + point4) / 2.0;
//     // std::cout<<" x0: "<<point0.x<<"  y0: "<<point0.y<<endl;
//     // std::cout<<" x1: "<<point1.x<<"  y1: "<<point1.y<<endl;
//     // std::cout<<" xe: "<<point_center_eye.x<<"  ye: "<<point_center_eye.y<<endl;



    Line line_eye_mouth(point_center_eye, point_center_mouth);

    _STRU_PointF_T vector_left2right = point1 - point0;
    auto rad = atan2(vector_left2right.y, vector_left2right.x);
    auto angle = rad * 180 / Face_PI;
    auto roll_dist = fabs(angle) / 180;



//     auto raw_yaw_dist = line_eye_mouth.distance(point2);
//     auto yaw_dist = raw_yaw_dist / (point0 ^ point1);

//     auto point_suppose_projection = point_center_eye * nose_center + point_center_mouth * (1 - nose_center);
//     auto point_projection = line_eye_mouth.projection(point2);
//     auto raw_pitch_dist = point_projection ^ point_suppose_projection;
//     auto pitch_dist = raw_pitch_dist / (point_center_eye ^ point_center_mouth);

//     // std::cout<<" raw_yaw_dist: "<<raw_yaw_dist<<std::endl;
//     // std::cout<<" point0.x: "<<point0.x<<std::endl;
//     // std::cout<<" point0.y: "<<point0.y<<std::endl;
//     // std::cout<<" point1.x: "<<point1.x<<std::endl;
//     // std::cout<<" point1.y: "<<point1.y<<std::endl;
//     // std::cout<<" point2.x: "<<point2.x<<std::endl;
//     // std::cout<<" point2.y: "<<point2.y<<std::endl;
//     // std::cout<<" point3.x: "<<point3.x<<std::endl;
//     // std::cout<<" point3.y: "<<point3.y<<std::endl;
//     // std::cout<<" point4.x: "<<point4.y<<std::endl;
//     // std::cout<<" point4.y: "<<point4.y<<std::endl;
//     // std::cout<<" dx: "<<point0.x-point1.x<<std::endl;
//     // std::cout<<" dy: "<<point0.y-point1.y<<std::endl;
//     // std::cout<<" dist: "<<sqrt((point0.x-point1.x)*(point0.x-point1.x)+(point0.y-point1.y)*(point0.y-point1.y))<<std::endl;

//     roll = roll_dist;
//     yaw = yaw_dist;
//     pitch = pitch_dist;
    return 0;
}


int face_quality(sdc_yuv_frame_s &yuv_frame,const SDC_SSD_OBJECT_INFO_S &object,
                int &size, 
                int &border,
                float &roll,
                float &yaw,
                float &pitch)
{
    int xmin,ymin,xmax,ymax,w,h;
    int x0,y0,x1,y1,x2,y2,x3,y3,x4,y4;
    xmin=int(object.f32Xmin*1.0f*yuv_frame.width);
    ymin=int(object.f32Ymin*1.0f*yuv_frame.height);
    xmax=int(object.f32Xmax*1.0f*yuv_frame.width);
    ymax=int(object.f32Ymax*1.0f*yuv_frame.height);
    w=xmax-xmin;
    h=ymax-ymin;
    size=std::min(w,h);
    evaluate_position(x1,y1,x2,y2,yuv_frame.width,yuv_frame.height,border);
    // evaluate_pose(

    // )
    return 0;

}

