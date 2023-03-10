//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "FaceDetect.hpp"
#include "M2utils/nms.h"
#include "M2utils/process.h"
using namespace std;
using namespace M2;



M2::ObjectInfo GetMaxFace(M2::ObjectInfo objinfo,int &rect_index)//获取面积最大的人脸
{
	M2:ObjectInfo temp;
	int maxarea = 0;
	for (int i = 0;i < objinfo.ObjectNum;i++)
	{
		int currarea = objinfo.objects[i].rect.width*objinfo.objects[i].rect.height;
		if (maxarea < currarea)
		{
			maxarea = currarea;
			temp.ObjectNum = 1;
			temp.objects[0].rect = objinfo.objects[i].rect;
            temp.objects[0].label = objinfo.objects[i].label;
            temp.objects[0].prob = objinfo.objects[i].prob;
            rect_index=i;
		}
	}
	return temp;
}

// M2:ObjectInfo GetMidFace(M2:ObjectInfo obj,int img_w, int img_h,int &rect_index)//获取居中的人脸区域
// {
// 	STRU_RectInfo_T temp;
// 	int midcoordinate_x = img_w / 2;
// 	int midcoordinate_y = img_h / 2;
// 	int mindis = img_w*img_h;
// 	for (int i = 0;i < rectinfo.nNum;i++)
// 	{
// 		int currx = rectinfo.boxes[i].xmin + rectinfo.boxes[i].width / 2;
// 		int curry = rectinfo.boxes[i].ymin + rectinfo.boxes[i].height / 2;
// 		int disx = midcoordinate_x - currx;
// 		int disy = midcoordinate_y - curry;
// 		int currdis = disx*disx + disy*disy;
// 		if (currdis < mindis)
// 		{
// 			mindis = currdis;
// 			temp.nNum = 1;
// 		    temp.boxes[0] = rectinfo.boxes[i];
// 			temp.labels[0] = rectinfo.labels[i];
//             rect_index=i;
// 		}
// 	}
// 	return temp;
// }



