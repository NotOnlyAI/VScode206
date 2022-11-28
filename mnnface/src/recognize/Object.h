
#ifndef __Socket_Server_H__
#define __Socket_Server_H__

#include <string>
#include <memory>
#include <ctime>

#include "FaceTracker.h"
#include "sample_comm_nnie.h"



class ObjectInfo
{
    public:
        ObjectInfo(FaceTracker *tracker);
        void Update(FaceTracker *tracker);
        void emptyInfo();
        void get_feature(SDC_FEATURE_INFO_S &FeatureInfo);
        void compare_feature();
        bool do_recognize();
        bool do_send();
        bool do_tlvshow();
        bool Update_best_image();
        void empty_best_image();


        FaceTracker *m_tracker;
        bool m_need_compare=false;
        bool m_have_update_image=false;



        float m_feature[512];
        char m_name[30]="None";
        uint64_t m_tracker_id;

        uint64_t m_send_ntimes;
        uint64_t m_send_hits;
        uint64_t m_show_ntimes;
        uint64_t m_show_hits;

        float m_max_w;
        float m_max_h;
        float m_history_max_w=10;
        float m_history_max_h=10;

        

        uint8_t m_jpegData[400000];
        int m_jpegLenth;

        unsigned int face_x;
        unsigned int face_y;
        unsigned int face_w;
        unsigned int face_h;

        unsigned int face_x0=0;
        unsigned int face_y0=0;
        unsigned int face_x1=0;
        unsigned int face_y1=0;
        unsigned int face_x2=0;
        unsigned int face_y2=0;
        unsigned int face_x3=0;
        unsigned int face_y3=0;
        unsigned int face_x4=0;
        unsigned int face_y4=0;

};

#endif /* __Socket_Server_H__ */
