/*
 * © Copyright 2021 Josef Schönberger
 *
 * This file is part of my-i3-blocks-snippets.
 *
 * my-i3-blocks-snippets is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * my-i3-blocks-snippets is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with my-i3-blocks-snippets. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <stdio.h>

FILE* getLog();

enum LOGLEVEL {
	LOG_FATAL, LOG_ERROR, LOG_WARN, LOG_INFO
};

void appendLog(enum LOGLEVEL level, FILE* logfile, char* msg);
__attribute__((format(printf, 3, 4))) void appendLogf(enum LOGLEVEL level, FILE* logfile, char* format, ...);
