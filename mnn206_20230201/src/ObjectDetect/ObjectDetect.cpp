//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "ObjectDetect.hpp"
#include <opencv2/opencv.hpp>
#include "M2utils/nms.h"
#include "M2utils/process.h"
using namespace std;


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




ObjectDetect::ObjectDetect() {

}


int ObjectDetect::InitWithBatch(int printCfg,int modelType,int batch)
{
    
    m_print=printCfg;
    m_modelType=modelType;
    string mnnPath;

    if (modelType==0){ 
        mnnPath="./models206/yolox_tiny_320_320_extract.mnn" ;
        dimType = MNN::Tensor::CAFFE;
        in_h=320;
        in_w=320;
        input_blob_names={"input_0"};
        output_blob_names={"/head/Concat_output_0","/head/Concat_1_output_0","/head/Concat_2_output_0"};
        float mean[3]     = {0.0f, 0.0f, 0.0f};
        float normals[3] = {1.0f, 1.0f, 1.0f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::BGR;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
    }


    if (modelType==1){ 
        mnnPath="./models206/yolox_tiny_640_640_silu_cocobdd_extract.mnn" ;
        dimType = MNN::Tensor::CAFFE;
        in_h=640;
        in_w=640;
        input_blob_names={"input0"};
        output_blob_names={"/head/Concat_output_0","/head/Concat_1_output_0","/head/Concat_2_output_0"};
        float mean[3]     = {0.0f, 0.0f, 0.0f};
        float normals[3] = {1.0f, 1.0f, 1.0f};
        ::memcpy(imconfig.mean, mean, sizeof(mean));
        ::memcpy(imconfig.normal, normals, sizeof(normals));
        imconfig.sourceFormat = MNN::CV::BGR;
        imconfig.destFormat = MNN::CV::BGR;
        pretreat=std::shared_ptr<MNN::CV::ImageProcess>(MNN::CV::ImageProcess::create(imconfig));
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

    if(batch<0)
    {
        MNN_PRINT("wrong batch: %d\n",batch);
        return -1;
    }
    
    if (batch>1)
    {
      auto shape = inputTensors[0]->shape();
      shape[0]   = batch;
      net->resizeTensor(inputTensors[0], shape);
      net->resizeSession(session);
    }


    for (int i = 0; i < input_blob_names.size(); i++) {
		  inputTensors_host[i] = new MNN::Tensor(inputTensors[i], dimType);
	  }


    
    outputTensors.resize(output_blob_names.size());
    outputTensors_host.resize(output_blob_names.size());
    for (int i = 0; i < output_blob_names.size(); i++) 
    {
		  outputTensors[i] = net->getSessionOutput(session, output_blob_names[i].c_str());
	  }
    for (int i = 0; i < output_blob_names.size(); i++) 
    {
		  outputTensors_host[i] = new MNN::Tensor(outputTensors[i], dimType);
	  }

    float memoryUsage = 0.0f;
    net->getSessionInfo(session, MNN::Interpreter::MEMORY, &memoryUsage);
    float flops = 0.0f;
    net->getSessionInfo(session, MNN::Interpreter::FLOPS, &flops);
    int backendType[2];
    net->getSessionInfo(session, MNN::Interpreter::BACKENDS, backendType);
    MNN_PRINT("Session Info: memory use %f MB, flops is %f M, backendType is %d, batch size = %d\n", memoryUsage, flops, backendType[0], batch);


    for (int i = 0; i < input_blob_names.size(); i++) 
    {
      MNN_PRINT("%s\n",input_blob_names[i].c_str());
		  inputTensors_host[i]->printShape();
	  }


    for (int i = 0; i < output_blob_names.size(); i++) 
    {
      MNN_PRINT("%s\n",output_blob_names[i].c_str());
		  outputTensors_host[i]->printShape();
	  } 

    return 0;
}



int ObjectDetect::ForwardBGR(const cv::Mat &image,M2::ObjectInfo &objectinfo) {


    //	 cv::imshow("in",image);
    //	 cv::waitKey(0);


    auto start = chrono::steady_clock::now();
    image_h = image.rows;
    image_w = image.cols;
    cv::Mat resize_img;
    cv::resize(image,resize_img,cv::Size(in_w,in_h));
    pretreat->convert((uint8_t *)resize_img.data, in_w,in_h,0,inputTensors_host[0]);
    inputTensors[0]->copyFromHostTensor(inputTensors_host[0]);
    net->runSession(session);
    for (int i = 0; i < output_blob_names.size(); i++) {
		  outputTensors[i]->copyToHostTensor(outputTensors_host[i]);
	  } 

    decode(outputTensors_host);


    // if(m_print>=1){
    //     chrono::duration<double> elapsed2 = chrono::steady_clock::now() - start;
    //     cout << "ObjectDetect time:" << elapsed2.count() << " s" << endl;
    //     outputTensors_host[0]->printShape();
    //     // for (int i = 0; i < 20; ++i) {
    //     //     MNN_PRINT("copy %f\n", outputTensors_host[0]->host<float>()[i]);
    //     // }
    // }



    objectinfo.ObjectNum=m_objInfo.ObjectNum;
    for (int i = 0; i < m_objInfo.ObjectNum; i++)
    {
        objectinfo.objects[i].rect=m_objInfo.objects[i].rect;
        objectinfo.objects[i].prob=m_objInfo.objects[i].prob;
        objectinfo.objects[i].label=m_objInfo.objects[i].label;
    }

    if(m_print>=1){
        chrono::duration<double> elapsed3 = chrono::steady_clock::now() - start;
        cout << "ObjectDetect time:" << elapsed3.count() << " s" << endl;
    }

    

    // if(m_print>=2){
    //     for (int i = 0; i < m_objInfo.ObjectNum; i++)
    //     {
    //         cout <<i << ": " << m_objInfo.objects[i].rect.x << " " << m_objInfo.objects[i].rect.y << " "<< m_objInfo.objects[i].rect.width << " "<< m_objInfo.objects[i].rect.height << " "<<m_objInfo.objects[i].label<<" "<<m_objInfo.objects[i].prob<< endl;
    //     }
    // }

    return 0;
}


int ObjectDetect::ForwardBatchBGR(const std::vector<cv::Mat> &batch_images,std::vector<M2::ObjectInfo> &batch_objectinfo) 
{
    int batch=batch_images.size();
    if(batch<0)
    {
      std::cout<<"no image in batch_images"<<std::endl;
      return -1;
    }


    auto start = chrono::steady_clock::now();

    std::vector<cv::Mat> resize_img_vector;
    std::vector<cv::Size> imageSize_vector;


    resize_img_vector.resize(batch);
    imageSize_vector.resize(batch);
    std::vector<cv::Mat> img_channels;
    auto data_host = inputTensors_host[0]->host<float>();
    auto batch_stride= inputTensors_host[0]->stride(0);
    int channel_size = in_h * in_w;
    for(int i=0;i<batch;i++)
    {   
        img_channels.clear();
        
        cv::resize(batch_images.at(i),resize_img_vector.at(i),cv::Size(in_w,in_h));
        cv::split(resize_img_vector.at(i),img_channels);

        for(int j=0;j<img_channels.size();j++)
        {
            // normalize
            float alpha = 1.0;
            float beta = 0.0;

            img_channels[j].convertTo(img_channels[j], CV_32F, alpha, beta);
            // cv::Mat是HWC格式，需要逐通道拷贝进nchw_tensor 
            ::memcpy(data_host+batch_stride*i + channel_size * j, img_channels[j].data, channel_size  * sizeof(float));

        }
        

        imageSize_vector.at(i)=batch_images.at(i).size();
        // pretreat->convert((uint8_t *)resize_img_vector.at(i).data, in_w,in_h,0,inputTensors_host[0]);
        //pretreat->convert((uint8_t*)resize_img_vector.at(i).data, in_w,in_h,0,inputTensors_host[0]->host<uint8_t>()+ inputTensors_host[0]->stride(0) * i * inputTensors_host[0]->getType().bytes(),size_w, size_h, bpp, 0, inputTensors_host[0]->getType());
    }
    inputTensors[0]->copyFromHostTensor(inputTensors_host[0]);
    net->runSession(session);
    for (int i = 0; i < output_blob_names.size(); i++)
    {
		    outputTensors[i]->copyToHostTensor(outputTensors_host[i]);
	  }

    batch_objectinfo.clear();
    batch_objectinfo.resize(batch);
    DecodeBatch(batch,imageSize_vector,outputTensors_host,batch_objectinfo);
    if(m_print>=1){
        chrono::duration<double> elapsed3 = chrono::steady_clock::now() - start;
        cout << "ObjectDetect time:" << elapsed3.count() << " s" << endl;
    }

    return 0;
}


int ObjectDetect::DecodeBatch(int batch, std::vector<cv::Size> &imageSize_vector,std::vector< MNN::Tensor*> &outputTensors_host,std::vector<M2::ObjectInfo> &batch_objectinfo)
{

    
    int end=output_blob_names.size();
    float x_center,y_center,x0,y0,ww,hh,box_objectness,box_prob,box_cls_score;
    float max_val=0.999;
    float min_val=0.001;

    float strides[3]={8,16,32};


    for(int b=0;b<batch;b++)
    {

      std::vector<Object> proposals;
      image_h = imageSize_vector.at(b).height;
      image_w = imageSize_vector.at(b).width;

      for(int i = 0; i < end; i++)
      {
        uint32_t u32Offset=0;
        uint32_t u32OffsetScore=0;


        int  grid_h = outputTensors_host[i]->height();
        int  grid_w = outputTensors_host[i]->width();
        int  grid_c = outputTensors_host[i]->channel();
        int  grid_stride=outputTensors_host[i]->stride(0);
        // std::cout<<grid_stride<<std::endl;
        float  stride =strides[i];

        for(int h=0;h<grid_h;h++)
        {
          for(int w=0;w<grid_w;w++)
          {

            box_objectness=outputTensors_host[i]->host<float>()[w+h*grid_w+4*grid_h*grid_w+grid_stride*b];
            // std::cout<<b<<","<<i<<","<<h<<","<<w<<","<<box_objectness<<std::endl;
            if(box_objectness>0.5)
            {
                // std::cout<<b<<","<<i<<","<<h<<","<<w<<","<<box_objectness<<std::endl;
                for(int c=5;c<grid_c;c++)
                {
                    box_cls_score = outputTensors_host[i]->host<float>()[w+h*grid_w+c*grid_h*grid_w];
                    box_prob = box_objectness * box_cls_score;
                    if(box_prob>0.3)
                    {
                        x_center = (outputTensors_host[i]->host<float>()[w+h*grid_w+0*grid_h*grid_w+grid_stride*b]+w)*stride;
                        y_center = (outputTensors_host[i]->host<float>()[w+h*grid_w+1*grid_h*grid_w+grid_stride*b]+h)*stride;
                        ww = exp(outputTensors_host[i]->host<float>()[w+h*grid_w+2*grid_h*grid_w+grid_stride*b])*stride;
                        hh = exp(outputTensors_host[i]->host<float>()[w+h*grid_w+3*grid_h*grid_w+grid_stride*b])*stride;
                        x0 = x_center - ww * 0.5f;
                        y0 = y_center - hh * 0.5f;

                        //cout<<b<<","<<i<<":"<<h<<":"<<w<<":"<<c<<":"<<x_center<<","<<y_center<<","<<ww<<","<<hh<<","<<box_prob<<endl;

                        Object obj;
                        obj.rect.x = x0;
                        obj.rect.y = y0;
                        obj.rect.width = ww;
                        obj.rect.height = hh;
                        obj.label = c-5;
                        obj.prob = box_prob;

                        proposals.push_back(obj);
                     }
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
      std::vector<Object> objects;
      objects.resize(count);

      for (int i = 0; i < count; i++) {
          objects[i] = proposals[picked[i]];

          // adjust offset to original unpadded
          float x0 = (objects[i].rect.x) / float(in_w)*image_w;
          float y0 = (objects[i].rect.y) / float(in_h)*image_h;
          float x1 = (objects[i].rect.x + objects[i].rect.width) / float(in_w)*image_w;;
          float y1 = (objects[i].rect.y + objects[i].rect.height) / float(in_h)*image_h;
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
      // cout<<"m_objInfo.ObjectNum : "<<m_objInfo.ObjectNum<<endl;
      batch_objectinfo.at(b).ObjectNum=m_objInfo.ObjectNum;
      for (int i = 0; i < m_objInfo.ObjectNum; i++)
      {
          batch_objectinfo.at(b).objects[i].rect=m_objInfo.objects[i].rect;
          batch_objectinfo.at(b).objects[i].prob=m_objInfo.objects[i].prob;
          batch_objectinfo.at(b).objects[i].label=m_objInfo.objects[i].label;
      }

    }
    return 0;

}




int ObjectDetect::decode(std::vector< MNN::Tensor*> &outputTensors_host)
{
    // for (int i = 0; i < 10; ++i) {
    //     MNN_PRINT("func %f, %f\n", outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+0]);
    // }


    // float box[3000][5];
    // float landmark[3000][10];

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
    int end=output_blob_names.size();
//     int grid_hs[4];
//     int grid_ws[4];
//     int BboxNumEachGrid[4]={3,2,2,3};
    float strides[3]={8,16,32};
//     float BoxMinSizes[4][3] = {{10, 16, 24}, {32, 48,0}, {64, 96,0}, {128, 192, 256}};
//     grid_hs[0]=40;grid_hs[1]=20;grid_hs[2]=10;grid_hs[3]=5;
//     grid_ws[0]=30;grid_ws[1]=15;grid_ws[2]=8;grid_ws[3]=4;

//     float f32Score0;
//     float f32Score1;
//     float f32Score;
//     float f32X;
//     float f32Y;
//     float f32Xmin;
//     float f32Ymin;
//     float f32Xmax;
//     float f32Ymax;
//     float f32Width;
//     float f32Height;
//     int N=0;
//     int BboxNum=0;
    float x_center;
    float y_center;
    float x0;
    float y0;
    float ww;
    float hh;
    float box_objectness;
    float box_cls_score;
    float box_prob;
    std::vector<Object> proposals;
    // std::vector<Object> objects;
    for(int i = start; i < end; i++)
    {
        uint32_t u32Offset=0;
        uint32_t u32OffsetScore=0;


        int  grid_h = outputTensors_host[i]->height();
        int  grid_w = outputTensors_host[i]->width();
        int  grid_c = outputTensors_host[i]->channel();
        float  stride =strides[i];

        // cout<<i<<":"<<grid_h<<","<<grid_w <<","<<grid_c<<endl;

        for(int h=0;h<grid_h;h++)
        {
            for(int w=0;w<grid_w;w++)
            {

                float max_val=0.999;
                float min_val=0.001;


                box_objectness=outputTensors_host[i]->host<float>()[w+h*grid_w+4*grid_h*grid_w];
               
                if(box_objectness>0.5)
                {
                    
                    for(int c=5;c<grid_c;c++)
                    {
                        box_cls_score = outputTensors_host[i]->host<float>()[w+h*grid_w+c*grid_h*grid_w];
                        box_prob = box_objectness * box_cls_score;
                        if(box_prob>0.3)
                        {
                            x_center = (outputTensors_host[i]->host<float>()[w+h*grid_w+0*grid_h*grid_w]+w)*stride;
                            y_center = (outputTensors_host[i]->host<float>()[w+h*grid_w+1*grid_h*grid_w]+h)*stride;
                            ww = exp(outputTensors_host[i]->host<float>()[w+h*grid_w+2*grid_h*grid_w])*stride;
                            hh = exp(outputTensors_host[i]->host<float>()[w+h*grid_w+3*grid_h*grid_w])*stride;
                            x0 = x_center - ww * 0.5f;
                            y0 = y_center - hh * 0.5f;

                            // cout<<i<<":"<<h<<":"<<w<<":"<<c<<":"<<x_center<<","<<y_center<<","<<ww<<","<<hh<<","<<box_prob<<endl;

                            Object obj;
                            obj.rect.x = x0;
                            obj.rect.y = y0;
                            obj.rect.width = ww;
                            obj.rect.height = hh;
                            obj.label = c-5;
                            obj.prob = box_prob;

                            proposals.push_back(obj);
                        }
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
    
    std::vector<Object> objects;
    objects.resize(count);

    for (int i = 0; i < count; i++) {
        objects[i] = proposals[picked[i]];

        // adjust offset to original unpadded
        float x0 = (objects[i].rect.x) / float(in_w)*image_w;
        float y0 = (objects[i].rect.y) / float(in_h)*image_h;
        float x1 = (objects[i].rect.x + objects[i].rect.width) / float(in_w)*image_w;;
        float y1 = (objects[i].rect.y + objects[i].rect.height) / float(in_h)*image_h;
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
    // cout<<"m_objInfo.ObjectNum : "<<m_objInfo.ObjectNum<<endl;


    return 0;

}




ObjectDetect::~ObjectDetect() {
    
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

