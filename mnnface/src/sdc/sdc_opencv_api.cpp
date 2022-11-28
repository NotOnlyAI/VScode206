// /******************************************************************************

//                   ��Ȩ���� (C), 2019-2029, SDC OS ��Դ����С������

//  ******************************************************************************
//   �� �� ��   : opencv_api.cpp
//   �� �� ��   : ����
//   ��  s30001871171
//   ��������   : 2020��7��4��
//   ����޸�   :
//   ��������   : opencv_api������
//   �����б�   :
//   �޸���ʷ   :
//   1.��    ��   : 2020��7��4��
//     ��  s30001871171
//     �޸�����   : �����ļ�

// ******************************************************************************/

#include "sdc_opencv_api.h"


int yuvFrame2Mat(sdc_yuv_frame_s &yuvFrame,cv::Mat &dst)
{
    if(yuvFrame.addr_virt == 0){
    printf("yuvFrame.addr_virt is null");
    return -1;
    }

    uint32_t w = yuvFrame.width * 2 / 2;
	uint32_t h = yuvFrame.height * 2 / 2;
    uint32_t imageLength = w * h * 3 / 2;
	cv::Mat input_yuv;
	input_yuv.create((int32_t)h * 3 / 2 , (int32_t)w, CV_8UC1);
	memcpy(input_yuv.data, (char *)(uintptr_t)yuvFrame.addr_virt, imageLength);
	
	dst.create(h , w, CV_8UC3);
	cv::cvtColor(input_yuv, dst, cv::COLOR_YUV2BGR_NV21);
    // cv::imwrite("yuv2BGR.jpg",dst);
    return 0;
}


void TransFormMat2Yuv(cv::Mat &img, unsigned char *yuv_nv21)
{
	int w = img.cols;
	int h = img.rows;
    int wh =  w * h;
    int uv_len = wh / 2;
    uint8_t *yuv = (uint8_t *)malloc(uv_len);                   // �洢CV_BGR2YUV_I420����

    int imageLength = wh * 3 / 2;                               //  3/2 �洢nv21���� 
    cv::Mat out;
	cv::cvtColor(img, out, cv::COLOR_BGR2YUV_IYUV);             // BGR�ռ�ת��ΪCV_BGR2YUV_I420 
	memcpy(yuv_nv21, out.data, wh*sizeof(unsigned char));       // ��ʱyuv_nv21�д洢����CV_BGR2YUV_I420�������� 
	memcpy(yuv, out.data + wh, uv_len*sizeof(unsigned char));   // ��ʱyuv�д洢����CV_BGR2YUV_I420�������� 
	int num = 0;                                                // ��u��v�����ļ��� 
	for (int j = wh; j != imageLength; j += 2) {
		yuv_nv21[j + 1] = yuv[num];                             // ��yuv_nv21��u�������и�ֵ 
		yuv_nv21[j] = yuv[wh / 4 + num];                        // ��yuv_nv21��v�������и�ֵ 
		++num;
	}
    free(yuv);
}

int ProcessImgPath(const char *imgPath, int &w, int &h, uint8_t **pyuv)
{
	cv::Mat img = cv::imread(imgPath);
    if (img.empty()) {
        return -1;
    }
    if(img.cols % 16 != 0 || img.rows % 16 != 0) {
        w = img.cols / 16 * 16;               //  ����16�ֽڶ���
        h = img.rows / 16 * 16;               //  ����16�ֽڶ���
        cv::resize(img, img, cv::Size(w, h)); 
    } else {
        w = img.cols;
        h = img.rows;
    }

	*pyuv = (uint8_t *) malloc(sizeof(uint8_t) * w * h * 3 / 2);
    TransFormMat2Yuv(img,  *pyuv);
	return OK;
}


// #define CV_FONT_HERSHEY_COMPLEX         3

