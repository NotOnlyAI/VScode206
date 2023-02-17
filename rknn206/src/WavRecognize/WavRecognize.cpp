//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#define clip(x, y) (x < 0 ? 0 : (x > y ? y : x))

#include "WavRecognize.hpp"
#include <opencv2/opencv.hpp>
#include "fbank.h"

using namespace std;
using namespace M2;
using namespace wenet;


float feats_last[3*80];
float feats_to_predict[67*80];
float output_subsampling_cache[1*96*256];


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

WavRecognize::WavRecognize()  {
    

}


int WavRecognize::InitModel(string model_path,
    rknn_context &ctx,rknn_input_output_num &io_num,
    rknn_tensor_attr *input_attrs,rknn_tensor_attr *output_attrs,
    rknn_input *inputs,rknn_output *outputs)
{
    printf("Loading mode...%s\n",model_path.c_str());
    int            model_data_size = 0;
    unsigned char* model_data      = load_model(model_path.c_str(), &model_data_size);

    
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


    for(int i=0;i<io_num.n_input;i++)
    {
        memset(inputs, 0, sizeof(inputs));
        inputs[i].index        = i;
        inputs[i].type         = RKNN_TENSOR_FLOAT32;
        inputs[i].size         = input_attrs[i].n_elems*4;
        inputs[i].fmt          = RKNN_TENSOR_NHWC;
        inputs[i].pass_through = 0;
    }


}






int WavRecognize::GetWavSignalsThread()
{
    std::string wav_path="DEV1.wav";
    WavReader wav_reader(wav_path);
    std::vector<float> cur_signal(wav_reader.data(), wav_reader.data()+1600);

    
    int index=0;
    int index_end;
    bool endflag=false;
    while(1)
    {
        if((index+1600)>=wav_reader.num_sample())
        {
            index_end=wav_reader.num_sample();
            endflag=true;
        }
        else{
            index_end=index+1600;
        }
        std::vector<float> cur_signal(wav_reader.data()+index, wav_reader.data()+index_end);
        index=index+1600;

        std::vector<float> signal;
        signal.insert(signal.end(),m_pre_signal.begin(),m_pre_signal.end());
        signal.insert(signal.end(),cur_signal.begin(),cur_signal.end());

        int rest_points = 0;
        std::vector<float> run_signal;
        if(rest_points!=0)
        {
            run_signal.insert(run_signal.end(),signal.begin(),signal.begin()-rest_points);
        }
        else
        {
            run_signal.insert(run_signal.end(),signal.begin(),signal.end());
        }

        int buf_sig_len = int((0.025 * 16000) + rest_points);
        m_pre_signal.clear();
        m_pre_signal.insert(m_pre_signal.end(),signal.end()-buf_sig_len,signal.end());

        
        std::vector<std::vector<float>> feats;
        int num_frames = m_fbank->Compute(run_signal, &feats);

        // for (size_t i = 0; i < feats.size(); ++i) 
        // {
        //     for(int j=0;j<80;j++)
        //     {
        //         std::cout<<j<<":"<<feats.at(i).at(j)<<std::endl;
        //     }
        // }
        // std::cout<<feats.size()<<std::endl;
        // std::cout<<m_feats_buffer.size()<<std::endl;
        
        if(num_frames<11)
        {
            int padding_len = 11 - feats.size();
            std::vector<float> zero_vector(80, 0);
            for(int i=0; i<padding_len; i++)
            {
                feats.push_back(zero_vector);
            }
            
        }
        if(feats.size()!=11){
            std::cout<<"wrong feats.size()"<<std::endl;
        }
        for(int i=1;i<feats.size();i++)
        {
            m_feats_buffer.push(feats.at(i));
        }
        
        if(m_feats_buffer.size()>5000*11)
        {
            m_feats_buffer.pop();
        }
        std::cout<<"index"<<index<<"  m_feats_buffer.size():"<<m_feats_buffer.size()<<std::endl;
        if(endflag)
        {
            break;
        }
        
    }

    

    
    // std::cout<<m_pre_signal.size()<<"::::"<<run_signal.size()<<std::endl;
    // std::cout<<num_frames<<":"<<feats.size()<<","<<feats.at(0).size()<<std::endl;
    // for (size_t i = 0; i < feats.size(); ++i) 
    // {
    //     for(int j=0;j<80;j++)
    //     {
    //         std::cout<<j<<":"<<feats.at(i).at(j)<<std::endl;
    //     }
    // }

    // extract_feat(cur_signal,pre_signal);
    return 0;
}



