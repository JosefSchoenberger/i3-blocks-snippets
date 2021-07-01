# © Copyright 2021 Josef Schönberger
#
# This file is part of my-i3-blocks-snippets.
#
# my-i3-blocks-snippets is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# my-i3-blocks-snippets is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with my-i3-blocks-snippets. If not, see <http://www.gnu.org/licenses/>.

CC=gcc
WFLAGS=-Wall -Wextra -Wpedantic
IFLAGS=-iquote $(CURDIR)
OFLAGS=-O3
DIR=./build

.PHONY:
all: | dir par
par: log.out color.out read_button.out cpu_usage.out

.PHONY: debug
debug: all
debug: OFLAGS+=-O0 -g -DDEBUG

.PHONY:
dir: 
	mkdir -p $(DIR)/

log.out: util/log.c util/log.h
	$(CC) $(WFLAGS) $(OFLAGS) $(IFLAGS) -o $(DIR)/$@ util/log.c -c

color.out: util/color.c util/color.h
	$(CC) $(WFLAGS) $(OFLAGS) $(IFLAGS) -o $(DIR)/$@ util/color.c -c

read_button.out: util/read_button.c util/read_button.h
	$(CC) $(WFLAGS) $(OFLAGS) $(IFLAGS) -o $(DIR)/$@ util/read_button.c -c

cpu_usage.out: $(DIR)/color.out $(DIR)/log.out cpu_usage/cpu_usage.c cpu_usage/temp.c
	$(CC) $(WFLAGS) $(OFLAGS) $(IFLAGS) -o $@ cpu_usage/cpu_usage.c cpu_usage/temp.c $(DIR)/log.out $(DIR)/color.out \
		$(DIR)/read_button.out -lsensors

.PHONY: clean
.SILENT: clean
clean:
	rm -rf build
	rm -f cpu_usage.out
