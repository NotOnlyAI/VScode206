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


