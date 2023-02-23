#include "image_utils_new.h"
#include <cstring>
#include <iostream>
#include<memory>
using namespace std;
using namespace M2;
template <typename T>
void CopyData( T *dst, const T *src, size_t _count )
{
    #if _MSC_VER >= 1600
    memcpy_s( dst, sizeof( T ) * _count, src, sizeof( T ) * _count );
    #else
    memcpy( dst, src, sizeof( T ) * _count );
    #endif
}


#ifdef FACE_CVSHOW
cv::Mat StructImage_to_CvImage(const STRU_ImgData_T &imagedata)
{
    if (imagedata.channel==3)
    {
    	cv::Mat image(cv::Size(imagedata.width, imagedata.height), CV_8UC3);
	    image.data =imagedata.data;
	    return image;
    }else
    {
    	cv::Mat image(cv::Size(imagedata.width, imagedata.height), CV_8UC1);
	    image.data =imagedata.data;
	    return image;
    }
}
#endif




// void Image_crop_v2( const STRU_ImgData_T &imgdata,STRU_ImgData_T &result, const STRU_Rect_T &rect )
// {

//     // Adjust rect
//     STRU_Rect_T fixed_rect = rect;
//     fixed_rect.width += fixed_rect.xmin;
//     fixed_rect.height += fixed_rect.ymin;
//     fixed_rect.xmin = max( 0, min( imgdata.width - 1, fixed_rect.xmin ) );
//     fixed_rect.ymin = max( 0, min( imgdata.height - 1, fixed_rect.ymin ) );
//     fixed_rect.width = max( 0, min( imgdata.width - 1, fixed_rect.width ) );
//     fixed_rect.height = max( 0, min( imgdata.height - 1, fixed_rect.height ) );
//     fixed_rect.width -= fixed_rect.xmin;
//     fixed_rect.height -= fixed_rect.ymin;

// //    cout<<fixed_rect.width<<endl;
// //    cout<<fixed_rect.height<<endl;
//     // crop image
//     result.width=fixed_rect.width;
//     result.height=fixed_rect.height;
//     result.depth= imgdata.depth;
// 	result.channel = imgdata.channel;
//     memset( result.data, 0, sizeof( unsigned char ) * result.width * result.height * result.channel );

//     const unsigned char* iter_in_ptr = &imgdata.data[fixed_rect.ymin * imgdata.width * imgdata.channel + fixed_rect.xmin * imgdata.channel];
//     int iter_in_step = imgdata.width * imgdata.channel;
//     int copy_size = fixed_rect.width * imgdata.channel;
//     int iter_size = fixed_rect.height;
//     unsigned char * iter_out_ptr = &result.data[max( 0, fixed_rect.ymin - rect.ymin ) * result.width * result.channel + max( 0, fixed_rect.xmin - rect.xmin ) * result.channel];
//     int iter_out_step = result.width * result.channel;

//     for( int i = 0; i < iter_size; ++i, iter_in_ptr += iter_in_step, iter_out_ptr += iter_out_step )
//     {
//         CopyData( iter_out_ptr, iter_in_ptr, copy_size );
//     }



//     return ;
// }