int WavRecognize::Init(int print_config,int modelType){
    
    std::shared_ptr<FeaturePipelineConfig> feature_config = std::make_shared<FeaturePipelineConfig>(80, 16000);
    m_feature_pipeline=std::make_shared<FeaturePipeline>(*feature_config);


    m_fbank.reset(new Fbank(80, 16000, 16000 / 1000 * 25,16000 / 1000 * 10));
    m_pre_signal.resize(400);
    for(int i=0;i<400;i++){m_pre_signal.at(i)=0;}


    std::ifstream fin;
    fin.open("words.txt", std::ios::in);
    if (!fin.is_open()) {std::cout << "无法找到这个文件！" << std::endl;}
    std::string buff;
    int c=0;
    while(getline(fin, buff))
    {
        std::string word="";
        for(int i = 0; i < buff.size(); i++) 
        { 
            if(buff[i]!=' ')
            {
	            //std::cout<<buff[i]<<std::endl;
	            word +=(buff[i]);
	        }
	        else 
            {
                break;
            }
        }
        // std::cout<<word<<std::endl;
        m_words_map.insert(make_pair(c, word));
        c++;
    }
    fin.close();
    std::cout<<"验证map："<<m_words_map[1720]<<std::endl;


    
    m_searcher.reset(new CtcPrefixBeamSearch(m_ctc_search_opt)); //context_graph==nllptr
    // std::cout<<"m_searcher"<<m_searcher.opt_.first_beam_size<<std::endl;
    string model_name="encoder_chunk_0-16.rknn";
    InitModel(model_name,encoder_ctx_0,encoder_io_num_0,encoder_input_attrs_0,encoder_output_attrs_0,encoder_inputs_0,encoder_outputs_0);
    string ctc_model_path_0="ctc_0-16.rknn";
    InitModel(ctc_model_path_0,ctc_ctx_0,ctc_io_num_0,ctc_input_attrs_0,ctc_output_attrs_0,ctc_inputs_0,ctc_outputs_0);


    string model_name1="encoder_chunk_16-32.rknn";
    InitModel(model_name1,encoder_ctx_1,encoder_io_num_1,encoder_input_attrs_1,encoder_output_attrs_1,encoder_inputs_1,encoder_outputs_1);
    string ctc_model_path_1="ctc_16-32.rknn";
    InitModel(ctc_model_path_1,ctc_ctx_1,ctc_io_num_1,ctc_input_attrs_1,ctc_output_attrs_1,ctc_inputs_1,ctc_outputs_1);

    string model_name2="encoder_chunk_32-48.rknn";
    InitModel(model_name2,encoder_ctx_2,encoder_io_num_2,encoder_input_attrs_2,encoder_output_attrs_2,encoder_inputs_2,encoder_outputs_2);
    string ctc_model_path_2="ctc_32-48.rknn";
    InitModel(ctc_model_path_2,ctc_ctx_2,ctc_io_num_2,ctc_input_attrs_2,ctc_output_attrs_2,ctc_inputs_2,ctc_outputs_2);

    string model_name3="encoder_chunk_48-64.rknn";
    InitModel(model_name3,encoder_ctx_3,encoder_io_num_3,encoder_input_attrs_3,encoder_output_attrs_3,encoder_inputs_3,encoder_outputs_3);
    string ctc_model_path_3="ctc_48-64.rknn";
    InitModel(ctc_model_path_3,ctc_ctx_3,ctc_io_num_3,ctc_input_attrs_3,ctc_output_attrs_3,ctc_inputs_3,ctc_outputs_3);

    string model_name4="encoder_chunk_64-80.rknn";
    InitModel(model_name4,encoder_ctx_4,encoder_io_num_4,encoder_input_attrs_4,encoder_output_attrs_4,encoder_inputs_4,encoder_outputs_4);
    string ctc_model_path_4="ctc_64-80.rknn";
    InitModel(ctc_model_path_4,ctc_ctx_4,ctc_io_num_4,ctc_input_attrs_4,ctc_output_attrs_4,ctc_inputs_4,ctc_outputs_4);


    string model_name5="encoder_chunk_80-96.rknn";
    InitModel(model_name5,encoder_ctx_5,encoder_io_num_5,encoder_input_attrs_5,encoder_output_attrs_5,encoder_inputs_5,encoder_outputs_5);
    string ctc_model_path_5="ctc_80-96.rknn";
    InitModel(ctc_model_path_5,ctc_ctx_5,ctc_io_num_5,ctc_input_attrs_5,ctc_output_attrs_5,ctc_inputs_5,ctc_outputs_5);

}

