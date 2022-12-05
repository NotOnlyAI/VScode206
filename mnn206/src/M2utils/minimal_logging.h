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
#ifndef TENSORFLOW_LITE_MINIMAL_LOGGING_H_
#define TENSORFLOW_LITE_MINIMAL_LOGGING_H_

#include <cstdarg>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdint.h>

#include <sstream>
#include <ctime>
namespace tflite {

void initLogger();

typedef enum log_rank {
        INFO,
        WARNING,
        ERROR,
        FATAL
}log_rank_t;


enum LogSeverity {
  TFLITE_LOG_INFO = 0,
  TFLITE_LOG_WARNING = 1,
  TFLITE_LOG_ERROR = 2,
};








// Helper class for simple platform-specific console logging. Note that we
// explicitly avoid the convenience of ostream-style logging to minimize binary
// size impact.
class MinimalLogger {
 public:
  // Logging hook that takes variadic args.
  static void Log(LogSeverity severity, const char* format, ...);

  // Logging hook that takes a formatted va_list.
  static void LogFormatted(LogSeverity severity, const char* format,
                           va_list args);

 private:
  static const char* GetSeverityName(LogSeverity severity);
};


class Logger
{
  friend void initLogger();

  public:
            //构造函数
  Logger(log_rank_t log_rank) : m_log_rank(log_rank) {};
  ~Logger();
  static std::ostream& start(log_rank_t log_rank,const int line,const std::string& function);
  private:
  static std::ostream& getStream();
  static std::ofstream m_log_file;                   ///< 信息日子的输出流
  log_rank_t m_log_rank;                             ///< 日志的信息的等级
  uint32_t m_log_times;
};

}  // namespace tflite

// Convenience macro for basic internal logging in production builds.
// Note: This should never be used for debug-type logs, as it will *not* be
// stripped in release optimized builds. In general, prefer the error reporting
// APIs for developer-facing errors, and only use this for diagnostic output
// that should always be logged in user builds.
#define TFLITE_LOG_PROD(severity, format, ...) tflite::MinimalLogger::Log(severity, format, ##__VA_ARGS__);

// Convenience macro for logging a statement *once* for a given process lifetime
// in production builds.
#define TFLITE_LOG_PROD_ONCE(severity, format, ...)    \
  do {                                                 \
    static const bool s_logged = [&] {                 \
      TFLITE_LOG_PROD(severity, format, ##__VA_ARGS__) \
      return true;                                     \
    }();                                               \
    (void)s_logged;                                    \
  } while (false);


#define TFLITE_LOG_TXT(log_rank)   tflite::Logger(log_rank).start(log_rank, __LINE__,__FUNCTION__)

// #ifndef NDEBUG
// In debug builds, always log.
#define TFLITE_LOG TFLITE_LOG_PROD
#define TFLITE_LOG_ONCE TFLITE_LOG_PROD_ONCE
// #else
// In prod builds, never log, but ensure the code is well-formed and compiles.
// #define TFLITE_LOG(severity, format, ...)             \
//   while (false) {                                     \
//     TFLITE_LOG_PROD(severity, format, ##__VA_ARGS__); \
//   }
// #define TFLITE_LOG_ONCE TFLITE_LOG
// #endif

#endif  // TENSORFLOW_LITE_MINIMAL_LOGGING_H_
