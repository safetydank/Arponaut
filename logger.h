#ifndef REDC_LOGGER_H
#define REDC_LOGGER_H

#include <stdio.h>
#include <stdarg.h>

namespace redc { namespace log
{

//  Minimal logger (not threadsafe)
class FileLogger
{
public:
	FileLogger(const char* logfile);
	~FileLogger();

	void debug(const char* fmt, ...);
	void message(const char* fmt, ...);

private:
	FILE* fp_;
};

FileLogger* log_instance(const char* logfile="redc.log");
//  switch logger output to a new file.  overwrites new logfile.
FileLogger* log_to(const char* logfile);

void log_dispose();

} }

//  Convenience logging function
#ifdef _DEBUG
#define Log(X, ...) redc::log::log_instance()->message(X, __VA_ARGS__);
#else
#define Log(X, ...)
#endif

#endif