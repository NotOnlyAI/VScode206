//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "FaceDetect.hpp"
#include <opencv2/opencv.hpp>
#include "M2utils/nms.h"
#include "M2utils/process.h"
using namespace std;
using namespace M2;


STRU_RectInfo_T GetMaxFace(STRU_RectInfo_T rectinfo,int &rect_index)//获取面积最大的人脸
{
	STRU_RectInfo_T temp;
	int maxarea = 0;
	for (int i = 0;i < rectinfo.nNum;i++)
	{
		int currarea = rectinfo.boxes[i].width*rectinfo.boxes[i].height;
		if (maxarea < currarea)
		{
			maxarea = currarea;
			temp.nNum = 1;
			temp.boxes[0] = rectinfo.boxes[i];
			temp.labels[0] = rectinfo.labels[i];
            rect_index=i;
		}
	}
	return temp;
}

STRU_RectInfo_T GetMidFace(STRU_RectInfo_T rectinfo,int img_w, int img_h,int &rect_index)//获取居中的人脸区域
{
	STRU_RectInfo_T temp;
	int midcoordinate_x = img_w / 2;
	int midcoordinate_y = img_h / 2;
	int mindis = img_w*img_h;
	for (int i = 0;i < rectinfo.nNum;i++)
	{
		int currx = rectinfo.boxes[i].xmin + rectinfo.boxes[i].width / 2;
		int curry = rectinfo.boxes[i].ymin + rectinfo.boxes[i].height / 2;
		int disx = midcoordinate_x - currx;
		int disy = midcoordinate_y - curry;
		int currdis = disx*disx + disy*disy;
		if (currdis < mindis)
		{
			mindis = currdis;
			temp.nNum = 1;
		    temp.boxes[0] = rectinfo.boxes[i];
			temp.labels[0] = rectinfo.labels[i];
            rect_index=i;
		}
	}
	return temp;
}



FaceDetect::FaceDetect() {


}


int FaceDetect::init(int deviceTpye,int print_config,int modelType)
{

    m_print=print_config;
    m_modelType=modelType;
    string mnnPath;


    mnnPath="./models206/FaceDetect.mnn" ;
    dimType = MNN::Tensor::TENSORFLOW;
    in_h=320;
    in_w=240;
    input_blob_names={"x"};
    output_blob_names={ "Identity","Identity_1","Identity_2","Identity_3","Identity_4","Identity_5","Identity_6","Identity_7",};
    float mean[3]     = {104.0f, 117.0f, 123.0f};
    float normals[3] = {1.0f, 1.0f, 1.0f};
    ::memcpy(imconfig.mean, mean, sizeof(mean));
    ::memcpy(imconfig.normal, normals, sizeof(normals));
    imconfig.sourceFormat = MNN::CV::BGR;
    imconfig.destFormat = MNN::CV::BGR;
    imconfig.filterType = MNN::CV::NEAREST;
    pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));

    if (modelType==0){ 
        mnnPath="./models206/FaceDetect.mnn" ;
        dimType = MNN::Tensor::TENSORFLOW;
        in_h=320;
        in_w=240;
        input_blob_names={"x"};
        output_blob_names={ "Identity","Identity_1","Identity_2","Identity_3","Identity_4","Identity_5","Identity_6","Identity_7",};
        float mean[3]     = {104.0f, 117.0f, 123.0f};
        float normals[3] = {1.0f, 1.0f, 1.0f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::BGR;
        imconfig.filterType = MNN::CV::NEAREST;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    }


    net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnnPath.c_str()));
    MNN::ScheduleConfig config;
    config.type=(MNNForwardType)(deviceTpye);
    config.mode = MNN_GPU_TUNING_NORMAL | MNN_GPU_MEMORY_IMAGE;
    MNN::BackendConfig backendConfig;
    backendConfig.precision = MNN::BackendConfig::Precision_Normal;
    backendConfig.memory    = MNN::BackendConfig::Memory_Normal;
    backendConfig.power     = MNN::BackendConfig::Power_Normal;
    config.backendConfig = &backendConfig;
    session = net->createSession(config);
    MNN_PRINT("Interpreter build, model_path: %s, dimType:%d\n",mnnPath.c_str(),dimType);

  
    inputTensors.resize(input_blob_names.size());
    inputTensors_host.resize(input_blob_names.size());
    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors[i] = net->getSessionInput(session,input_blob_names[0].c_str());
	}
    for (int i = 0; i < input_blob_names.size(); i++) {
		inputTensors_host[i] = new MNN::Tensor(inputTensors[i], dimType);
	}
 
    outputTensors.resize(output_blob_names.size());
    outputTensors_host.resize(output_blob_names.size());
    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors[i] = net->getSessionOutput(session, output_blob_names[i].c_str());
	}
    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors_host[i] = new MNN::Tensor(outputTensors[i], dimType);
	}

    float memoryUsage = 0.0f;
    net->getSessionInfo(session, MNN::Interpreter::MEMORY, &memoryUsage);
    float flops = 0.0f;
    net->getSessionInfo(session, MNN::Interpreter::FLOPS, &flops);
    int backendType[2];
    net->getSessionInfo(session, MNN::Interpreter::BACKENDS, backendType);
    MNN_PRINT("Session Info: memory use %f MB, flops is %f M, backendType is %d, batch size = %d\n", memoryUsage, flops, backendType[0], 1);


    for (int i = 0; i < input_blob_names.size(); i++) {
        MNN_PRINT("%s\n",input_blob_names[i].c_str());
		inputTensors_host[i]->printShape();
	}


    for (int i = 0; i < output_blob_names.size(); i++) {
        MNN_PRINT("%s\n",output_blob_names[i].c_str());
		outputTensors_host[i]->printShape();
	} 

    return 0;
}


