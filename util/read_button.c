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

#include "read_button.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char* line = NULL;
static size_t linebuffer_size = 0;

int get_button() {
	ssize_t n = getline(&line, &linebuffer_size, stdin);
	if (n == -1 && errno == 0) {
		errno = ENODATA;
		return -1;
	}

	if (n < 0)
		return -1;


	static const char needle[] = "\"button\":";
	char *button_text_pos = strstr(line, needle);
	if (!button_text_pos || !button_text_pos[sizeof(needle)] || button_text_pos[sizeof(needle) - 1] == '-')
		goto proto_error;

	char* end_ptr;
	long button = strtol(button_text_pos + sizeof(needle) - 1, &end_ptr, 10);
	if(button_text_pos + sizeof(needle) - 1 == end_ptr)
		goto proto_error;
	return button;

proto_error:
	errno = EPROTO;
	return -1;
}

void button_uninit() {
	if (line)
		free(line);
	line = NULL;
}