// void SDC_Mat2RGB(cv::Mat &inputmat, VW_YUV_FRAME_S *pstRGBFrameData)
// {
//     pstRGBFrameData->enPixelFormat = PIXEL_FORMAT_RGB_888;
//     pstRGBFrameData->uWidth = inputmat.cols;
//     pstRGBFrameData->uHeight =  inputmat.rows;
//     pstRGBFrameData->uStride[0] = inputmat.cols;
//     pstRGBFrameData->uStride[1] = inputmat.cols;
//     pstRGBFrameData->uStride[2] = inputmat.cols;
//     pstRGBFrameData->uFrmSize = inputmat.cols * inputmat.rows * 3;
//     pstRGBFrameData->uPoolId = 0;
//     pstRGBFrameData->uVbBlk = 0;
// }

// void SetSdcYuvFrame(int src_w, int src_h, sdc_yuv_frame_s *src_yuv)
// {
//     src_yuv->format = SDC_YVU_420SP;
//     src_yuv->width  = src_w;
//     src_yuv->height = src_h;
//     src_yuv->stride = src_w;
//     src_yuv->size   = src_w * src_h * 3 / 2 ; 
// }

// // ��ȡletterbox��ʽ�����ų߶ȣ����룺 ԭʼ���ݵĿ��ߺ�ģ������Ŀ���
// float GetScaleRatio (int src_w, int src_h,int forwardsize_w, int forwardsize_h){
//     float w_ratio = 1.0f * forwardsize_w / src_w;
// 	float h_ratio = 1.0f * forwardsize_h / src_h;
//     return w_ratio > h_ratio ? h_ratio : w_ratio;
// }

// void pre_process_letterbox_Mat(const cv::Mat &img, int forwardsize_w, int forwardsize_h,cv::Mat &outputMat)
// {
// 	outputMat = cv::Mat::zeros( cv::Size(forwardsize_w, forwardsize_h),CV_32F);
// 	float w_ratio = 1.0f * outputMat.cols / img.cols;
// 	float h_ratio = 1.0f * outputMat.rows / img.rows;
// 	cv::Mat img_resize;
// 	float min_ratio = w_ratio > h_ratio ? h_ratio : w_ratio;
// 	int new_w = (int)(img.cols* min_ratio);
// 	int new_h = (int)(img.rows* min_ratio);
// 	cv::resize(img,img_resize, cv::Size(new_w, new_h));
// 	                                   //int top, int bottom, int left, int right,
// 	cv::copyMakeBorder(img_resize, outputMat, 0, outputMat.rows-img_resize.rows, 0, outputMat.cols-img_resize.cols,cv::BORDER_CONSTANT);
//     // split(zeo, *input_channels);
// }

void BGR2yuv_nv21(cv::Mat& img, unsigned char * pyuv)
{
	int imageLength = img.cols * img.rows * 3 / 2; 
    int wh =  img.cols * img.rows;
	int w = img.cols / 2 * 2; // bgr תyuvʱ����Ҫ������ż��
	int h = img.rows / 2 * 2;
	
    int uv_len = w * h / 2;
	unsigned char *yuv = new unsigned char[uv_len];             /* �洢CV_BGR2YUV_I420���� */
	cv::Mat out;
	out.create(h * 3 / 2 , w, CV_8UC2);

    cv::cvtColor(img, out, cv::COLOR_BGR2YUV_IYUV);                 /* BGR�ռ�ת��ΪCV_BGR2YUV_I420 */
	memcpy(pyuv, out.data, wh * sizeof(unsigned char));               /* ��ʱyuv_nv21�д洢����CV_BGR2YUV_I420�������� */
	memcpy(yuv, out.data + wh, uv_len*sizeof(unsigned char));         /* ��ʱyuv�д洢����CV_BGR2YUV_I420�������� */
	int num = 0;  /* ��u��v�����ļ��� */
	for (int j = w * h; j != imageLength; j += 2) {
		pyuv[j + 1] = yuv[num];                          /* ��yuv_nv21��u�������и�ֵ */
		pyuv[j] = yuv[w * h / 4 + num];                 /* ��yuv_nv21��v�������и�ֵ */
		++num;
	}
	delete yuv;
}

// void SaveYUV(int resize_w, int resize_h, unsigned char * pyuv)
// {
// 	/* ����nv21��ʽ��yuv420 */
// 	FILE *fp = NULL;
// 	char buffer[50];
// 	snprintf(buffer, sizeof(buffer), "crop_%dW_%dH.yuv", resize_w, resize_h);
// 	fp = fopen(buffer, "wb+");
// 	if (!fp)
// 	{
// 		printf("file does not exist\n");
// 	}
// 	fwrite(pyuv, 1, sizeof(char)* resize_w * resize_h * 3 / 2, fp);
// 	fclose(fp);
// 	fp = NULL;
// }