int FaceDetect::ForwardBGR(const cv::Mat &image,M2::ObjectInfo &objectinfo,int max_or_mid) {

    auto start = chrono::steady_clock::now();


    image_h = image.rows;
    image_w = image.cols;



    cv::Mat resize_img;
    cv::resize(image,resize_img,cv::Size(in_w,in_h));
    // cv::imshow("resize",resize_img);
    // cv::waitKey(0);


    pretreat->convert((uint8_t *)resize_img.data, in_w,in_h,0,inputTensors_host[0]);
    inputTensors[0]->copyFromHostTensor(inputTensors_host[0]);
    net->runSession(session);
    for (int i = 0; i < output_blob_names.size(); i++) {
		outputTensors[i]->copyToHostTensor(outputTensors_host[i]);
	}
    if(m_print>=1){
        chrono::duration<double> elapsed2 = chrono::steady_clock::now() - start;
        cout << "net time:" << elapsed2.count() << " s" << endl;
        outputTensors_host[0]->printShape();
        for (int i = 0; i < 20; ++i) {
            MNN_PRINT("copy %f\n", outputTensors_host[0]->host<float>()[i]);
        }
    }


    decode(outputTensors_host);

    




    if(m_rectinfo.nNum>1 && (max_or_mid==0 || max_or_mid==1))
    {
        int index;
        STRU_RectInfo_T temp_rectinfo;
        if (max_or_mid==0)
        {
            temp_rectinfo=GetMaxFace( m_rectinfo,index);
        }else if (max_or_mid==1)
        {
            temp_rectinfo=GetMidFace( m_rectinfo,image_w,image_h,index);
        }else
        {
            temp_rectinfo=GetMaxFace( m_rectinfo,index);
        }
        // STRU_LandmarkInfo_T temp_landmarkinfo;
        // temp_landmarkinfo.nFaceNum=1;
        // temp_landmarkinfo.landmark[0]=landmarkinfo.landmark[index];
        m_rectinfo=temp_rectinfo;
        // landmarkinfo=temp_landmarkinfo;
    }
    
    objectinfo.ObjectNum=m_rectinfo.nNum;
    for (int j = 0; j < m_rectinfo.nNum; j++)
    {
        objectinfo.objects[j].rect.x = m_rectinfo.boxes[j].xmin;
        objectinfo.objects[j].rect.y = m_rectinfo.boxes[j].ymin;
        objectinfo.objects[j].rect.width = m_rectinfo.boxes[j].width;
        objectinfo.objects[j].rect.height = m_rectinfo.boxes[j].height;
        objectinfo.objects[j].label = m_rectinfo.labels[j].cls;
        objectinfo.objects[j].prob = m_rectinfo.labels[j].score;
    }

    if(m_print>=1){
        chrono::duration<double> elapsed3 = chrono::steady_clock::now() - start;
        cout << "FaceDetect Decode time:" << elapsed3.count() << " s" << endl;
    }

    if(m_print>=2){
        for (int j = 0; j <objectinfo.ObjectNum; j++)
        {
            cout <<j << ": " << objectinfo.objects[j].rect.x << " " << objectinfo.objects[j].rect.y  << " "<<objectinfo.objects[j].rect.width << " "<< objectinfo.objects[j].rect.height << " "<<objectinfo.objects[j].prob<< endl;
        }
    }

    return 0;
}




