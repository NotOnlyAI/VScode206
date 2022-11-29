
#include <algorithm>
#include "TrackManager.h"

using namespace std;


TrackManager::TrackManager(int _max_history_size)
{
    max_history_size = _max_history_size;
    iter = 0;
}

TrackManager::~TrackManager()
{
    list<FaceTracker*>::iterator tracker_iter = m_trackerList.begin();
    for (; tracker_iter != m_trackerList.end(); tracker_iter++) {
        delete *tracker_iter;
    }
    iter = 0;
    m_trackerList.clear();
}

bool TrackManager::add(uint64_t tracker_id, Detection &initDetection,int detection_idx)
{
    FaceTracker* tracker = new FaceTracker(tracker_id, initDetection,detection_idx);
    m_trackerList.push_back(tracker);
    return true;
}

bool TrackManager::track()
{
    for (auto tracker_iter = m_trackerList.begin(); tracker_iter != m_trackerList.end();)
	{
        (*tracker_iter)->predict();
        if ((*tracker_iter)->lost_too_long(max_history_size))
		{
            delete *tracker_iter;
			tracker_iter = m_trackerList.erase(tracker_iter);
		}
		else
		{
			tracker_iter++;
		}
    }
    return true;
}

float overlap_rate(const Rect2f& a, const Rect2f& b)
{
    float x1 = max(a.x, b.x);
    float y1 = max(a.y, b.y);
    float x2 = min(a.x + a.width, b.x + b.width);
    float y2 = min(a.y + a.height, b.y + b.height);
    float dx = max(0.0f, x2 - x1);
    float dy = max(0.0f, y2 - y1);

    return (dx * dy) / max(a.width * a.height, b.width * b.height);
}


bool cmp_time_since_update(FaceTracker *s1,FaceTracker *s2)//返回值不是true的，就交换 
{
    return s1->time_since_update<s2->time_since_update;
}


bool cmp_iou(FaceTracker *s1,FaceTracker *s2)//返回值不是true的，就交换 
{
    return s1->m_detection_iou>s2->m_detection_iou;
}




bool TrackManager::track(std::vector<Detection> & detections)
{
    track();

    // printf("detections=%d\n",detections.size());

    //1:记录匹配信息
    std::vector< std::vector<FaceTracker *> > match_tracker_ptr(detections.size());
    std::vector< std::vector<float> > match_tracker_iou(detections.size());

    std::vector< std::vector<int> > match_detection_idx(m_trackerList.size());
    std::vector< std::vector<float> > match_detection_iou(m_trackerList.size());
    for (int i = 0; i < detections.size(); i++)
    {
        const Rect2f bbox1(detections.at(i).m_xmin,detections.at(i).m_ymin,detections.at(i).m_width,detections.at(i).m_height);
        int j=0;
        for (auto tracker_iter:m_trackerList)
        {
            const Rect2f bbox2 = tracker_iter->rect();
            float iou=overlap_rate(bbox1, bbox2);
            if(iou>0.2)
            {   
                match_tracker_ptr.at(i).push_back(tracker_iter);
                match_tracker_iou.at(i).push_back(iou);
                match_detection_idx.at(j).push_back(i);
                match_detection_iou.at(j).push_back(iou);
            }
            j++;
        }
    }

    //2:处理多个detection匹配到一个tracker情况,取最大iou的detetction
    int j=0;
    for (auto tracker_iter:m_trackerList)
    {
        if(match_detection_idx.at(j).empty())
        { 
            tracker_iter->m_detection_idx=-1;
            tracker_iter->m_detection_iou=0;
        }
        else if(match_detection_idx.at(j).size()==1)
        {
            tracker_iter->m_detection_idx=match_detection_idx.at(j).at(0);
            tracker_iter->m_detection_iou=match_detection_iou.at(j).at(0);
        }
        else
        {
            std::vector<float>::iterator max_iou = std::max_element(std::begin(match_detection_iou.at(j)), std::end(match_detection_iou.at(j)));
            int max_pos=std::distance(std::begin(match_detection_iou.at(j)), max_iou );
            int detection_idx=match_detection_idx.at(j).at(max_pos);

            tracker_iter->m_detection_idx=detection_idx;
            tracker_iter->m_detection_iou=*max_iou;

            if(match_tracker_ptr.at(detection_idx).size()>=2)
            {
                if(tracker_iter->m_state>=2)
                {
                    tracker_iter->m_state=1;
                    tracker_iter->m_have_recognize=false;
                    // tracker_iter->m_have_show=false;
                }
            }
        }
        j++;
    }

    for (int i = 0; i < detections.size(); i++)
    {
        
        if(match_tracker_ptr.at(i).empty())
        {
            add(iter++,detections.at(i),i);
            // printf("add i=%d track_id=%ld\n",i,iter); 
        }
        else
        {
            std::vector<FaceTracker *> good_tracker;
            for(int j=0;j<match_tracker_ptr.at(i).size();j++)
            {
                if(match_tracker_ptr.at(i).at(j)->m_detection_idx==i)
                {
                    good_tracker.push_back(match_tracker_ptr.at(i).at(j));
                }
            }
            if(good_tracker.empty())
            {
                add(iter++,detections.at(i),i);
                // printf("add i=%d track_id=%ld\n",i,iter); 
            }
            else
            {
                if(good_tracker.size()==1)
                {
                    good_tracker.at(0)->update(detections.at(i));
                    // printf("update i=%d,track_id=%ld\n",i,good_tracker.at(0)->m_id);
                    // if(good_tracker.at(0)->m_state>=2)
                    // {
                    //     if(detections.at(i).good_quality()){good_tracker.at(0)->m_state=4;}
                    // }    
                    
                }
                else
                {
                    std::sort(good_tracker.begin(),good_tracker.end(),cmp_time_since_update);
                    std::vector<FaceTracker *> better_tracker;
                    better_tracker.push_back(good_tracker.at(0));
                    
                    for(int k=1;k<good_tracker.size();k++)
                    {
                        if(good_tracker.at(k)->time_since_update>good_tracker.at(0)->time_since_update){break;}
                        better_tracker.push_back(good_tracker.at(k));
                    }
                    if(better_tracker.size()==1)
                    {
                        better_tracker.at(0)->update(detections.at(i));
                        // printf("update i=%d,track_id=%ld\n",i,better_tracker.at(0)->m_id);
                        // if(better_tracker.at(0)->m_state>=2)
                        // {
                        //     if(detections.at(i).good_quality()){better_tracker.at(0)->m_state=4;}
                        // }
                    }
                    else
                    {
                        std::sort(better_tracker.begin(),better_tracker.end(),cmp_iou);
                        better_tracker.at(0)->update(detections.at(i));
                        // printf("update i=%d,track_id=%ld\n",i,better_tracker.at(0)->m_id);
                        if(better_tracker.at(0)->m_state>=2)
                        {
                            better_tracker.at(0)->m_state=1;
                            better_tracker.at(0)->m_have_recognize=false;
                            // better_tracker.at(0)->m_have_show=false;
                        }
                    }
                } 
            }
        }
    }

    return true;
}
