#ifndef LOGGER_H_
#define LOGGER_H_

#include <cstdio>
#include <cstring>
#include <ctime>

static inline char* currentTime();

class Logger {
	public:
		~Logger();
		static Logger* Instance();
		void openLogFile(const char* fileName, bool trunc = false);
		void closeLogFile();
		void log(const char* fmt, ...);

	private:
		static Logger* instance;
		FILE* logFile;
		Logger() { logFile = NULL; };
		Logger(Logger const&) { };
		Logger& operator=(Logger const&) { };
};

#define NEWLINE "\n"

#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define LOG_FILE	"Logger.log"
#define LOG_FMT		"%s | %-5s | %s:%d | "
#define LOG_ARGS(LOG_TAG) currentTime(), LOG_TAG, _FILE, __LINE__
#define INDENT_FMT	"%*s"

#if DEBUG
#define LOG_DEBUG(message, args...)														\
	do {																				\
		Logger::Instance()->log(LOG_FMT message NEWLINE, LOG_ARGS("DEBUG"), ## args);	\
	} while(0)
#else
#define LOG_DEBUG(message, args...)
#endif

#define LOG_INFO_INDENT(indent, message, args...)										\
	do {																				\
		Logger::Instance()->log(LOG_FMT message NEWLINE, LOG_ARGS("INFO"), ## args);	\
		fprintf(stdout, INDENT_FMT message NEWLINE, (indent * 4), "", ## args);			\
	} while(0)

#define LOG_INFO(message, args...) LOG_INFO_INDENT(0, message, ## args)

#define LOG_ERROR_INDENT(indent, message, args...)										\
	do {																				\
		Logger::Instance()->log(LOG_FMT message NEWLINE, LOG_ARGS("ERROR"), ## args);	\
		fprintf(stderr, INDENT_FMT message NEWLINE, (indent * 4), "", ## args);			\
	} while(0)

#define LOG_ERROR(message, args...) LOG_ERROR_INDENT(0, message, ## args)

static inline char *currentTime() {
	static char buffer[64];
	time_t rawTime;
	struct tm *timeInfo;

	time(&rawTime);
	timeInfo = localtime(&rawTime);

	strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeInfo);

	return buffer;
}

#endif // LOGGER_H_
