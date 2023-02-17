//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "HandLandmark.hpp"
#include "M2utils/nms.h"
#include <opencv2/opencv.hpp>
using namespace std;
using namespace M2;

/*-------------------------------------------
                  Functions
-------------------------------------------*/

static void dump_tensor_attr(rknn_tensor_attr* attr)
{
  printf("  index=%d, name=%s, n_dims=%d, dims=[%d, %d, %d, %d], n_elems=%d, size=%d, fmt=%s, type=%s, qnt_type=%s, "
         "zp=%d, scale=%f\n",
         attr->index, attr->name, attr->n_dims, attr->dims[0], attr->dims[1], attr->dims[2], attr->dims[3],
         attr->n_elems, attr->size, get_format_string(attr->fmt), get_type_string(attr->type),
         get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}


static unsigned char* load_data(FILE* fp, size_t ofst, size_t sz)
{
  unsigned char* data;
  int            ret;

  data = NULL;

  if (NULL == fp) {
    return NULL;
  }

  ret = fseek(fp, ofst, SEEK_SET);
  if (ret != 0) {
    printf("blob seek failure.\n");
    return NULL;
  }

  data = (unsigned char*)malloc(sz);
  if (data == NULL) {
    printf("buffer malloc failure.\n");
    return NULL;
  }
  ret = fread(data, 1, sz, fp);
  return data;
}

static unsigned char* load_model(const char* filename, int* model_size)
{
  FILE*          fp;
  unsigned char* data;

  fp = fopen(filename, "rb");
  if (NULL == fp) {
    printf("Open file %s failed.\n", filename);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);

  data = load_data(fp, 0, size);

  fclose(fp);

  *model_size = size;
  return data;
}

static double __get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

static bool LessSort(lane_DECODE a, lane_DECODE b)  //����
{
    return (a.prob > b.prob);
}

static bool LessSort_point(cv::Point a, cv::Point b) //����
{
    return (a.y > b.y);
}
// �������ߵľ�������
static bool moreSort(lane_DECODE a, lane_DECODE b) //����
{
    return (a.dis < b.dis);
}

static int compare_greater(cv::Point2f a, cv::Point2f b)  //vector�õ���Զ������ĵ�
{
    return a.x > b.x;
}



/*-------------------------------------------
                  Functions
-------------------------------------------*/



HandLandmark::HandLandmark() {


}

int HandLandmark::Init(int deviceTpye,int print_config,int modelType){

    m_print=print_config;
    m_modelType=modelType;

    string model_name="./models206/hand_landmark_full.rknn";
    /* Create the neural network */
    printf("Loading mode...\n");
    int            model_data_size = 0;
    unsigned char* model_data      = load_model(model_name.c_str(), &model_data_size);

    
    int ret= rknn_init(&ctx, model_data, model_data_size, 0, NULL);

    rknn_sdk_version version;
    ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version, sizeof(rknn_sdk_version));
    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
    }
    printf("sdk version: %s driver version: %s\n", version.api_version, version.drv_version);

    
    ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret < 0) {
        printf("rknn_init error ret=%d\n", ret);
        return -1;
    }
    printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < io_num.n_input; i++) {
        input_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret < 0) {
            printf("rknn_init error ret=%d\n", ret);
            return -1;
        }
        dump_tensor_attr(&(input_attrs[i]));
    }

    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < io_num.n_output; i++) {
        output_attrs[i].index = i;
        ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        dump_tensor_attr(&(output_attrs[i]));
    }


    if (input_attrs[0].fmt == RKNN_TENSOR_NCHW) {
        printf("model is NCHW input fmt\n");
        channel = input_attrs[0].dims[1];
        height  = input_attrs[0].dims[2];
        width   = input_attrs[0].dims[3];
    } else {
        printf("model is NHWC input fmt\n");
        height  = input_attrs[0].dims[1];
        width   = input_attrs[0].dims[2];
        channel = input_attrs[0].dims[3];
    }
    printf("model input height=%d, width=%d, channel=%d\n", height, width, channel);


    memset(inputs, 0, sizeof(inputs));
    inputs[0].index        = 0;
    inputs[0].type         = RKNN_TENSOR_UINT8;
    inputs[0].size         = width * height * channel;
    inputs[0].fmt          = RKNN_TENSOR_NHWC;
    inputs[0].pass_through = 0;

}