void WavRecognize::GetChunkThread()
{
    bool end_flag=false;
    const int feature_dim_ = 80;
    const int num_frames_ = 67;



    while(!end_flag)
    {
      //这里主要实现的是，读取一段音频，对音频进行每67个frame一次送入forward，
        std::vector<std::vector<float>> chunk_feats;
        if (!m_feature_pipeline->Read(num_frames_, &chunk_feats)) //说明feat结束，没有获取67个frame数据，则自动补0
        {
  	        int padding_len = num_frames_ - chunk_feats.size();
  	        std::vector<float> zero_vector(feature_dim_, 0);
  	        for(int i=0; i<padding_len; i++)
  	        {
  	            chunk_feats.push_back(zero_vector);
  	        }
       	    end_flag=true;
  	    }
        m_chunk_queue.push(chunk_feats);
        // cout<<"add one chunk_feats"<<endl;
    }
} 


bool WavRecognize::ReadOneChunk(std::vector<std::vector<float>> &chunk) {
    if (!m_chunk_queue.empty()) 
    {
        chunk = m_chunk_queue.front();
        m_chunk_queue.pop();
        return true;
    } else 
    {
        return false; 
    }  
}






void WavRecognize::ForwardThread()
{   
    int decoding_window=67;
    int stride=64;
    int chunk_num=0;

    for(int i=0; i<3;i++)
    {
        for(int j=0;j<80;j++)
        {
            feats_last[i*80+j]=-15.9424;
        }
    }


    while(((m_feats_buffer.size()>67)&&(chunk_num==0)) || ((m_feats_buffer.size()>64) && (chunk_num!=0)) )
    {
        cout<<"chunk_num:"<<chunk_num<<"m_feats_buffer.size()"<<m_feats_buffer.size()<<endl;
        if(chunk_num==0)
        {
            for(int i=0; i<67;i++)
            {
            
                for(int j=0;j<80;j++)
                {
                    feats_to_predict[i*80+j]=m_feats_buffer.front().at(j);
                    if(i>=64){   feats_last[(i-64)*80+j]=feats_to_predict[i*80+j];}
                }
                m_feats_buffer.pop();
            }  
        }
        else
        {
            for(int i=0; i<67;i++)
            {
                if(i<3)
                {
                    for(int j=0;j<80;j++)
                    {
                        feats_to_predict[i*80+j]=feats_last[i*80+j];
                    }
                }
                else
                {
                    for(int j=0;j<80;j++)
                    {
                        feats_to_predict[i*80+j]=m_feats_buffer.front().at(j);
                        if(i>=64)
                        {   
                            feats_last[(i-64)*80+j]=feats_to_predict[i*80+j];
                        }
                    }
                    m_feats_buffer.pop();
                }  
            }
        }
        

        
        struct timeval start_time, stop_time;
        gettimeofday(&start_time, NULL);

        int ret;
        // for(int i=0;i<time_n;i++)
        // {
        //     for(int j=0;j<feature_n;j++)
        //     {
        //         input_chunks[i*feature_n+j]=chunk_feats.at(i).at(j);
        //         cout<<i<<":"<<j<<":"<<chunk_feats.at(i).at(j)<<endl;
        //     }
        // }

        if(chunk_num==0)
        {

            encoder_inputs_0[0].buf = (void*)feats_to_predict;

            // for (int i = 0; i < 67; i++) 
            // {
            //     for(int j=0;j<80;j++)
            //     {
            //         float out=((float *)encoder_inputs_0[0].buf)[i*80+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }


            rknn_inputs_set(encoder_ctx_0, encoder_io_num_0.n_input, encoder_inputs_0);
            memset(encoder_outputs_0, 0, sizeof(encoder_outputs_0));
            for (int i = 0; i < encoder_io_num_0.n_output; i++) {encoder_outputs_0[i].want_float = 1;}
            ret = rknn_run(encoder_ctx_0, NULL);
            ret = rknn_outputs_get(encoder_ctx_0, encoder_io_num_0.n_output, encoder_outputs_0, NULL);

            // for (int i = 0; i < 16; i++) {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_outputs_0[0].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }
            // break;

            for (int i = 0; i < 16*(chunk_num+1); i++) {
                for(int j=0;j<256;j++)
                {
                    output_subsampling_cache[i*256+j]=((float *)encoder_outputs_0[1].buf)[i*256+j];
                }
            }


            ctc_inputs_0[0].buf = (void*)encoder_outputs_0[0].buf;
            rknn_inputs_set(ctc_ctx_0, ctc_io_num_0.n_input, ctc_inputs_0);
            memset(ctc_outputs_0, 0, sizeof(ctc_outputs_0));
            for (int i = 0; i < ctc_io_num_0.n_output; i++) {ctc_outputs_0[i].want_float = 1;}
            ret = rknn_run(ctc_ctx_0, NULL);
            ret = rknn_outputs_get(ctc_ctx_0, ctc_io_num_0.n_output, ctc_outputs_0, NULL);

            std::vector<std::vector<float>> ctc_prob;
            int num_outputs = ctc_output_attrs_0[0].dims[1]; //16 //output_probs->getDimensionType()[0];
            int output_dim =  ctc_output_attrs_0[0].dims[2];
            ctc_prob.resize(num_outputs);
            for (int i = 0; i < num_outputs; i++) {
                ctc_prob.at(i).resize(output_dim);
                for(int j=0;j<output_dim;j++)
                {
                    ctc_prob.at(i).at(j)=((float *)ctc_outputs_0[0].buf)[i*output_dim+j];
                    // cout<<i<<":"<<j<<":"<<ctc_prob.at(i).at(j)<<endl;
                }
            }
            m_searcher->Search(ctc_prob);
            UpdateResult(); 
            std::cout<<"partial解码："<<m_result[0].sentence<<std::endl;
            // std::cout<<"partial解码："<< num_outputs<<","<<output_dim<<std::endl;
            
        }


        if(chunk_num==1)
        {

            encoder_inputs_1[0].buf = (void*)feats_to_predict;
            encoder_inputs_1[1].buf = (void*)output_subsampling_cache;

            // for (int i = 0; i < 16; i++) 
            // {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_inputs_1[1].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }

            rknn_inputs_set(encoder_ctx_1, encoder_io_num_1.n_input, encoder_inputs_1);
            memset(encoder_outputs_1, 0, sizeof(encoder_outputs_1));
            for (int i = 0; i < encoder_io_num_1.n_output; i++) {encoder_outputs_1[i].want_float = 1;}
            ret = rknn_run(encoder_ctx_1, NULL);
            ret = rknn_outputs_get(encoder_ctx_1, encoder_io_num_1.n_output, encoder_outputs_1, NULL);

            // for (int i = 0; i < 16; i++) {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_outputs_0[0].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }
            // break;

            for (int i = 0; i < 16*(chunk_num+1); i++) {
                for(int j=0;j<256;j++)
                {
                    output_subsampling_cache[i*256+j]=((float *)encoder_outputs_1[1].buf)[i*256+j];
                }
            }


            ctc_inputs_1[0].buf = (void*)encoder_outputs_1[0].buf;
            rknn_inputs_set(ctc_ctx_1, ctc_io_num_1.n_input, ctc_inputs_1);
            memset(ctc_outputs_1, 0, sizeof(ctc_outputs_1));
            for (int i = 0; i < ctc_io_num_1.n_output; i++) {ctc_outputs_1[i].want_float = 1;}
            ret = rknn_run(ctc_ctx_1, NULL);
            ret = rknn_outputs_get(ctc_ctx_1, ctc_io_num_0.n_output, ctc_outputs_1, NULL);

            std::vector<std::vector<float>> ctc_prob;
            int num_outputs = ctc_output_attrs_1[0].dims[1]; //16 //output_probs->getDimensionType()[0];
            int output_dim =  ctc_output_attrs_1[0].dims[2];
            ctc_prob.resize(num_outputs);
            for (int i = 0; i < num_outputs; i++) {
                ctc_prob.at(i).resize(output_dim);
                for(int j=0;j<output_dim;j++)
                {
                    ctc_prob.at(i).at(j)=((float *)ctc_outputs_1[0].buf)[i*output_dim+j];
                    // cout<<i<<":"<<j<<":"<<ctc_prob.at(i).at(j)<<endl;
                }
            }
            m_searcher->Search(ctc_prob);
            UpdateResult(); 
            std::cout<<"partial解码："<<m_result[0].sentence<<std::endl;
            // std::cout<<"partial解码："<< num_outputs<<","<<output_dim<<std::endl;
            
        }

        if(chunk_num==2)
        {

            encoder_inputs_2[0].buf = (void*)feats_to_predict;
            encoder_inputs_2[1].buf = (void*)output_subsampling_cache;

            // for (int i = 0; i < 16; i++) 
            // {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_inputs_1[1].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }

            rknn_inputs_set(encoder_ctx_2, encoder_io_num_2.n_input, encoder_inputs_2);
            memset(encoder_outputs_2, 0, sizeof(encoder_outputs_2));
            for (int i = 0; i < encoder_io_num_2.n_output; i++) {encoder_outputs_2[i].want_float = 1;}
            ret = rknn_run(encoder_ctx_2, NULL);
            ret = rknn_outputs_get(encoder_ctx_2, encoder_io_num_2.n_output, encoder_outputs_2, NULL);

            // for (int i = 0; i < 16; i++) {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_outputs_0[0].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }
            // break;

            for (int i = 0; i < 16*(chunk_num+1); i++) {
                for(int j=0;j<256;j++)
                {
                    output_subsampling_cache[i*256+j]=((float *)encoder_outputs_2[1].buf)[i*256+j];
                }
            }


            ctc_inputs_2[0].buf = (void*)encoder_outputs_2[0].buf;
            rknn_inputs_set(ctc_ctx_2, ctc_io_num_2.n_input, ctc_inputs_2);
            memset(ctc_outputs_2, 0, sizeof(ctc_outputs_2));
            for (int i = 0; i < ctc_io_num_2.n_output; i++) {ctc_outputs_2[i].want_float = 1;}
            ret = rknn_run(ctc_ctx_2, NULL);
            ret = rknn_outputs_get(ctc_ctx_2, ctc_io_num_0.n_output, ctc_outputs_2, NULL);

            std::vector<std::vector<float>> ctc_prob;
            int num_outputs = ctc_output_attrs_2[0].dims[1]; //16 //output_probs->getDimensionType()[0];
            int output_dim =  ctc_output_attrs_2[0].dims[2];
            ctc_prob.resize(num_outputs);
            for (int i = 0; i < num_outputs; i++) {
                ctc_prob.at(i).resize(output_dim);
                for(int j=0;j<output_dim;j++)
                {
                    ctc_prob.at(i).at(j)=((float *)ctc_outputs_2[0].buf)[i*output_dim+j];
                    // cout<<i<<":"<<j<<":"<<ctc_prob.at(i).at(j)<<endl;
                }
            }
            m_searcher->Search(ctc_prob);
            UpdateResult(); 
            std::cout<<"partial解码："<<m_result[0].sentence<<std::endl;
            // std::cout<<"partial解码："<< num_outputs<<","<<output_dim<<std::endl;
            
        }

        if(chunk_num==3)
        {

            encoder_inputs_3[0].buf = (void*)feats_to_predict;
            encoder_inputs_3[1].buf = (void*)output_subsampling_cache;

            // for (int i = 0; i < 16; i++) 
            // {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_inputs_1[1].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }

            rknn_inputs_set(encoder_ctx_3, encoder_io_num_3.n_input, encoder_inputs_3);
            memset(encoder_outputs_3, 0, sizeof(encoder_outputs_3));
            for (int i = 0; i < encoder_io_num_3.n_output; i++) {encoder_outputs_3[i].want_float = 1;}
            ret = rknn_run(encoder_ctx_3, NULL);
            ret = rknn_outputs_get(encoder_ctx_3, encoder_io_num_3.n_output, encoder_outputs_3, NULL);

            // for (int i = 0; i < 16; i++) {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_outputs_0[0].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }
            // break;

            for (int i = 0; i < 16*(chunk_num+1); i++) {
                for(int j=0;j<256;j++)
                {
                    output_subsampling_cache[i*256+j]=((float *)encoder_outputs_3[1].buf)[i*256+j];
                }
            }


            ctc_inputs_3[0].buf = (void*)encoder_outputs_3[0].buf;
            rknn_inputs_set(ctc_ctx_3, ctc_io_num_3.n_input, ctc_inputs_3);
            memset(ctc_outputs_3, 0, sizeof(ctc_outputs_3));
            for (int i = 0; i < ctc_io_num_3.n_output; i++) {ctc_outputs_3[i].want_float = 1;}
            ret = rknn_run(ctc_ctx_3, NULL);
            ret = rknn_outputs_get(ctc_ctx_3, ctc_io_num_0.n_output, ctc_outputs_3, NULL);

            std::vector<std::vector<float>> ctc_prob;
            int num_outputs = ctc_output_attrs_3[0].dims[1]; //16 //output_probs->getDimensionType()[0];
            int output_dim =  ctc_output_attrs_3[0].dims[2];
            ctc_prob.resize(num_outputs);
            for (int i = 0; i < num_outputs; i++) {
                ctc_prob.at(i).resize(output_dim);
                for(int j=0;j<output_dim;j++)
                {
                    ctc_prob.at(i).at(j)=((float *)ctc_outputs_3[0].buf)[i*output_dim+j];
                    // cout<<i<<":"<<j<<":"<<ctc_prob.at(i).at(j)<<endl;
                }
            }
            m_searcher->Search(ctc_prob);
            UpdateResult(); 
            std::cout<<"partial解码："<<m_result[0].sentence<<std::endl;
            // std::cout<<"partial解码："<< num_outputs<<","<<output_dim<<std::endl;
            
        }


        if(chunk_num==4)
        {

            encoder_inputs_4[0].buf = (void*)feats_to_predict;
            encoder_inputs_4[1].buf = (void*)output_subsampling_cache;

            // for (int i = 0; i < 16; i++) 
            // {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_inputs_1[1].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }

            rknn_inputs_set(encoder_ctx_4, encoder_io_num_4.n_input, encoder_inputs_4);
            memset(encoder_outputs_4, 0, sizeof(encoder_outputs_4));
            for (int i = 0; i < encoder_io_num_4.n_output; i++) {encoder_outputs_4[i].want_float = 1;}
            ret = rknn_run(encoder_ctx_4, NULL);
            ret = rknn_outputs_get(encoder_ctx_4, encoder_io_num_4.n_output, encoder_outputs_4, NULL);

            // for (int i = 0; i < 16; i++) {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_outputs_0[0].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }
            // break;

            for (int i = 0; i < 16*(chunk_num+1); i++) {
                for(int j=0;j<256;j++)
                {
                    output_subsampling_cache[i*256+j]=((float *)encoder_outputs_4[1].buf)[i*256+j];
                }
            }


            ctc_inputs_4[0].buf = (void*)encoder_outputs_4[0].buf;
            rknn_inputs_set(ctc_ctx_4, ctc_io_num_4.n_input, ctc_inputs_4);
            memset(ctc_outputs_4, 0, sizeof(ctc_outputs_4));
            for (int i = 0; i < ctc_io_num_4.n_output; i++) {ctc_outputs_4[i].want_float = 1;}
            ret = rknn_run(ctc_ctx_4, NULL);
            ret = rknn_outputs_get(ctc_ctx_4, ctc_io_num_0.n_output, ctc_outputs_4, NULL);

            std::vector<std::vector<float>> ctc_prob;
            int num_outputs = ctc_output_attrs_4[0].dims[1]; //16 //output_probs->getDimensionType()[0];
            int output_dim =  ctc_output_attrs_4[0].dims[2];
            ctc_prob.resize(num_outputs);
            for (int i = 0; i < num_outputs; i++) {
                ctc_prob.at(i).resize(output_dim);
                for(int j=0;j<output_dim;j++)
                {
                    ctc_prob.at(i).at(j)=((float *)ctc_outputs_4[0].buf)[i*output_dim+j];
                    // cout<<i<<":"<<j<<":"<<ctc_prob.at(i).at(j)<<endl;
                }
            }
            m_searcher->Search(ctc_prob);
            UpdateResult(); 
            std::cout<<"partial解码："<<m_result[0].sentence<<std::endl;
            // std::cout<<"partial解码："<< num_outputs<<","<<output_dim<<std::endl;
            
        }

        if(chunk_num==5)
        {

            encoder_inputs_5[0].buf = (void*)feats_to_predict;
            encoder_inputs_5[1].buf = (void*)output_subsampling_cache;

            // for (int i = 0; i < 16; i++) 
            // {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_inputs_1[1].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }

            rknn_inputs_set(encoder_ctx_5, encoder_io_num_5.n_input, encoder_inputs_5);
            memset(encoder_outputs_5, 0, sizeof(encoder_outputs_5));
            for (int i = 0; i < encoder_io_num_5.n_output; i++) {encoder_outputs_5[i].want_float = 1;}
            ret = rknn_run(encoder_ctx_5, NULL);
            ret = rknn_outputs_get(encoder_ctx_5, encoder_io_num_5.n_output, encoder_outputs_5, NULL);

            // for (int i = 0; i < 16; i++) {
            //     for(int j=0;j<256;j++)
            //     {
            //         float out=((float *)encoder_outputs_0[0].buf)[i*256+j];
            //         cout<<i<<":"<<j<<":"<<out<<endl;
            //     }
            // }
            // break;

            for (int i = 0; i < 16*(chunk_num+1); i++) {
                for(int j=0;j<256;j++)
                {
                    output_subsampling_cache[i*256+j]=((float *)encoder_outputs_5[1].buf)[i*256+j];
                }
            }


            ctc_inputs_5[0].buf = (void*)encoder_outputs_5[0].buf;
            rknn_inputs_set(ctc_ctx_5, ctc_io_num_5.n_input, ctc_inputs_5);
            memset(ctc_outputs_5, 0, sizeof(ctc_outputs_5));
            for (int i = 0; i < ctc_io_num_5.n_output; i++) {ctc_outputs_5[i].want_float = 1;}
            ret = rknn_run(ctc_ctx_5, NULL);
            ret = rknn_outputs_get(ctc_ctx_5, ctc_io_num_0.n_output, ctc_outputs_5, NULL);

            std::vector<std::vector<float>> ctc_prob;
            int num_outputs = ctc_output_attrs_5[0].dims[1]; //16 //output_probs->getDimensionType()[0];
            int output_dim =  ctc_output_attrs_5[0].dims[2];
            ctc_prob.resize(num_outputs);
            for (int i = 0; i < num_outputs; i++) {
                ctc_prob.at(i).resize(output_dim);
                for(int j=0;j<output_dim;j++)
                {
                    ctc_prob.at(i).at(j)=((float *)ctc_outputs_5[0].buf)[i*output_dim+j];
                    // cout<<i<<":"<<j<<":"<<ctc_prob.at(i).at(j)<<endl;
                }
            }
            m_searcher->Search(ctc_prob);
            UpdateResult(); 
            std::cout<<"partial解码："<<m_result[0].sentence<<std::endl;
            // std::cout<<"partial解码："<< num_outputs<<","<<output_dim<<std::endl;
            
        }


        
        gettimeofday(&stop_time, NULL);
        printf("once run use %f ms\n", (__get_us(stop_time) - __get_us(start_time)) / 1000);
        chunk_num++;

    }
    
    
   
   
    
    
}


void WavRecognize::UpdateResult()
{
    const auto& hypotheses = m_searcher->Outputs();
    const auto& inputs =  m_searcher->Inputs();
    const auto& likelihood =  m_searcher->Likelihood();
    const auto& times =  m_searcher->Times();
    m_result.clear();
    for (size_t i = 0; i < hypotheses.size(); i++) {
      const std::vector<int>& hypothesis = hypotheses[i];
      DecodeResult path;
      path.score = likelihood[i];
    //   int offset = global_frame_offset_ * feature_frame_shift_in_ms();
      for (size_t j = 0; j < hypothesis.size(); j++) 
      {
	    std::string word = m_words_map[hypothesis[j]];
        path.sentence += (word);	  
      }
      m_result.emplace_back(path);  
    }
}


WavRecognize::~WavRecognize() {
   
}

