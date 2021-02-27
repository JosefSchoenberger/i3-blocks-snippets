#pragma once
#include <stdio.h>

FILE* getLog();

enum LOGLEVEL {
	LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO
};

void appendLog(enum LOGLEVEL level, FILE* logfile, char* msg);
void appendLogf(enum LOGLEVEL level, FILE* logfile, char* format, ...);
