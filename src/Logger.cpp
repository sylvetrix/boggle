#include <cstdarg>
#include <fstream>

#include "Logger.h"

Logger* Logger::instance = NULL;

Logger::~Logger() {
	if (instance != NULL) {
		instance->closeLogFile();
	}
}

Logger* Logger::Instance() {
	if (!instance) {
		instance = new Logger();
	}

	return instance;
}

void Logger::openLogFile(const char* fileName, bool trunc) {
	if (logFile) { return; }

	if (trunc) {
		logFile = fopen(fileName, "w");
	}
	else {
		logFile = fopen(fileName, "a");
	}

	if (!logFile) {
		// use fprintf instead of perror to add log file name
		fprintf(stderr, "Error opening log file '%s': %s\n", fileName, strerror(errno));
	}
}

void Logger::closeLogFile() {
	if (logFile) {
		fflush(logFile);
		fclose(logFile);
		logFile = NULL;
	}
}

void Logger::log(const char* fmt, ...) {
	if ((logFile != NULL) && logFile) {
		va_list args;
		va_start(args, fmt);
		vfprintf(logFile, fmt, args);
		va_end(args);
		fflush(logFile);
	}
}