static float s_af32ExpCoef[10][16] = {
    {1.0f, 1.00024f, 1.00049f, 1.00073f, 1.00098f, 1.00122f, 1.00147f, 1.00171f, 1.00196f, 1.0022f, 1.00244f, 1.00269f, 1.00293f, 1.00318f, 1.00342f, 1.00367f},
    {1.0f, 1.00391f, 1.00784f, 1.01179f, 1.01575f, 1.01972f, 1.02371f, 1.02772f, 1.03174f, 1.03578f, 1.03984f, 1.04391f, 1.04799f, 1.05209f, 1.05621f, 1.06034f},
    {1.0f, 1.06449f, 1.13315f, 1.20623f, 1.28403f, 1.36684f, 1.45499f, 1.54883f, 1.64872f, 1.75505f, 1.86825f, 1.98874f, 2.117f, 2.25353f, 2.39888f, 2.55359f},
    {1.0f, 2.71828f, 7.38906f, 20.0855f, 54.5981f, 148.413f, 403.429f, 1096.63f, 2980.96f, 8103.08f, 22026.5f, 59874.1f, 162755.0f, 442413.0f, 1.2026e+006f, 3.26902e+006f},
    {1.0f, 8.88611e+006f, 7.8963e+013f, 7.01674e+020f, 6.23515e+027f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f},
    {1.0f, 0.999756f, 0.999512f, 0.999268f, 0.999024f, 0.99878f, 0.998536f, 0.998292f, 0.998049f, 0.997805f, 0.997562f, 0.997318f, 0.997075f, 0.996831f, 0.996588f, 0.996345f},
    {1.0f, 0.996101f, 0.992218f, 0.98835f, 0.984496f, 0.980658f, 0.976835f, 0.973027f, 0.969233f, 0.965455f, 0.961691f, 0.957941f, 0.954207f, 0.950487f, 0.946781f, 0.94309f},
    {1.0f, 0.939413f, 0.882497f, 0.829029f, 0.778801f, 0.731616f, 0.687289f, 0.645649f, 0.606531f, 0.569783f, 0.535261f, 0.502832f, 0.472367f, 0.443747f, 0.416862f, 0.391606f},
    {1.0f, 0.367879f, 0.135335f, 0.0497871f, 0.0183156f, 0.00673795f, 0.00247875f, 0.000911882f, 0.000335463f, 0.00012341f, 4.53999e-005f, 1.67017e-005f, 6.14421e-006f, 2.26033e-006f, 8.31529e-007f, 3.05902e-007f},
    {1.0f, 1.12535e-007f, 1.26642e-014f, 1.42516e-021f, 1.60381e-028f, 1.80485e-035f, 2.03048e-042f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};

static float SVP_NNIE_QuickExp( int s32Value )
{
    if( s32Value & 0x80000000 )
    {
        s32Value = ~s32Value + 0x00000001;
        return s_af32ExpCoef[5][s32Value & 0x0000000F] * s_af32ExpCoef[6][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[7][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[8][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[9][(s32Value>>16) & 0x0000000F ];
    }
    else
    {
        return s_af32ExpCoef[0][s32Value & 0x0000000F] * s_af32ExpCoef[1][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[2][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[3][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[4][(s32Value>>16) & 0x0000000F ];
    }
}

static int SVP_NNIE_SoftMax2( float &pf32Src1,float &pf32Src2)
{
    float f32Max = 0;
    float f32Sum = 0;
    uint32_t i = 0;

    if(f32Max <  pf32Src1) f32Max =  pf32Src1;
    if(f32Max <  pf32Src2) f32Max =  pf32Src2;
    pf32Src1 = (float)SVP_NNIE_QuickExp((int)((pf32Src1 - f32Max))*4096);
    pf32Src2 = (float)SVP_NNIE_QuickExp((int)((pf32Src2 - f32Max))*4096);
    f32Sum =pf32Src1+pf32Src2;
    pf32Src1 /= f32Sum;
    pf32Src2 /= f32Sum;
    return 0;
}


static inline float intersection_area(const Object &a, const Object &b) {
  cv::Rect_<float> inter = a.rect & b.rect;
  return inter.area();
}

static void qsort_descent_inplace(std::vector<Object> &faceobjects, int left,
                                  int right) {
  int i = left;
  int j = right;
  float p = faceobjects[(left + right) / 2].prob;

  while (i <= j) {
    while (faceobjects[i].prob > p)
      i++;

    while (faceobjects[j].prob < p)
      j--;

    if (i <= j) {
      // swap
      std::swap(faceobjects[i], faceobjects[j]);

      i++;
      j--;
    }
  }

#pragma omp parallel sections
  {
#pragma omp section
    {
      if (left < j)
        qsort_descent_inplace(faceobjects, left, j);
    }
#pragma omp section
    {
      if (i < right)
        qsort_descent_inplace(faceobjects, i, right);
    }
  }
}

static void qsort_descent_inplace(std::vector<Object> &objects) {
  if (objects.empty())
    return;

  qsort_descent_inplace(objects, 0, objects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<Object> &faceobjects,
                              std::vector<int> &picked, float nms_threshold) {
  picked.clear();

  const int n = faceobjects.size();

  std::vector<float> areas(n);
  for (int i = 0; i < n; i++) {
    areas[i] = faceobjects[i].rect.area();
  }

  for (int i = 0; i < n; i++) {
    const Object &a = faceobjects[i];

    int keep = 1;
    for (int j = 0; j < (int)picked.size(); j++) {
      const Object &b = faceobjects[picked[j]];

      // intersection over union
      float inter_area = intersection_area(a, b);
      float union_area = areas[i] + areas[picked[j]] - inter_area;
      // float IoU = inter_area / union_area
      if (inter_area / union_area > nms_threshold)
        keep = 0;
    }

    if (keep)
      picked.push_back(i);
  }
}




FaceDetect::FaceDetect() {


}


int FaceDetect::Init(int printConfig,int modelType,float score_thresh)
{

    m_print=printConfig;
    m_modelType=modelType;
    m_score_thresh=score_thresh;
    string mnnPath;

    if (modelType==0){ 
        mnnPath="./models206/facedetect_h360_w640_c3.mnn" ;
        dimType = MNN::Tensor::CAFFE;

        MNN_PRINT("\n");
        MNN_PRINT("modelType=0 (facedetect model : my slim model) \n");
        MNN_PRINT("Interpreter build, model_path: %s, dimType:%d\n",mnnPath.c_str(),dimType);
        MNN_PRINT("\n");

        in_h=360;
        in_w=640;
        input_blob_names={"input0"};
        output_blob_names={ "conf0","conf1","conf2","conf3","loc0","loc1","loc2","loc3",};
        float mean[3]     = {104.0f, 117.0f, 123.0f};
        float normals[3] = {1.0f, 1.0f, 1.0f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::BGR;
        imconfig.filterType = MNN::CV::NEAREST;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    }
    else if(modelType==1)
    {
        mnnPath="./models206/facedetect_h640_w320_c3.mnn" ;
        dimType = MNN::Tensor::CAFFE;

        MNN_PRINT("\n");
        MNN_PRINT("modelType=0 (facedetect model : my slim model) \n");
        MNN_PRINT("Interpreter build, model_path: %s, dimType:%d\n",mnnPath.c_str(),dimType);
        MNN_PRINT("\n");

        in_h=640;
        in_w=320;
        input_blob_names={"input0"};
        output_blob_names={ "conf0","conf1","conf2","conf3","loc0","loc1","loc2","loc3",};
        float mean[3]     = {104.0f, 117.0f, 123.0f};
        float normals[3] = {1.0f, 1.0f, 1.0f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::BGR;
        imconfig.filterType = MNN::CV::NEAREST;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    }
    else{
        MNN_PRINT(" wrong modelType %d,should be 0 or 1  \n",modelType);
        MNN_PRINT("\n");
    }



    net = std::shared_ptr<MNN::Interpreter>(MNN::Interpreter::createFromFile(mnnPath.c_str()));
    MNN::ScheduleConfig config;
    int deviceTpye=3;
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
    // if(m_print>=1){
    //     chrono::duration<double> elapsed2 = chrono::steady_clock::now() - start;
    //     cout << "net time:" << elapsed2.count() << " s" << endl;
    //     outputTensors_host[0]->printShape();
    //     for (int i = 0; i < 20; ++i) {
    //         MNN_PRINT("copy %f\n", outputTensors_host[0]->host<float>()[i]);
    //     }
    // }


    decode(outputTensors_host);

    objectinfo.ObjectNum=m_objInfo.ObjectNum;
    for (int i = 0; i < m_objInfo.ObjectNum; i++)
    {
        objectinfo.objects[i].rect=m_objInfo.objects[i].rect;
        objectinfo.objects[i].prob=m_objInfo.objects[i].prob;
        objectinfo.objects[i].label=m_objInfo.objects[i].label;
    }




    if(objectinfo.ObjectNum>1 && (max_or_mid==0 || max_or_mid==1))
    {
        int index;
        M2::ObjectInfo temp_objectinfo;
        if (max_or_mid==0)
        {
            temp_objectinfo=GetMaxFace(objectinfo,index);
        }else if (max_or_mid==1)
        {
            temp_objectinfo=GetMaxFace(objectinfo,index);
        }else
        {
            temp_objectinfo=GetMaxFace(objectinfo,index);
        }
        // STRU_LandmarkInfo_T temp_landmarkinfo;
        // temp_landmarkinfo.nFaceNum=1;
        // temp_landmarkinfo.landmark[0]=landmarkinfo.landmark[index];
        // m_rectinfo=temp_rectinfo;
        // landmarkinfo=temp_landmarkinfo;
        objectinfo.ObjectNum=1;
        objectinfo.objects[0].rect=temp_objectinfo.objects[0].rect;
        objectinfo.objects[0].prob=temp_objectinfo.objects[0].prob;
        objectinfo.objects[0].label=temp_objectinfo.objects[0].label;
    }
    


    if(m_print>=1){
        chrono::duration<double> elapsed3 = chrono::steady_clock::now() - start;
        cout << "FaceDetect Decode time:" << elapsed3.count() << " s" << endl;
    }

    // if(m_print>=2){
    //     for (int j = 0; j <objectinfo.ObjectNum; j++)
    //     {
    //         cout <<j << ": " << objectinfo.objects[j].rect.x << " " << objectinfo.objects[j].rect.y  << " "<<objectinfo.objects[j].rect.width << " "<< objectinfo.objects[j].rect.height << " "<<objectinfo.objects[j].prob<< endl;
    //     }
    // }

    return 0;
}




int FaceDetect::decode(std::vector< MNN::Tensor*> &outputTensors_host)
{
    // for (int i = 0; i < 10; ++i) {
    //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
    // }

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
    float strides[4]={8,16,32,64};
    float BoxMinSizes[4][3] = {{10, 16, 24}, {32, 48,0}, {64, 96,0}, {128, 192, 256}};


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
    std::vector<Object> proposals;
    for(int i = start; i < end; i++)
    {

        uint32_t u32Offset=0;
        uint32_t u32OffsetScore=0;


        int  grid_h = outputTensors_host[i]->height();
        int  grid_w = outputTensors_host[i]->width();
        int  grid_c = outputTensors_host[i]->channel();
        float  stride =strides[i];
        // cout<<"grid_h:"<<grid_h<<"  grid_w:"<<grid_w<<"  grid_c:  "<<grid_c<<endl;

        for(int h=0;h<grid_h;h++)
        {
            for(int w=0;w<grid_w;w++)
            {
                for(int k = 0; k < BboxNumEachGrid[i]; k++)
                {
                    f32Score0=outputTensors_host[i]->host<float>()[w+h*grid_w+(k*2)*grid_h*grid_w];
                    f32Score1=outputTensors_host[i]->host<float>()[w+h*grid_w+(k*2+1)*grid_h*grid_w];
                    // cout<<"f32Score0:"<<f32Score0<<"  f32Score1:"<<f32Score1<<endl;
                    SVP_NNIE_SoftMax2(f32Score0,f32Score1);
                    if(f32Score1>m_score_thresh)
                    {
                        // cout<<"f32Score0:"<<f32Score0<<"  f32Score1:"<<f32Score1<<endl;
                        float max_val=0.999;
                        float min_val=0.001;
                        float dense_cx = (w+0.5) * stride / in_w;
                        float dense_cy = (h+0.5) * stride / in_h;
                        float s_kx = BoxMinSizes[i][k] / in_w;
                        float s_ky = BoxMinSizes[i][k] / in_h;
                        f32X = outputTensors_host[i+4]->host<float>()[w+h*grid_w+(k*4)*grid_h*grid_w]*0.1*s_kx+dense_cx;
                        f32Y = outputTensors_host[i+4]->host<float>()[w+h*grid_w+(k*4+1)*grid_h*grid_w]*0.1*s_ky+dense_cy;
                        f32Width = exp(outputTensors_host[i+4]->host<float>()[w+h*grid_w+(k*4+2)*grid_h*grid_w]*0.2)*s_kx;
                        f32Height = exp(outputTensors_host[i+4]->host<float>()[w+h*grid_w+(k*4+3)*grid_h*grid_w]*0.2)*s_ky;
                        // cout<<" score:"<<f32Score1<<","<<f32X<<","<<f32Y<<","<<f32Width<<","<<f32Height<<","<<endl;
                        f32Xmin = min(float(f32X-0.5*f32Width),max_val);
                        f32Ymin = min(float(f32Y-0.5*f32Height),max_val);
                        f32Xmax  =min(float(f32X+0.5*f32Width),max_val);
                        f32Ymax = min(float(f32Y+0.5*f32Height),max_val);
                        f32Xmin = max(f32Xmin,min_val);
                        f32Ymin = max(f32Ymin,min_val);
                        f32Xmax = max(f32Xmax,min_val);
                        f32Ymax = max(f32Ymax,min_val);

                        Object obj;
                        obj.rect.x = f32Xmin;
                        obj.rect.y = f32Ymin;
                        obj.rect.width =f32Xmax-f32Xmin;
                        obj.rect.height = f32Ymax-f32Ymin;
                        obj.label =0;
                        obj.prob = f32Score1;
                        proposals.push_back(obj);
                    }
                    
                }

            }
        }
    }

    qsort_descent_inplace(proposals);

    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked, 0.5);
    int count = picked.size();
    m_objInfo.ObjectNum=count;
    // cout<<"cout:"<<count<<endl;
    std::vector<Object> objects;
    objects.resize(count);

    for (int i = 0; i < count; i++) {
        objects[i] = proposals[picked[i]];

        // adjust offset to original unpadded
        float x0 = (objects[i].rect.x) *image_w;
        float y0 = (objects[i].rect.y)*image_h;
        float x1 = (objects[i].rect.x + objects[i].rect.width) *image_w;;
        float y1 = (objects[i].rect.y + objects[i].rect.height)*image_h;
        // cout<<" x0:"<<x0<<" y0:"<<y0<<" x1:"<<x1<<" y1:"<<y1<<endl;
        // clip
        x0 = std::max(std::min(x0, (float)(image_w - 1)), 0.f);
        y0 = std::max(std::min(y0, (float)(image_h - 1)), 0.f);
        x1 = std::max(std::min(x1, (float)(image_w - 1)), 0.f);
        y1 = std::max(std::min(y1, (float)(image_h- 1)), 0.f);

        // cout<<" x0:"<<x0<<" y0:"<<y0<<" x1:"<<x1<<" y1:"<<y1<<endl;
        m_objInfo.objects[i].rect.x = x0;
        m_objInfo.objects[i].rect.y = y0;
        m_objInfo.objects[i].rect.width = x1 - x0;
        m_objInfo.objects[i].rect.height = y1 - y0;
        m_objInfo.objects[i].label=objects[i].label;
        m_objInfo.objects[i].prob=objects[i].prob;
    }
    
    return 0;

}


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

