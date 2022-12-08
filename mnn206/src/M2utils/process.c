#include "process.h"


using namespace M2;

inline int ImageArrayOffset(int height, int width, int h, int w, int c) {
  return (h * width + w) * 3 + c;
}

inline void Crop(int input_height, int input_width, int start_h, int start_w,
                 int crop_height, int crop_width, const uint8_t* input_data,
                 std::vector<float>* output_data) {
  const int stop_h = start_h + crop_height;
  const int stop_w = start_w + crop_width;

  for (int in_h = start_h; in_h < stop_h; ++in_h) {
    for (int in_w = start_w; in_w < stop_w; ++in_w) {
      for (int c = 0; c < 3; ++c) {
        output_data->push_back(static_cast<float>(input_data[ImageArrayOffset(
            input_height, input_width, in_h, in_w, c)]));
      }
    }
  }
}


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

// STRU_RectInfo_T GetGoodFace(STRU_RectInfo_T rectinfo, std::vector<int> &indexes)//获取面积最大的人脸
// {
// 	STRU_RectInfo_T temp;
// 	for (int i = 0;i < rectinfo.nNum;i++)
// 	{
// 		int label = rectinfo.labels[i].label;
// 		if (label == 1)
// 		{
// 			temp.rects[temp.nFaceNum] = rectinfo.rects[i];
// 			temp.labels[temp.nFaceNum] = rectinfo.labels[i];
//             temp.nFaceNum =temp.nFaceNum+1;
//             indexes.push_back(i);
// 		}
// 	}
// 	return temp;
// }
