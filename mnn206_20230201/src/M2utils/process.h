#ifndef FD_PROCESS_H_
#define FD_PROCESS_H_
#include <string>
#include <map>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <typeinfo>
#include <time.h>

#include "models206_typedef.h"

using namespace M2;

inline int ImageArrayOffset(int height, int width, int h, int w, int c);

inline void Crop(int input_height, int input_width, int start_h, int start_w,
                 int crop_height, int crop_width, const uint8_t* input_data,
                 std::vector<float>* output_data);



#endif