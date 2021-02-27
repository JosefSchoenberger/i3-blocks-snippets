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
OFLAGS=-O3
DIR=./build

.PHONY: all
all: | dir par
par: log.out color.out cpu_usage

.PHONY: debug
debug: all
debug: OFLAGS+=-O0 -g -DDEBUG

.PHONY: all
dir: 
	mkdir -p $(DIR)/

log.out: log.c log.h
	$(CC) $(WFLAGS) $(OFLAGS) -o $(DIR)/$@ log.c -c

color.out: color.c color.h
	$(CC) $(WFLAGS) $(OFLAGS) -o $(DIR)/$@ color.c -c

cpu_usage: color.out log.out cpu_usage.c
	$(CC) $(WFLAGS) $(OFLAGS) -o $@ cpu_usage.c $(DIR)/log.out $(DIR)/color.out

.PHONY: clean
.SILENT: clean
clean:
	rm -rf build
	rm -f cpu_usage