int FaceDetect::decode(std::vector< MNN::Tensor*> &outputTensors_host)
{
    // for (int i = 0; i < 10; ++i) {
    //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
    // }


    float box[3000][5];
    float landmark[3000][10];

    std::vector<int> indexes = {};
    std::vector<float*> vec_boxs = {};
    std::vector<float*> vec_landmarks = {};
    std::vector<int> nms_indexes = {};
    std::vector<float*> nms_boxs={};


    // for (int i = 0; i < output_blob_names.size(); i++) {
    //     // MNN_PRINT("%s\n",output_blob_names[i]);
	// 	outputTensors_host[i]->printShape();
	// }

    int start=0;
    int end=4;
    int grid_hs[4];
    int grid_ws[4];
    int BboxNumEachGrid[4]={3,2,2,3};
    float BoxSteps[4]={8,16,32,64};
    float BoxMinSizes[4][3] = {{10, 16, 24}, {32, 48,0}, {64, 96,0}, {128, 192, 256}};
    grid_hs[0]=40;grid_hs[1]=20;grid_hs[2]=10;grid_hs[3]=5;
    grid_ws[0]=30;grid_ws[1]=15;grid_ws[2]=8;grid_ws[3]=4;



    float f32Score0;
    float f32Score1;
    float f32Score;
    float f32X;
    float f32Y;
    float f32Xmin;
    float f32Ymin;
    float f32Xmax;
    float f32Ymax;
    float f32Width;
    float f32Height;
    int N=0;
    int BboxNum=0;
    for(int i = start; i < end; i++)
    {
        uint32_t u32Offset=0;
        uint32_t u32OffsetScore=0;


        int  grid_h = grid_hs[i];
        int  grid_w = grid_ws[i];
        int  grid_c_bbox =4;

        for(int j = 0; j < grid_h*grid_w; j++)
        {


            for(int k = 0; k < BboxNumEachGrid[i]; k++)
            {


               
                u32OffsetScore = (j * BboxNumEachGrid[i] + k) * 2;
                SVP_NNIE_SoftMax(&(outputTensors_host[i]->host<float>()[u32OffsetScore]), 2);
                f32Score0=outputTensors_host[i]->host<float>()[u32OffsetScore];
                f32Score1=outputTensors_host[i]->host<float>()[u32OffsetScore+1];
                
                
                // u32Offset = (j * BboxNumEachGrid[i] + k) * 4;
                // f32X = outputTensors_host[i+4]->host<float>()[u32Offset + 0];
                // f32Y = outputTensors_host[i+4]->host<float>()[u32Offset + 1];
                // f32Width = outputTensors_host[i+4]->host<float>()[u32Offset + 2];
                // f32Height = outputTensors_host[i+4]->host<float>()[u32Offset + 3];

                

                // cout<<N<<":"<<f32X<<","<<f32Y <<","<<f32Width<<","<<f32Height<<","<<f32Score1<<endl;

                if(f32Score1>0.5)
                {
                    uint32_t x = j % grid_w;
                    uint32_t y = j / grid_w;
                    float dense_cx = (x+0.5) * BoxSteps[i] / in_w;
                    float dense_cy = (y+0.5) * BoxSteps[i] / in_h;
                    float s_kx = BoxMinSizes[i][k] / in_w;
                    float s_ky = BoxMinSizes[i][k] / in_h;

                    float max_val=0.999;
                    float min_val=0.001;


                    u32Offset = (j * BboxNumEachGrid[i] + k) * 4;

                    f32X = outputTensors_host[i+4]->host<float>()[u32Offset + 0]*0.1*s_kx+dense_cx;
                    f32Y = outputTensors_host[i+4]->host<float>()[u32Offset + 1]*0.1*s_ky+dense_cy;
                    f32Width = exp(outputTensors_host[i+4]->host<float>()[u32Offset + 2]*0.2)*s_kx;
                    f32Height = exp(outputTensors_host[i+4]->host<float>()[u32Offset + 3]*0.2)*s_ky;

                    // cout<<u32Offset<<" score:"<<f32Score1<<","<<f32X<<","<<f32Y<<","<<f32Width<<","<<f32Height<<","<<endl;

                    f32Xmin = min(float(f32X-0.5*f32Width),max_val);
                    f32Ymin = min(float(f32Y-0.5*f32Height),max_val);
                    f32Xmax  =min(float(f32X+0.5*f32Width),max_val);
                    f32Ymax = min(float(f32Y+0.5*f32Height),max_val);
                    f32Xmin = max(f32Xmin,min_val);
                    f32Ymin = max(f32Ymin,min_val);
                    f32Xmax = max(f32Xmax,min_val);
                    f32Ymax = max(f32Ymax,min_val);
                    box[N][0] = f32Xmin;
                    box[N][1] = f32Ymin;
                    box[N][2] = max(f32Xmax-f32Xmin,min_val);
                    box[N][3] = max(f32Ymax-f32Ymin,min_val);
                    box[N][4] = f32Score1;
                    vec_boxs.push_back(box[N]);

                    // cout<<u32OffsetScore<<" score:"<<f32Score1<<","<<f32Xmin<<","<<f32Ymin<<","<<f32Xmax<<","<<f32Ymax<<","<<endl;
                    BboxNum++;
                }
                N++;
            }
        }
    }
    // cout<<"BboxNum:"<<BboxNum<<endl;
    nms_boxs = nms(vec_boxs, 0.5,0.5,nms_indexes);
    // cout<<"nms_boxs:"<<nms_boxs.size()<<endl;
    
    m_rectinfo.nNum=0;
    // landmarkinfo.nFaceNum=0;

    for (int j = 0; j < int(nms_boxs.size()); j++)
    {
        Box rect;
        Label label_;
        rect.xmin=nms_boxs.at(j)[0]*image_w;
        rect.ymin=nms_boxs.at(j)[1]*image_h;
        rect.width=nms_boxs.at(j)[2]*image_w;
        rect.height=nms_boxs.at(j)[3]*image_h;
        label_.cls=1;
        label_.score=nms_boxs.at(j)[4];

        m_rectinfo.boxes[j] = rect;
        m_rectinfo.labels[j] = label_;

        
        // STRU_Landmark_T landmark_t;
        // landmark_t.point[0].x=nms_landmarks.at(j)[0]*float(origin_image_width);
        // landmark_t.point[0].y=nms_landmarks.at(j)[1]*float(origin_image_height);
        // landmark_t.point[1].x=nms_landmarks.at(j)[2]*origin_image_width;
        // landmark_t.point[1].y=nms_landmarks.at(j)[3]*origin_image_height;
        // landmark_t.point[2].x=nms_landmarks.at(j)[4]*origin_image_width;
        // landmark_t.point[2].y=nms_landmarks.at(j)[5]*origin_image_height;
        // landmark_t.point[3].x=nms_landmarks.at(j)[6]*origin_image_width;
        // landmark_t.point[3].y=nms_landmarks.at(j)[7]*origin_image_height;
        // landmark_t.point[4].x=nms_landmarks.at(j)[8]*origin_image_width;
        // landmark_t.point[4].y=nms_landmarks.at(j)[9]*origin_image_height;
        // landmark_t.score=1;
        // landmarkinfo.landmark[j]=landmark_t;
//        cout<<j<<":"<<landmarkinfo.landmark[j].point[0].x<<endl;

    }

    m_rectinfo.nNum=m_rectinfo.nNum+int(nms_boxs.size());
    // landmarkinfo.nFaceNum=landmarkinfo.nFaceNum+int(nms_boxs.size());
    // cout<<"rectinfo.nFaceNum:"<<rectinfo.nFaceNum<<endl;
    return 1;

}



