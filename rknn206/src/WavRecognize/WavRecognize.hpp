//  Created by Linzaer on 2019/11/15.
//  Copyright © 2019 Linzaer. All rights reserved.

#ifndef WavRecognize_LA21_hpp12
#define WavRecognize_LA21_hpp12

#pragma once



#include "models206_typedef.h"
#include "rknn_api.h"
#include "rga.h"
#include "RgaUtils.h"
#include "im2d.h"


#include <opencv2/opencv.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <sys/time.h>
#include <fstream>
#include <map>

#include "wav.h"
#include "feature_pipeline.h"
#include "ctc_prefix_beam_search.h"
#include "search_interface.h"
#include "fbank.h"

using namespace M2;
using namespace wenet;



struct WordPiece {
  std::string word;
  int start = -1;
  int end = -1;
  WordPiece(std::string word, int start, int end): word(std::move(word)), start(start), end(end) {}
};

struct DecodeResult {
  float score = -kFloatMax;
  std::string sentence;
  std::vector<WordPiece> word_pieces;
  
  static bool CompareFunc(const DecodeResult& a, const DecodeResult& b) {
    return a.score > b.score;
  }
};




class WavRecognize {
public:
    WavRecognize();
    ~WavRecognize();

    int Init(int print_config,int modelType);
    void ForwardThread();
    bool ReadOneChunk(std::vector<std::vector<float>> &chunk);
    void GetChunkThread();


    int InitModel(string model_path,
                                rknn_context &ctx,rknn_input_output_num &io_num,
                                rknn_tensor_attr *input_attrs,rknn_tensor_attr *output_attrs,
                                rknn_input *inputs,rknn_output *outputs);

    void UpdateResult();
    int GetWavSignalsThread();

    int model_is_ok;


private:
    
    std::shared_ptr<FeaturePipeline> m_feature_pipeline;
    std::queue<std::vector<std::vector<float>>> m_chunk_queue;
    CtcPrefixBeamSearchOptions m_ctc_search_opt;
    std::unique_ptr<SearchInterface> m_searcher;
    std::map<int, std::string> m_words_map;
    std::vector<DecodeResult> m_result;
    std::vector<float> m_wave;

    std::shared_ptr<Fbank> m_fbank;

    std::vector<float> m_pre_signal;

    std::queue<std::vector<float>> m_feats_buffer;
    



    int channel = 1;
    int width   = 0;
    int height  = 0;
    
    rknn_context   encoder_ctx_0;
    rknn_input_output_num encoder_io_num_0;
    rknn_tensor_attr encoder_input_attrs_0[1];
    rknn_tensor_attr encoder_output_attrs_0[2];
    rknn_input encoder_inputs_0[1];
    rknn_output encoder_outputs_0[2];
    rknn_context   ctc_ctx_0;
    rknn_input_output_num ctc_io_num_0;
    rknn_tensor_attr ctc_input_attrs_0[1];
    rknn_tensor_attr ctc_output_attrs_0[1];
    rknn_input ctc_inputs_0[1];
    rknn_output ctc_outputs_0[1];


    rknn_context   encoder_ctx_1;
    rknn_input_output_num encoder_io_num_1;
    rknn_tensor_attr encoder_input_attrs_1[2];
    rknn_tensor_attr encoder_output_attrs_1[2];
    rknn_input encoder_inputs_1[2];
    rknn_output encoder_outputs_1[2];
    rknn_context   ctc_ctx_1;
    rknn_input_output_num ctc_io_num_1;
    rknn_tensor_attr ctc_input_attrs_1[1];
    rknn_tensor_attr ctc_output_attrs_1[1];
    rknn_input ctc_inputs_1[1];
    rknn_output ctc_outputs_1[1];


    rknn_context   encoder_ctx_2;
    rknn_input_output_num encoder_io_num_2;
    rknn_tensor_attr encoder_input_attrs_2[2];
    rknn_tensor_attr encoder_output_attrs_2[2];
    rknn_input encoder_inputs_2[2];
    rknn_output encoder_outputs_2[2];
    rknn_context   ctc_ctx_2;
    rknn_input_output_num ctc_io_num_2;
    rknn_tensor_attr ctc_input_attrs_2[1];
    rknn_tensor_attr ctc_output_attrs_2[1];
    rknn_input ctc_inputs_2[1];
    rknn_output ctc_outputs_2[1];

    rknn_context   encoder_ctx_3;
    rknn_input_output_num encoder_io_num_3;
    rknn_tensor_attr encoder_input_attrs_3[2];
    rknn_tensor_attr encoder_output_attrs_3[2];
    rknn_input encoder_inputs_3[2];
    rknn_output encoder_outputs_3[2];
    rknn_context   ctc_ctx_3;
    rknn_input_output_num ctc_io_num_3;
    rknn_tensor_attr ctc_input_attrs_3[1];
    rknn_tensor_attr ctc_output_attrs_3[1];
    rknn_input ctc_inputs_3[1];
    rknn_output ctc_outputs_3[1];

    rknn_context   encoder_ctx_4;
    rknn_input_output_num encoder_io_num_4;
    rknn_tensor_attr encoder_input_attrs_4[2];
    rknn_tensor_attr encoder_output_attrs_4[2];
    rknn_input encoder_inputs_4[2];
    rknn_output encoder_outputs_4[2];
    rknn_context   ctc_ctx_4;
    rknn_input_output_num ctc_io_num_4;
    rknn_tensor_attr ctc_input_attrs_4[1];
    rknn_tensor_attr ctc_output_attrs_4[1];
    rknn_input ctc_inputs_4[1];
    rknn_output ctc_outputs_4[1];



    rknn_context   encoder_ctx_5;
    rknn_input_output_num encoder_io_num_5;
    rknn_tensor_attr encoder_input_attrs_5[2];
    rknn_tensor_attr encoder_output_attrs_5[2];
    rknn_input encoder_inputs_5[2];
    rknn_output encoder_outputs_5[2];
    rknn_context   ctc_ctx_5;
    rknn_input_output_num ctc_io_num_5;
    rknn_tensor_attr ctc_input_attrs_5[1];
    rknn_tensor_attr ctc_output_attrs_5[1];
    rknn_input ctc_inputs_5[1];
    rknn_output ctc_outputs_5[1];

};

#endif /* WavRecognize_hpp */
