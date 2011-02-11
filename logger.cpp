#include "logger.h"
#include <cassert>
#include <wchar.h>

using namespace redc::log;

FileLogger::FileLogger(const char* logfile)
{
#ifdef WIN32
    errno_t err;

    if ((err = fopen_s(&fp_, logfile, "w+")) != 0)
        fprintf(stderr, "Unable to open file %s", logfile);
#else
    fp_ = fopen(logfile, "w+");
    if (!fp_)
        fprintf(stderr, "Unable to open file %s", logfile);
#endif
}

FileLogger::~FileLogger()
{
    fclose(fp_);
}

void FileLogger::message(const char* fmt, ...)
{
    static char buf[512];

    assert(fp_);

    va_list args;
    va_start(args, fmt);
    vfprintf(fp_, fmt, args);
    va_end(args);
}

static FileLogger* instance_ = NULL;

FileLogger* redc::log::log_instance(const char* logfile)
{
    if (instance_ == NULL)
        instance_ = new FileLogger(logfile);

    return instance_;
}

void redc::log::log_dispose()
{
    //if (instance_)
    //    Log("...log disposed");

    delete(instance_);
    instance_ = NULL;
}

FileLogger* redc::log::log_to(const char* logfile)
{
    redc::log::log_dispose();
    return log_instance(logfile);
}