int HandLandmark::ForwardBGR(const cv::Mat &img,std::vector<M2::lane_DECODE> &final_lane) {

    struct timeval start_time, stop_time;

    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect      src_rect;
    im_rect      dst_rect;
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    int ret;




    img_width  = img.cols;
    img_height = img.rows;
    printf("img width = %d, img height = %d\n", img_width, img_height);

    // You may not need resize when src resulotion equals to dst resulotion
    void* resize_buf = nullptr;
    if (img_width != width || img_height != height) {
        printf("resize with RGA!\n");
        resize_buf = malloc(height * width * channel);
        memset(resize_buf, 0x00, height * width * channel);
        src = wrapbuffer_virtualaddr((void*)img.data, img_width, img_height, RK_FORMAT_RGB_888);
        dst = wrapbuffer_virtualaddr((void*)resize_buf, width, height, RK_FORMAT_RGB_888);
        ret = imcheck(src, dst, src_rect, dst_rect);
        if (IM_STATUS_NOERROR != ret) {
            printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
            return -1;
        }
        IM_STATUS STATUS = imresize(src, dst);
        // for debug
        // cv::Mat resize_img(cv::Size(width, height), CV_8UC3, resize_buf);
        // cv::imwrite("resize_input.jpg", resize_img);
        inputs[0].buf = resize_buf;
    } else {
        inputs[0].buf = (void*)img.data;
    }

    gettimeofday(&start_time, NULL);
    rknn_inputs_set(ctx, io_num.n_input, inputs);

    
    memset(outputs, 0, sizeof(outputs));
    for (int i = 0; i < io_num.n_output; i++) {
        outputs[i].want_float = 1;
    }
    ret = rknn_run(ctx, NULL);
    ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
    gettimeofday(&stop_time, NULL);

    for(int i=0;i<20;i++)
    {
        printf("func  %d %f, %f ,%f\n", i, ((float*)outputs[0].buf)[3*i+0], ((float*)outputs[0].buf)[3*i+1], ((float*)outputs[0].buf)[3*i+2]);
    }
    
    for(int i=0;i<21;i++)
    {
                // cout<<i<<": "<<landmarkinfo.landmark[i].x<<landmarkinfo.landmark[i].y<<endl;
        cv::Point p1(((float*)outputs[0].buf)[3*i+0]/224.0*640,((float*)outputs[0].buf)[3*i+1]/224.0*480);
        cv::circle(img, p1, 1, cv::Scalar(0, 255, 0), -1); 
    }
    cv::imshow("11",img);
    cv::waitKey(0);

    // decode();
    // selected_lane(m_decode_lane,200);
    // LeftRightGet(m_select_lane);

    // for (int i = 0; i < m_final_lane_with_type.size(); i++){
    //     final_lane.push_back(m_final_lane_with_type[i]);
    // }

    // m_decode_lane.clear();
    // m_select_lane.clear();
    // m_final_lane_with_type.clear();

   printf("once run use %f ms\n", (__get_us(stop_time) - __get_us(start_time)) / 1000);

    return 0;
}


