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

#include <errno.h>
#include <stdlib.h>

#include <stdbool.h>
#include <string.h>
#include <sensors/sensors.h>

#include "temp.h"
#include "util/log.h"

static inline bool startsWith(const char* const a, const char* const prefix) {
	return strncmp(prefix, a, strlen(prefix)) == 0;
}

static sensors_chip_name const *sensor_chip = NULL;
static sensors_subfeature const** temp_subfeatures = NULL;
static unsigned temp_subfeatures_size = 0;

void initTemp(FILE* logFile, const char* progName) {
	if(sensors_init(NULL)) {
		appendLogf(LOG_ERROR, logFile, "%sCould not initialize temperature sensors."
				"Continuing without the temperature feature", progName);
	}

	// find the appropriate sensor chip
	for(int i = 0; (sensor_chip = sensors_get_detected_chips(NULL, &i));) {
#ifdef DEBUG
		appendLogf(LOG_INFO, logFile, "%sFound sensor chip: %s", progName, sensor_chip->prefix);
#endif
		// determine whether the this is a cpu temperature chip
		// Much thanks to htop!
		// https://github.com/htop-dev/htop/blob/master/linux/LibSensors.c
		if(startsWith(sensor_chip->prefix, "coretemp")
				|| startsWith(sensor_chip->prefix, "cpu_thermal")
				|| startsWith(sensor_chip->prefix, "k10temp")
				|| startsWith(sensor_chip->prefix, "zenpower"))
			break;
	}
	if(!sensor_chip) {
		appendLogf(LOG_ERROR, logFile,
				"%sNo temperature chip detected. Proceeding without the temperature feature.", progName);
		return;
	}
#ifdef DEBUG
	appendLogf(LOG_INFO, logFile, "%sSensor chip \"%s\" was deemed suitable.", progName, sensor_chip->prefix);
#endif

	unsigned current_array_size = 128;
	temp_subfeatures = malloc(sizeof(sensors_subfeature const*) * current_array_size);

	sensors_feature const *core;
	for(int i = 0; (core = sensors_get_features(sensor_chip, &i));) {
		// get the input feature
		sensors_subfeature const *sub = sensors_get_subfeature(sensor_chip, core, SENSORS_SUBFEATURE_TEMP_INPUT);

		temp_subfeatures[temp_subfeatures_size++] = sub;
		if(temp_subfeatures_size >= current_array_size) {
			current_array_size *= 2;
			sensors_subfeature const** new = realloc(temp_subfeatures, current_array_size * sizeof(sensors_subfeature const*));
			if(!new) {
				appendLogf(LOG_ERROR, logFile, "%sCould not realloc buffer for up to %u cpus: %s\n"
						"\t\tContinuing with the %d sensors we already have.", progName, current_array_size, strerror(errno), temp_subfeatures_size);
				// Continue with the sensors we already have - 128 are a lot already.
				return;
			} else
				temp_subfeatures = new;
		}
	}

	// reduce buffer size
	sensors_subfeature const** new = realloc(temp_subfeatures, temp_subfeatures_size * sizeof(sensors_subfeature const*));
	if (new || !temp_subfeatures_size)
		temp_subfeatures = new;
}


double getCPUTemp(FILE* logFile, const char* progName) {
	if(!sensor_chip || !temp_subfeatures)
		return -1;

	double max = -1;
	double count = 0;
	for(unsigned i = 0; i < temp_subfeatures_size; i++) {
		double val;
		if(!sensors_get_value(sensor_chip, temp_subfeatures[i]->number, &val)) {
			max = max > val ? max : val;
			count++;
		}
#ifdef DEBUG
		else
			appendLogf(LOG_INFO, logFile, "%s could not get value from temperature sensor %s", progName, temp_subfeatures[i]->name);
#endif
	}
	if (!count) {
		appendLogf(LOG_INFO, logFile, "%s could not get a value for even a single temperature sensor", progName);
	}
	return max;
}

void destructTemp() {
	if(temp_subfeatures)
		free(temp_subfeatures);
}