// int FaceDetect::visImg(const M2::ImgData_T &imagedata,const M2::DetectResult &rectinfo)
// {
    

//     cv::Mat ori_image(cv::Size(imagedata.width, imagedata.height), CV_8UC3);
// 	ori_image.data =imagedata.data;

//     cv::Mat image=ori_image.clone();
    

//     for(int i=0;i<rectinfo.nNum;i++)
//     {
//         std::string text =std::to_string(rectinfo.labels[i].score);
//         int font_face = cv::FONT_HERSHEY_COMPLEX;
//         double font_scale = 1;
//         int thickness = 1;
//     //    int baseline;
//     //    cv::Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
//         // 将文本框居中绘制
//         cv::Point origin;
//         origin.x = rectinfo.boxes[i].xmin+20;
//         origin.y = rectinfo.boxes[i].ymin+20;
//         cv::putText(image, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);

//         cv::Rect r = cv::Rect(rectinfo.boxes[i].xmin, rectinfo.boxes[i].ymin, rectinfo.boxes[i].width, rectinfo.boxes[i].height);
//         cv::rectangle(image, r, cv::Scalar(255, 0, 0), 1, 8, 0);
//     }
//     cv::imshow("1",image);
//     cv::waitKey(0);
//     return 0;
// }


FaceDetect::~FaceDetect() {
    
    if (net!=nullptr){
        net->releaseModel();
        net->releaseSession(session);
        for (int i = 0; i < input_blob_names.size(); i++) {
            // delete inputTensors[i];
            delete inputTensors_host[i];
        }
        for (int i = 0; i < output_blob_names.size(); i++) {
            // delete outputTensors[i];
            delete outputTensors_host[i];
        }
        inputTensors.clear();
        inputTensors_host.clear();
        outputTensors.clear();
        outputTensors_host.clear();
        input_blob_names.clear();
        output_blob_names.clear();

    }
}

