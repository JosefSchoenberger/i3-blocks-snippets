#pragma once
#include <stdio.h>

void initColor(FILE* logFile, const char* program_name);

__attribute ((const)) char* color(double val, double criticalThreshold, double alertThreshold, double warnThreshold);