// bool YUVResize(sdc_yuv_frame_s &yuvFrame, int resize_w, int resize_h, unsigned char * pyuv)
// {
// 	bool res = true;
// 	uint32_t w = yuvFrame.width * 2 / 2;
// 	uint32_t h = yuvFrame.height * 2 / 2;
// 	uint32_t forward_w = resize_w * 16 / 16;
//     uint32_t forward_h = resize_h * 16 / 16;

//     if(w != yuvFrame.width || h != yuvFrame.height){
//         LOG_ERROR("input resize not proper, should could devide by 2\n");
//         return false;
//     }
//     uint32_t imageLength = w * h * 3 / 2;
// 	cv::Mat input_yuv;
// 	input_yuv.create((int32_t)h * 3 / 2 , (int32_t)w, CV_8UC1);
// 	memcpy(input_yuv.data, (char *)(uintptr_t)yuvFrame.addr_virt, imageLength);
// 	cv::Mat dst;
// 	dst.create(h , w, CV_8UC3);
// 	cv::cvtColor(input_yuv, dst, cv::COLOR_YUV2BGR_NV21);
//     cv::imwrite("dst2.jpg",dst);
//     cv::Mat leboxMat;
//     pre_process_letterbox_Mat(dst,forward_w,forward_h,leboxMat);

//     BGR2yuv_nv21(leboxMat,pyuv);
// 	return res;
// }

// void TransFormJpg2Yuv(cv::Mat &img, unsigned char *yuv_nv21)
// {
// 	int w = img.cols;
// 	int h = img.rows;
//     int wh =  w * h;
//     int uv_len = wh / 2;
//     uint8_t *yuv = (uint8_t *)malloc(uv_len);                   // �洢CV_BGR2YUV_I420����

//     int imageLength = wh * 3 / 2;                               //  3/2 �洢nv21���� 
//     cv::Mat out;
// 	cv::cvtColor(img, out, cv::COLOR_BGR2YUV_IYUV);             // BGR�ռ�ת��ΪCV_BGR2YUV_I420 
// 	memcpy(yuv_nv21, out.data, wh*sizeof(unsigned char));       // ��ʱyuv_nv21�д洢����CV_BGR2YUV_I420�������� 
// 	memcpy(yuv, out.data + wh, uv_len*sizeof(unsigned char));   // ��ʱyuv�д洢����CV_BGR2YUV_I420�������� 
// 	int num = 0;                                                // ��u��v�����ļ��� 
// 	for (int j = wh; j != imageLength; j += 2) {
// 		yuv_nv21[j + 1] = yuv[num];                             // ��yuv_nv21��u�������и�ֵ 
// 		yuv_nv21[j] = yuv[wh / 4 + num];                        // ��yuv_nv21��v�������и�ֵ 
// 		++num;
// 	}
//     free(yuv);
// }

// int TransYuv2Jpg(unsigned char *yuvAddr, int w, int h, const char *imgPath)
// {
//     int ret;
//     cv::Mat yuvMat;
//     cv::Mat rgbMat;
    
//     if ( yuvAddr == NULL ){
//         std::cout << "ptr is NULL" << std::endl;
//         return -1;
//     }

//     yuvMat.create(h * 3 / 2, w, CV_8UC1);
//     ret = memcpy_s(yuvMat.data, w * h * 3 / 2, yuvAddr, w * h * 3 / 2);
//     if( ret != EOK ){
//         printf("memcpy_s failed[%d]\n", ret);
//         return -1;
//     }

//     cv::cvtColor(yuvMat, rgbMat, CV_YUV2RGB_NV12);
//     std::vector<unsigned char> jpgData;
//     ret = cv::imencode(imgPath, rgbMat, jpgData); 
//     if (ret != 0 ){
//         printf("imencode failed[%d], jpgSize[%ld]\n", ret, jpgData.size());
//     }

