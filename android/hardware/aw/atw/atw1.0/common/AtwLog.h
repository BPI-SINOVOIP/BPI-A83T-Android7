#pragma once

#include <android/log.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

#include <string.h>

#ifndef _LOG_TAG_
#define _LOG_TAG_ "HwAtw"
#endif

#define _LOGV(...) ( (void)__android_log_print(ANDROID_LOG_VERBOSE, _LOG_TAG_, __VA_ARGS__) )
#define _LOG(...) ( (void)__android_log_print(ANDROID_LOG_DEBUG, _LOG_TAG_, __VA_ARGS__) )
#define _ERROR(...) ( (void)__android_log_print(ANDROID_LOG_ERROR, _LOG_TAG_, __VA_ARGS__) )
#define _FATAL(...) {(void)__android_log_print(ANDROID_LOG_FATAL, _LOG_TAG_, __VA_ARGS__);abort();}

#define _LOGE _ERROR

#define _LOG_FATAL_IF(condition,...)\
            {\
              if((condition) == true)\
              {\
                _FATAL(__VA_ARGS__);\
              }\
            }

#define UN_USED(x) (void)(x)


#define ENTER_FUNC()   _LOGV("enter %s, pid=%d, tid=%d", __func__, getpid(), gettid())
#define LEAVE_FUNC()   _LOGV("leave %s, pid=%d, tid=%d", __func__, getpid(), gettid())

class AutoScope
{
public:
  AutoScope(const char* str)
  {
    strcpy(buffer, str);

    _LOGV("ENTER %s", buffer);
  }
  AutoScope(char* str)
  {
    strcpy(buffer, str);

    _LOGV("ENTER %s", buffer);
  }

  ~AutoScope()
  {
    _LOGV("LEAVE %s", buffer);
  }
private:
  char buffer[1024];
};