#include <iostream>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <memory>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <cstring>

#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs/legacy/constants_c.h"
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>



#include "Object.h"


using namespace std;




ObjectInfo::ObjectInfo(FaceTracker *tracker)
{
    m_tracker=tracker;
    m_tracker_id=tracker->m_id;
    emptyInfo();
    m_tracker->m_have_recognize=false;
    m_tracker->m_have_show=false;
    m_max_h=10;
    m_max_w=10;
    memset(m_jpegData, 0, sizeof(m_jpegData));
    m_jpegLenth=0;
    m_have_update_image=false;
    m_send_ntimes=0;
    
}

void ObjectInfo::emptyInfo()
{

}


void ObjectInfo::Update(FaceTracker *tracker)
{
    m_tracker=tracker;
    
    // m_tracker_id=tracker->m_id;
    // if(m_tracker->have_recognize && m_tracker->m_state<=1)
    // {
    //     emptyInfo();
    //     have_recognize=false;
    // }
}

void ObjectInfo::empty_best_image()
{

    m_max_h=10;
    m_max_w=10;

    memset(m_jpegData, 0, sizeof(m_jpegData));
    m_jpegLenth=0;
    m_have_update_image=false;
}


bool ObjectInfo::Update_best_image()
{
    
    if(m_tracker->m_state>=1 && m_tracker->m_height > m_max_h && m_tracker->m_width > m_max_w)
    {
        m_max_h=m_tracker->m_height;
        m_max_w=m_tracker->m_width;

        if(m_max_h>m_history_max_h)
        {
            m_history_max_h=m_max_h;
        }
        if(m_max_w>m_history_max_w)
        {
            m_history_max_w=m_max_w;
        }


        m_have_update_image=true;
        
        return true;
    }
    

    return false;
}


bool ObjectInfo::do_send()
{
    
    if(m_tracker->m_state>=1 && !m_tracker->m_have_recognize && m_have_update_image)
    {        
        m_send_ntimes=m_send_ntimes+1;
        m_send_hits=m_tracker->hits;
        m_have_update_image=false;
        m_tracker->m_have_recognize=true;
        return true;
    }
    int n_per=10;

    if(m_send_ntimes>10)
    {
        n_per=20;
    }

    if(m_send_ntimes>30)
    {
        n_per=30;
    }

    if(m_send_ntimes>60)
    {
        n_per=40;
    }

    if(m_send_ntimes>100)
    {
        n_per=50;
    }

    
    if(m_tracker->m_state>=1 && (m_tracker->hits+5)%n_per==0 && m_have_update_image)
    {
        m_send_hits=m_tracker->hits;
        m_send_ntimes=m_send_ntimes+1;
        m_have_update_image=false;
        // m_tracker->hits=m_tracker->hits+1;
        return true;
    }


    // if(m_tracker->m_state==0 && m_tracker->m_have_recognize && m_tracker->hits>m_send_hits && m_have_update_image && m_max_h>=m_history_max_h && m_max_w>=m_history_max_w)
    // {
    //     m_send_hits=m_tracker->hits;
    //     m_send_ntimes=m_send_ntimes+1;
    //     m_have_update_image=false;
    //     return true;
    // }


    return false;
}


bool ObjectInfo::do_tlvshow()
{
    
    if(m_tracker->m_state>=1 && !m_tracker->m_have_show && m_have_update_image)
    {
        m_tracker->m_have_show=true;
        m_show_ntimes=m_show_ntimes+1;
        m_show_hits=m_tracker->hits;
        m_have_update_image=false;
        return true;
    }


    // int n_per=10;

    // if(m_show_ntimes>10)
    // {
    //     n_per=20;
    // }

    // if(m_show_ntimes>30)
    // {
    //     n_per=30;
    // }

    // if(m_show_ntimes>60)
    // {
    //     n_per=40;
    // }

    // if(m_show_ntimes>100)
    // {
    //     n_per=50;
    // }

    int n_per=30;

    if(m_tracker->m_state>=1 && (m_tracker->hits)%n_per==0 && m_have_update_image){
        
        m_show_ntimes=m_show_ntimes+1;
        m_show_hits=m_tracker->hits;
        m_have_update_image=false;

        // m_tracker->hits=m_tracker->hits+1;
        return true;
    }

    // if(m_tracker->m_state==0 && m_tracker->m_have_show && m_tracker->hits>m_show_hits && m_have_update_image && m_max_h>=m_history_max_h && m_max_w>=m_history_max_w)
    // {
    //     printf("m_max_h=%f(m_history_max_h=%f); %f %f \n",m_max_h,m_history_max_h,m_max_w,m_history_max_w);
    //     m_send_ntimes=m_send_ntimes+1;
    //     m_show_hits=m_tracker->hits;
    //     m_have_update_image=false;
    //     return true;
    // }

    return false;
}




bool ObjectInfo::do_recognize()
{

    if(!m_tracker->m_have_recognize && m_tracker->m_state>=1 && !m_need_compare)
    {
        // m_recognizeinfo.recognize_id=100;
        m_tracker->m_have_recognize=true;
        return true;
    }
    
    // if(m_tracker->hits%50==0 && m_tracker->m_state==4 && !m_need_compare)
    // {
    //     m_tracker->hits=m_tracker->hits+1;
    //     // m_recognizeinfo.recognize_id=100;
    //     // m_tracker->have_recognize=true;
    //     return true;
    // }


    if(m_tracker->hits%50==0 && m_tracker->m_state>=1 && !m_need_compare)
    {
        m_tracker->hits=m_tracker->hits+1;
        // m_recognizeinfo.recognize_id=100;
        // m_tracker->have_recognize=true;
        return true;
    }
  
    return false;
}


void ObjectInfo::get_feature(SDC_FEATURE_INFO_S &FeatureInfo)
{

    for (int i = 0; i <512 ; i++)
    {
        m_feature[i]=FeatureInfo.af32feature[i];
        printf("m_feature[i %d] : %f\n",i,m_feature[i]);
    }

    m_need_compare=true;
}



void ObjectInfo::compare_feature()
{
    // m_pstFeatureInfo=pstFeatureInfo;
    int ret1 = snprintf(m_name, sizeof(m_name), "%lld",m_tracker_id);
    // strcpy_s(m_name, sizeof(m_name),"KingJames");

    m_need_compare=false;

}



