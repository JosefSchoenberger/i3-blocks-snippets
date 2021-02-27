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

#include "color.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "log.h"

char *colors[4] = { "#FF4040", "#FF8000", "#FFFF90", "#FFFFFF" };

__attribute ((const)) bool isValidColor(char* color) {
	// TODO vectorize?
	/*
	 * uint64_t v = (uint64_t*) color;
	 * v -= 0x0030303030303023UL;
	 * uint64_t mask = v & 0x8080808080808080UL;
	 * v -= 0x010A0A0A0A0A0A01UL;
	 * mask |= ~(v & 0x8080808080808080UL);
	 * v &=
	 * return mask;
	 */
	bool res = *color == '#';
	for(int i = 1; i < 7; i++) {
		res &= (color[i] >= '0' && color[i] <= '9') || (color[i] >= 'A' && color[i] <= 'F') || (color[i] >= 'a' && color[i] <= 'f');
	}
	res &= color[7] == '\000';
	return res;
}

void initColor(FILE* logFile, const char* program_name) {
	char *c = getenv("COLOR_CRIT");

	if (c && !isValidColor(c))
		appendLogf(LOG_WARN, logFile, "%sColor \"%s\" is not valid", program_name, c);
	else if (c)
		colors[0] = c;

	c = getenv("COLOR_ALRT");
	if (c && !isValidColor(c))
		appendLogf(LOG_WARN, logFile, "%sColor \"%s\" is not valid", program_name, c);
	else if (c)
		colors[1] = c;

	c = getenv("COLOR_WARN");
	if (c && !isValidColor(c))
		appendLogf(LOG_WARN, logFile, "%sColor \"%s\" is not valid", program_name, c);
	else if (c)
		colors[2] = c;

	c = getenv("COLOR_NORM");
	if (c && !isValidColor(c))
		appendLogf(LOG_WARN, logFile, "%sColor \"%s\" is not valid", program_name, c);
	else if (c)
		colors[3] = c;
}

__attribute ((const)) char* color(double val, double criticalThreshold, double alertThreshold, double warnThreshold) {
	if(val >= criticalThreshold)
		return colors[0];
	else if (val >= alertThreshold)
		return colors[1];
	else if (val >= warnThreshold)
		return colors[2];
	else
		return colors[3];
}
