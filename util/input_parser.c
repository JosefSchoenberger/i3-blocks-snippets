/*
 * © Copyright 2024 Josef Schönberger
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "input_parser.h"


int parseButtonFromString(const char* input, FILE* logFile, const char* log_prog_name) {
	const char* button = strstr(input, "\"button\":");
	if (!button) {
		appendLogf(LOG_WARN, logFile, "%sGot input line that does not contain a button field: %s", log_prog_name, input);
		return -1;
	}

	while(*button && !isdigit(*button))
		button++;

	if (!*button) {
		appendLogf(LOG_WARN, logFile, "%sGot input line with invalid button field: %s", log_prog_name, input);
		return -1;
	}

	return *button - '0';
}

int readAndParseButton(FILE* logFile, const char* log_prog_name) {
	char buffer[1024];
	fgets(buffer, sizeof(buffer), stdin);
	if (feof(stdin)) {
		appendLogf(LOG_FATAL, logFile, "%sRecieved EOF; Terminating.", log_prog_name);
		return -2;
	}

	return parseButtonFromString(buffer, logFile, log_prog_name);
}
