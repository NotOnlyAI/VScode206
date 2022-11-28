#include <iostream>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <math.h>


#include "hi_results_show.h"



extern SAMPLE_SVP_NNIE_SSD_SOFTWARE_PARAM_S s_stSsdSoftwareParam;


using namespace std;
using namespace cv;


HI_S32 SAMPLE_SVP_NNIE_ShowResult(cv::Mat image,SDC_SSD_RESULT_S stResult)
{

    cv::Mat image_bbox = image;

    for(int i = 0; i < stResult.numOfObject; i++)
    {   
        cv::Mat image_bbox;
        image_bbox=image.clone();
        printf("\n");
        printf("  DetectFace:%d\n",i+1);
        printf("BBox(score=%f)(xmin,ymin,xmax,ymax): %f,%f,%f,%f \n",
            stResult.pObjInfo[i].f32Score,
            stResult.pObjInfo[i].f32Xmin,
            stResult.pObjInfo[i].f32Ymin,
            stResult.pObjInfo[i].f32Xmax,
            stResult.pObjInfo[i].f32Ymax);

        printf("Landmark:(x0,y0,x1,y1,x2,y2,x3,y3,x4,y4)\n");
        printf("x0:%f,y0:%f\n",stResult.pObjInfo[i].f32X0,stResult.pObjInfo[i].f32Y0);
        printf("x1:%f,y1:%f\n",stResult.pObjInfo[i].f32X1,stResult.pObjInfo[i].f32Y1);
        printf("x2:%f,y2:%f\n",stResult.pObjInfo[i].f32X2,stResult.pObjInfo[i].f32Y2);
        printf("x3:%f,y3:%f\n",stResult.pObjInfo[i].f32X3,stResult.pObjInfo[i].f32Y3);
        printf("x4:%f,y4:%f\n",stResult.pObjInfo[i].f32X4,stResult.pObjInfo[i].f32Y4);
        

        int x1=int(stResult.pObjInfo[i].f32Xmin);
        int y1=int(stResult.pObjInfo[i].f32Ymin);
        int w=int(stResult.pObjInfo[i].f32Xmax)-int(stResult.pObjInfo[i].f32Xmin);
        int h=int(stResult.pObjInfo[i].f32Ymax)-int(stResult.pObjInfo[i].f32Ymin);

        cv::Point origin;
	    origin.x = x1 + 20;
	    origin.y = y1 + 20;
        int font_face = cv::FONT_HERSHEY_COMPLEX;
        double font_scale = 1;
	    int thickness = 1;
        std::string text = std::to_string(stResult.pObjInfo[i].f32Score);
        cv::putText(image_bbox, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);

        cv::Rect r = cv::Rect(x1, y1, w, h);
        cv::rectangle(image_bbox, r, cv::Scalar(255, 0, 0), 1, 8, 0);
        cv::Point p1(stResult.pObjInfo[i].f32X0, stResult.pObjInfo[i].f32Y0);
	    cv::Point p2(stResult.pObjInfo[i].f32X1, stResult.pObjInfo[i].f32Y1);
	    cv::Point p3(stResult.pObjInfo[i].f32X2, stResult.pObjInfo[i].f32Y2);
	    cv::Point p4(stResult.pObjInfo[i].f32X3, stResult.pObjInfo[i].f32Y3);
	    cv::Point p5(stResult.pObjInfo[i].f32X4, stResult.pObjInfo[i].f32Y4);
	    cv::circle(image_bbox, p1, 1, cv::Scalar(0, 255, 0), -1); // 画半径为1的圆(画点�?
	    cv::circle(image_bbox, p2, 1, cv::Scalar(0, 255, 0), -1);  // 画半径为1的圆(画点�?
	    cv::circle(image_bbox, p3, 1, cv::Scalar(0, 255, 0), -1);  // 画半径为1的圆(画点�?
	    cv::circle(image_bbox, p4, 1, cv::Scalar(0, 255, 0), -1);  // 画半径为1的圆(画点�?
	    cv::circle(image_bbox, p5, 1, cv::Scalar(0, 255, 0), -1);  // 画半径为1的圆(画点�?
        std::string save_path = "detcect_face_"+std::to_string(i)+".jpg";
        cv::imwrite(save_path, image_bbox);
        printf("save_face_image in detcect_face \n");
        printf("\n");

    }
    
    return 0;
}