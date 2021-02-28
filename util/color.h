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

void initColor(FILE* logFile, const char* program_name);

__attribute ((const)) char* color(double val, double criticalThreshold, double alertThreshold, double warnThreshold);
