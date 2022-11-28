//
//  Detection.cpp
//  FaceTracking
//
//  Created by Menglong Yang on 2018/11/16.
//  Copyright © 2018 Menglong Yang. All rights reserved.
//

#include "Detection.hpp"


#ifndef Face_PI
#define Face_PI 3.14159265358979323846
#endif



using namespace std;
using namespace cv;


Detection::Detection(float &xmin,float &ymin,float &w,float &h,float &score,
                float &x0,float &y0,float &x1,float &y1,float &x2,float &y2,float &x3,float &y3,float &x4,float &y4,
                int frame_w,int frame_h,
                int face_size,int face_position,float pose_roll,float pose_yaw,float pose_pitch)
{
    m_xmin=xmin;
    m_ymin=ymin;
    m_width=w;
    m_height=h;
    m_score=score;

    m_x0=x0;
    m_y0=y0;
    m_x1=x1;
    m_y1=y1;
    m_x2=x2;
    m_y2=y2;
    m_x3=x3;
    m_y3=y3;
    m_x4=x4;
    m_y4=y4;

    m_frame_w=frame_w;
    m_frame_h=frame_h;

    m_face_size=face_size;
    m_face_position=face_position;
    m_pose_roll=pose_roll;
    m_pose_yaw=pose_yaw;
    m_pose_pitch=pose_pitch;

    m_features={0};
}


bool  Detection::good_quality()
{
    if(int(m_width)<m_face_size||int(m_height)<m_face_size){return false;}

    int dx1=int(m_xmin)-1;
    int dy1=int(m_ymin)-1;
    int dx2=m_frame_w-int(m_xmin+m_width);
    int dy2=m_frame_h-int(m_ymin+m_height);
    if(dx1<m_face_position||dx2<m_face_position||dy1<m_face_position||dy2<m_face_position){return false;}

    _STRU_PointF_T point0(1.0f*m_x0,1.0f*m_y0);
    _STRU_PointF_T point1(1.0f*m_x1,1.0f*m_y1);
    _STRU_PointF_T point2(1.0f*m_x2,1.0f*m_y2);
    _STRU_PointF_T point3(1.0f*m_x3,1.0f*m_y3);
    _STRU_PointF_T point4(1.0f*m_x4,1.0f*m_y4);


    _STRU_PointF_T point_center_eye = (point0 + point1) / 2.0;
    _STRU_PointF_T point_center_mouth = (point3 + point4) / 2.0;

    Line line_eye_mouth(point_center_eye, point_center_mouth);
    _STRU_PointF_T vector_left2right = point1 - point0;
    auto rad = atan2(vector_left2right.y, vector_left2right.x);
    // printf("rad=%f\n",rad);

    auto angle = rad * 180.0f / Face_PI;
    auto roll_dist = fabs(angle);

    if(roll_dist>m_pose_roll){printf("roll_dist=%f\n",roll_dist);return false;}


    auto raw_yaw_dist = line_eye_mouth.distance(point2);
    auto yaw_dist = raw_yaw_dist / (point0 ^ point1);
    // printf("yaw=%f\n",yaw_dist);

    yaw_dist=yaw_dist* 180.0f / Face_PI;
    if(yaw_dist>m_pose_yaw){printf("yaw_dist=%f\n",yaw_dist);return false;}

    static const float nose_center = 0.5f;
    auto point_suppose_projection = point_center_eye * nose_center + point_center_mouth * (1 - nose_center);
    auto point_projection = line_eye_mouth.projection(point2);
    auto raw_pitch_dist = point_projection ^ point_suppose_projection;
    auto pitch_dist = raw_pitch_dist / (point_center_eye ^ point_center_mouth);
    // printf("pitch=%f\n",pitch_dist);

    pitch_dist=pitch_dist* 180.0f / Face_PI;
    if(pitch_dist>m_pose_pitch){printf("pitch_dist=%f,\n",pitch_dist);return false;}


    printf("roll_dist=%f,yaw_dist=%f,pitch_dist=%f\n",roll_dist,yaw_dist,pitch_dist);


    return true;
}

Detection:: ~Detection()
{
}

void Detection::get_features()
{
}

