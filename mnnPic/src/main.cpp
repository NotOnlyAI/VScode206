#include <iostream>
#include <sys/unistd.h>             //提供POSIX操作系统API，如Unix的所有官方版本
#include <sys/socket.h>
#include <netinet/in.h>             //主要定义了一些类型
#include <arpa/inet.h>              //主要定义了格式转换函数
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
#include <opencv2/opencv.hpp>
#include <dlfcn.h>

#include "MNN/Interpreter.hpp"
#include "MNN/MNNDefine.h"
#include "MNN/Tensor.hpp"
#include "MNN/ImageProcess.hpp"
#include "PicRecognize/PicRecognize.hpp"
using namespace std;



int main(int argc, char *argv[])
{
    
    auto handle = dlopen("libMNN_CL.so", RTLD_NOW);
    cv::Mat img=cv::imread("1.jpg");
    // cv::imshow("11",img);
    // cv::waitKey(0);

    std::shared_ptr<PicRecognize> PicRecognizeModel=std::make_shared<PicRecognize>();
    PicRecognizeModel->Forward(img);
    while(1)
    {
        std::cout<<"hello world!6"<<std::endl;
        sleep(10);
    }


    return 0;
}
