#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>
#include <stdarg.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CRESET "\033[0m"

FILE* getLog() {
	const char* s = getenv("LOGPATH");
	if(!s)
		return NULL;

	FILE* f = fopen(s, "a");
	if(!f) {
		fprintf(stderr, "Could not open log file: %s\n", strerror(errno));
	}
	return f;
}

static char* levelToString(enum LOGLEVEL level) {
	switch(level) {
		case LOG_FATAL: return RED " FATAL " CRESET;
		case LOG_ERROR: return RED " ERROR " CRESET;
		case LOG_WARN: return YELLOW "WARNING" CRESET;
		case LOG_INFO: return BLUE " INFO  " CRESET;
		default: return "UNKNOWN";
	}
}

void appendLog(enum LOGLEVEL level, FILE* logfile, char* msg) {
	if(!logfile)
		return;

	char timeBuffer[32];
	time_t timer = time(NULL);
	struct tm *tm_info = localtime(&timer);
	strftime(timeBuffer, sizeof(timeBuffer), GREEN "%Y-%m-%d %H:%M:%S" CRESET, tm_info);

	flock(fileno(logfile), LOCK_EX);
	fprintf(logfile, "%s [%s] %s\n", timeBuffer, levelToString(level), msg);
	fflush(logfile);
	flock(fileno(logfile), LOCK_UN);
}

void appendLogf(enum LOGLEVEL level, FILE* logfile, char* format, ...) {
	if(!logfile)
		return;

	char buf[512];
	va_list argp;
	va_start(argp, format);
	size_t size_needed = vsnprintf(buf, sizeof(buf), format, argp);
	if (size_needed++ >= 512) {
		char* new_buf = malloc(size_needed);
		if (!new_buf) { // man, this really would not be our day!
			appendLog(LOG_ERROR, logfile, "OUT OF MEMORY FOR LOG. Here's a truncated version:");
			appendLog(level, logfile, buf);
			va_end(argp);
			return;
		}
		va_end(argp);
		va_start(argp, format);
		vsnprintf(new_buf, size_needed, format, argp);
		appendLog(level, logfile, new_buf);
		free(new_buf);
	} else
		appendLog(level, logfile, buf);
	va_end(argp);
}