int HandLandmark::decode()
{
    // for (int i = 0; i < 20; ++i) {
    //     MNN_PRINT("func  %d %f, %f\n", i, outputTensors_host[0]->host<float>()[2*i+0], outputTensors_host[0]->host<float>()[2*i+1]);
    // }
    for(int i=0;i<20;i++)
    {
        printf("func  %d %f, %f\n", i, ((float*)outputs[0].buf)[2*i+0], ((float*)outputs[0].buf)[2*i+1]);
    }

    
    float score0;
    float score1;

    float down_anchor_lane[72];
    float up_anchor_lane[73];


    int feature_h=18;
    int feature_w=32;
    int points_per_anchor = 4;   // 72/18 = 4
    int points_per_line = 72;   // 72/18 = 4
    float interval = 288.0 / 72.0;

    for (int j=0;j<output_attrs[0].dims[1];j++)
    {
        score0=((float*)outputs[0].buf)[2*j+0];
        score1=((float*)outputs[0].buf)[2*j+1];

        if(score1>0.6)
        {
            memcpy(down_anchor_lane, &(((float*)outputs[1].buf)[145*j+0]), points_per_line  * sizeof(float));
            memcpy(up_anchor_lane, &(((float*)outputs[1].buf)[145*j+72]), (points_per_line +1) * sizeof(float));
            for (int k = 0;k < 72; ++k) {
                printf("func2  %d %d %f, %f ,%f\n",j, k,score1, down_anchor_lane[k], up_anchor_lane[k]);
            }
            float relative_end_pos = up_anchor_lane[0];
            int h=j/feature_w;
            int w=j%feature_w;
            int anchor_y_pos = int((feature_h - 1 - h) * points_per_anchor);
            float anchor_center_x = (1.0 * w + 0.5) * 16;
            float anchor_center_y = (1.0 * h + 0.5) * 16;
            int end_pos = anchor_y_pos;
            int start_pos = anchor_y_pos;
            std::vector<cv::Point> Lane;


            for (int i = 0; i < points_per_line; i++)
            {
                if (i >= relative_end_pos || anchor_y_pos + i >= points_per_line)
                    break;
                int rela_x = up_anchor_lane[1 + i];
                float abs_x = anchor_center_x + rela_x;
                float abs_y = height - 1 - (anchor_y_pos + i) * interval;
                cv::Point p;
                p.x = abs_x;
                p.y = abs_y;
                end_pos = anchor_y_pos + i + 1;
                Lane.push_back(p);
            }

            for (int i = 0; i < anchor_y_pos; i++)
            {
                int rela_x = down_anchor_lane[i];
                float abs_x = anchor_center_x + rela_x;
                float abs_y = height - 1 - (anchor_y_pos - 1 - i) * interval;
                cv::Point p;
                p.x = abs_x;
                p.y = abs_y;
                start_pos = anchor_y_pos - 1 - i;
                Lane.push_back(p);
            }
            if (Lane.size() >= 2)
            {
                sort(Lane.begin(), Lane.end(), LessSort_point);
                lane_DECODE a;
                a.prob = score1;
                a.start_pos = start_pos;
                a.end_pos = end_pos;
                a.Lane = Lane;
                a.ax = anchor_center_x;
                a.ax = anchor_center_y;
                m_decode_lane.push_back(a);
            }
            //printf("index is %d,prob is %f,  start_pos is %d,  end_pos is  %d, anchor_center_x is %f, anchor_center_y is %f \n", index, score1, start_pos, end_pos, anchor_center_x, anchor_center_y);
        }
    }


   
   

    return 0;

}



float HandLandmark::calc_err_dis_with_pos(lane_DECODE L_1, lane_DECODE L_2) // ���������ߵľ���
{
    int max_start_pos = max(L_1.start_pos, L_2.start_pos);
    int min_end_pos = min(L_1.end_pos, L_2.end_pos);
    if (min_end_pos <= max_start_pos || max_start_pos < 0 || min_end_pos < 1)
        return 10e6;
    std::vector<cv::Point> pts1 = L_1.Lane;
    std::vector<cv::Point> pts2 = L_2.Lane;
    float dis = 0.0;
    for (int i = max_start_pos; i < min_end_pos; i++) {
        dis += abs(pts1[i - L_1.start_pos].x - pts2[i - L_2.start_pos].x);
    }
    dis = dis / (min_end_pos - max_start_pos);
    float dis_start = abs(L_1.Lane[max_start_pos - L_1.start_pos].x - L_2.Lane[max_start_pos - L_2.start_pos].x);
    dis = max(dis, dis_start);
    float dis_end = abs(L_1.Lane[min_end_pos - 1 - L_1.start_pos].x - L_2.Lane[min_end_pos - 1 - L_2.start_pos].x);
    dis = max(dis, dis_end);
    dis = max(dis_start, dis_end);
    //cout << "dis is " << dis << "\n";
    return dis;
}

