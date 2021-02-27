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