//     std::ofstream file("out.jpg", std::ios::out | std::ofstream::binary);
//     std::copy(jpgData.begin(), jpgData.end(), std::ostreambuf_iterator<char>(file));
    
//     std::ofstream fout("vector.jpg");
//     for(auto const& x : jpgData){
//         fout << x ;
//     }

//     return ret;
// }


// int ProcessInputPath(const char *imgPath, int &w, int &h, uint8_t **pyuv)
// {
// 	cv::Mat img = cv::imread(imgPath);
//     if (img.empty()) {
//         return -1;
//     }
//     if(img.cols % 16 != 0 || img.rows % 16 != 0) {
//         w = img.cols / 16 * 16;               //  ����16�ֽڶ���
//         h = img.rows / 16 * 16;               //  ����16�ֽڶ���
//         cv::resize(img, img, cv::Size(w, h)); 
//     } else {
//         w = img.cols;
//         h = img.rows;
//     }

// 	*pyuv = (uint8_t *) malloc(sizeof(uint8_t) * w * h * 3 / 2);
//     TransFormJpg2Yuv(img,  *pyuv);
// 	return OK;
// }

// void DrawImgBoxes(cv::Mat &input, SDC_SSD_RESULT_S *stResult, float ratio)
// {
//     for(HI_U32 i = 0; i < stResult->numOfObject; i++) {
//         SDC_SSD_OBJECT_INFO_S infos = stResult->pObjInfo[i];
//         HI_FLOAT confi = infos.confidence;
//         cv::Point2i pt1((int)(infos.x_left/ratio),(int)(infos.y_top/ratio));
//         cv::Point2i pt2((int)(infos.x_right/ratio),(int)(infos.y_bottom/ratio));
//         cv::rectangle(input,pt1, pt2, cv::Scalar(255,0,0), 1, 1, 0);
//         char buffer[20];
//         snprintf(buffer, sizeof(buffer), "%.2f", confi);
//         cv::Point2i pt3(pt1.x - 10,pt1.y);
//         putText(input, buffer, pt1, CV_FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 0, 255), 1.5);   //��ɫ
//     }
// }

// void DrawImage(const char* imgPath, SDC_SSD_RESULT_S* stResult, int src_w, int src_h, int for_w,int for_h)
// {
//     cv::Mat mat = cv::imread(imgPath);
// 	if (mat.empty()) {
//         printf("Img is empty, pelease check the path!\t%s\n", imgPath);
//         return;
//     }
//     float ratio = GetScaleRatio(src_w, src_h, for_w, for_h);
//     DrawImgBoxes(mat, stResult, ratio);
//     cv::imwrite("/tmp/detresult.jpg", mat);
// }

// void readJpgData(const char* imgPath, std::vector<unsigned char>& jpgData){
//     cv::Mat rgbMat = cv::imread(imgPath);
//     if(rgbMat.empty()){
//         printf("img is empty");
//         return;
//     }
//     cv::imencode(imgPath, rgbMat, jpgData);
// }


int CropInGivenRect(sdc_yuv_frame_s &yuvFrame, int xmin,int ymin,int width,int height, std::vector<unsigned char>& jpgData)
{
    if(yuvFrame.addr_virt == 0){
        printf("yuvFrame.addr_virt is null");
        return -1;
    }

    cv::Rect inputRect(xmin,ymin,width,height);

	int32_t w = yuvFrame.width;
	int32_t h = yuvFrame.height;

    int32_t imageLength = w * h * 3 / 2;
	cv::Mat input_yuv;
	input_yuv.create((int32_t)h * 3 / 2 , (int32_t)w, CV_8UC1);

	memcpy(input_yuv.data, (char *)(uintptr_t)yuvFrame.addr_virt, imageLength);

    cv::Mat dst;
	dst.create(h , w, CV_8UC3);
	cv::cvtColor(input_yuv, dst, cv::COLOR_YUV2BGR_NV21);
    
    inputRect.x = xmin < 0 ? 0 : xmin;
    inputRect.y = ymin < 0 ? 0 : ymin ;
    inputRect.width = width > w ? w : width;
    inputRect.height = height > h ? h : height;

    cv::Mat rectDst = dst(inputRect);
    cv::imencode(".jpg", rectDst, jpgData);
    return 0;
}