int HandLandmark::selected_lane(std::vector<lane_DECODE> ALL_LANE, int thresh)
{
    // std::vector<lane_DECODE> save_LANE = {};
    sort(ALL_LANE.begin(), ALL_LANE.end(), LessSort);

    int NumLane = ALL_LANE.size();
    if (NumLane == 0)
    {
        return -1;
    }

    bool selected[ALL_LANE.size()] = { false };

    for (int i = 0; i < ALL_LANE.size() - 1; i++)
    {
        if (selected[i])
            continue;
        m_select_lane.push_back(ALL_LANE[i]);
        selected[i] = true;
        for (int j = i + 1; j < ALL_LANE.size() - 1; j++)
        {
            float dis = calc_err_dis_with_pos(ALL_LANE[i], ALL_LANE[j]);

            if (dis <= thresh) {
                selected[j] = true;
            }
        }
    }
    return 0;
}

void HandLandmark::LeftRightGet(std::vector<lane_DECODE>& final_lane)
{
    float sx1 = float(img_width) / float(width);
    float sy1 = float(img_height) / float(height);
    std::vector<lane_DECODE> right;
    std::vector<lane_DECODE> left;
    for (int i = 0; i < final_lane.size(); i++) {
        for (int j = 0; j < final_lane[i].Lane.size(); j++) {
            float px = final_lane[i].Lane[j].x * sx1;
            float py = final_lane[i].Lane[j].y * sy1;
            if (px > 0 && px < 1280) {
                if (px - 640 < 0) {
                    final_lane[i].dis = abs(px - 640);
                    left.push_back(final_lane[i]);
                    break;
                }
                if (px - 640 >= 0) {
                    final_lane[i].dis = px - 640;
                    right.push_back(final_lane[i]);
                    break;
                }
            }
        }
    }
    if (left.size() > 0)
    {
        sort(left.begin(), left.end(), moreSort);
        for (int i = 0; i < left.size(); i++) {
            if (i == 0) {
                left[i].LeftRightType = -1;
            }
            else
            {
                left[i].LeftRightType = -2;
            }
            m_final_lane_with_type.push_back(left[i]);
        }
    }

    if (right.size() > 0) {
        sort(right.begin(), right.end(), moreSort);
        for (int i = 0; i < right.size(); i++) {
            if (i == 0) {
                right[i].LeftRightType = 1;
            }
            else
            {
                right[i].LeftRightType = 2;
            }
            m_final_lane_with_type.push_back(right[i]);
        }
    }
}

HandLandmark::~HandLandmark() {
    // if (net!=nullptr){
    //     net->releaseModel();
    //     net->releaseSession(session);
    //     for (int i = 0; i < input_blob_names.size(); i++) {
    //         // delete inputTensors[i];
    //         delete inputTensors_host[i];
    //     }
    //     for (int i = 0; i < output_blob_names.size(); i++) {
    //         // delete outputTensors[i];
    //         delete outputTensors_host[i];
    //     }
    //     inputTensors.clear();
    //     inputTensors_host.clear();
    //     outputTensors.clear();
    //     outputTensors_host.clear();
    //     input_blob_names.clear();
    //     output_blob_names.clear();
    //     m_decode_lane.clear();
    //     m_select_lane.clear();
    //     m_final_lane_with_type.clear();
    // }
   
}

