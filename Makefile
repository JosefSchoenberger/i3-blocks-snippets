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
OFLAGS ?= -O3
DIR=./build

.PHONY: all
all: cpu_usage.out memory_usage.out

.PHONY: debug
debug: all
debug: OFLAGS+=-O0 -g -DDEBUG

.PHONY: all
$(DIR):
	mkdir -p $(DIR)/

$(DIR)/%.out: util/%.c util/%.h | $(DIR)
	$(CC) $(WFLAGS) $(OFLAGS) $(IFLAGS) -o $@ $< -c

cpu_usage.out: $(DIR)/color.out $(DIR)/log.out $(DIR)/input_parser.out cpu_usage/cpu_usage.c cpu_usage/temp.c
	$(CC) $(WFLAGS) $(OFLAGS) $(IFLAGS) -o $@ $^ -lsensors
memory_usage.out: $(DIR)/color.out $(DIR)/log.out $(DIR)/input_parser.out memory_usage/memory_usage.c
	$(CC) $(WFLAGS) $(OFLAGS) $(IFLAGS) -o $@ $^

.PHONY: clean
.SILENT: clean
clean:
	rm -rf build
	rm -f cpu_usage.out memory_usage.out
