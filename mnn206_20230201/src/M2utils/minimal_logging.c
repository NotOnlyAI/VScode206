/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "minimal_logging.h"
#include <fstream>
#include <string>
#include <sstream>

#include <cstdlib>
#include <ctime>
#include <cstdarg>



namespace tflite {


void MinimalLogger::Log(LogSeverity severity, const char* format, ...) {
  va_list args;
  va_start(args, format);
  LogFormatted(severity, format, args);
  va_end(args);
}

const char* MinimalLogger::GetSeverityName(LogSeverity severity) {
  switch (severity) {
    case TFLITE_LOG_INFO:
      return "INFO";
    case TFLITE_LOG_WARNING:
      return "WARNING";
    case TFLITE_LOG_ERROR:
      return "ERROR";
  }
  return "<Unknown severity>";
}
void MinimalLogger::LogFormatted(LogSeverity severity, const char* format,
                                 va_list args) {
    fprintf(stderr, "%s: ", GetSeverityName(severity));
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wformat-nonliteral"
    vfprintf(stderr, format, args);
    #pragma clang diagnostic pop
    fputc('\n', stderr);
}

std::ofstream Logger::m_log_file;


std::ostream& Logger::getStream(){
    return (m_log_file.is_open() ? m_log_file : std::cout) ;
}

void initLogger(){
    time_t tNowTime;
    time(&tNowTime);
    tm* tLocalTime = localtime(&tNowTime);
        //"2011-07-18 23:03:01 ";
    std::string strFormat = "%Y-%m-%d-%H-%M-%S-";
    char strDateTime[30] = { '\0' };
    strftime(strDateTime, 30, strFormat.c_str(), tLocalTime);
    std::string strRes = strDateTime;
    std::string sDateStr = "./models206/logs/"+strRes + "log";
    Logger::m_log_file.open(sDateStr.c_str(),std::ios_base::out | std::ios_base::app);
}



std::ostream& Logger::start(log_rank_t log_rank,const int line,const std::string&function)
{
    time_t tm;
    time(&tm);
    char time_string[128];
    ctime_r(&tm, time_string);
    if (log_rank==INFO)
    {
        return getStream() <<std::endl<<std::endl<< "INFO"<<":"<< "function (" << function << ")"<< "line " << line<<" "<<std::endl<< time_string<<std::flush;
    }else if (log_rank==ERROR)
    {
        return getStream() <<std::endl<<std::endl<< "ERROR"<<":"<< "function (" << function << ")"<< "line " << line<<" "<<std::endl<< time_string<<std::flush;
    }else if (log_rank==WARNING)
    {
       return getStream() <<std::endl<<std::endl<< "WARNING"<<":"<< "function (" << function << ")"<< "line " << line<<" "<<std::endl<< time_string<<std::flush;
    }else if (log_rank==FATAL)
    {
       return getStream() << "FATAL"<<":"<< "function (" << function << ")"<< "line " << line<<" "<<std::endl<< time_string<<std::flush;
    }

}

Logger::~Logger(){
    getStream() << std::endl << std::flush;

    if (FATAL == m_log_rank) {
        m_log_file.close();
        abort();
    }
}

}  // namespace tflite
