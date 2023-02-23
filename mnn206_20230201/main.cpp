#include <iostream>
#include <sys/unistd.h>             //�ṩPOSIX����ϵͳAPI����Unix�����йٷ��汾
#include <sys/socket.h>
#include <netinet/in.h>             //��Ҫ������һЩ����
#include <arpa/inet.h>              //��Ҫ�����˸�ʽת������
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#include <iostream>


using namespace std;




#include "models206.h"
#include "LaneDetect/LaneDetect.hpp"
#include <opencv2/opencv.hpp>


int main(int argc, char *argv[])
{
    
    LaneDetect mlane ;
    mlane.Init(0,0,1);

    
    

    while(1)
    {
        cv::Mat frame=cv::imread("lane1.jpg");
        std::vector<M2::lane_DECODE> final_lane;
        mlane.ForwardBGR(frame,final_lane);
        float sx = float(frame.cols) / float(512.0);
        float sy = float(frame.rows) / float(288.0);


        for (int i = 0; i < final_lane.size(); i++)
        {
            for (int j = 0; j < final_lane[i].Lane.size() - 1; j++) {
                float px = final_lane[i].Lane[j].x;
                float py = final_lane[i].Lane[j].y;
                cv::Point P;
                P.x = int(px * sx);
                P.y = int(py * sy);
                if (P.x > 0 && P.x < 1280) {
                    cv::circle(frame, P, 5, cv::Scalar(0, 0, 255), -1);
                }
            }
        }
        cv::imshow("11",frame);
        cv::waitKey(0);

    }

        



    return 0;
}
