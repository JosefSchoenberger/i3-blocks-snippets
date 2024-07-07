#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <unistd.h>

#include "util/log.h"
#include "util/color.h"
#include "util/input_parser.h"

#define LOG_PROG_NAME "\033[33mMem Usage\033[0m: "

static FILE* logFile;
static void handleSIGINT(int v) {
	(void) v;
	appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Recieved SIGINT");
	exit(EXIT_SUCCESS);
}

static unsigned long valueForMeminfoType(const char* meminfo_data, const char* type) {
	const char* p = strstr(meminfo_data, type);
	if (!p) {
		appendLogf(LOG_FATAL, logFile, "Could not find \"%s\" in /proc/meminfo?!? Terminating.", type);
		exit(1);
	}
	p += strlen(type);
	while(*p && !isdigit(*p))
		p++;
	if (!p) {
		appendLogf(LOG_FATAL, logFile, "\"%s\" in /proc/meminfo has no value?!? Terminating.", type);
		exit(1);
	}

	errno = 0;
	unsigned long value = strtoul(p, NULL, 10);
	if (errno) {
		appendLogf(LOG_FATAL, logFile, "value for \"%s\" in /proc/meminfo is out of range (%s)?!? Terminating.", type, p);
		return 0;
	}
	return value;
}

static int meminfo_fd = -1;
static void prepare_meminfo(void) {
	if (meminfo_fd < 0) {
		meminfo_fd = open("/proc/meminfo", O_RDONLY);
		if (meminfo_fd < 0) {
			appendLogf(LOG_FATAL, logFile, "Could not open /proc/meminfo: %s. Terminating.", strerror(errno));
			exit(1);
		}
	} else {
		if (lseek(meminfo_fd, SEEK_SET, 0) < 0) {
			appendLogf(LOG_FATAL, logFile, "Could not rewind /proc/meminfo: %s. Terminating.", strerror(errno));
			exit(1);
		}
	}

}
static double calc_percentage(const char* total_key, const char* free_key) {
	prepare_meminfo();

	char buffer[1024*6];
	ssize_t r = read(meminfo_fd, buffer, sizeof(buffer)-1);
	if (r < 0) {
		appendLogf(LOG_FATAL, logFile, "Could not read from /proc/meminfo: %s. Terminating.", strerror(errno));
		exit(1);
	}
	buffer[r] = '\0';

	unsigned long total = valueForMeminfoType(buffer, total_key);
	unsigned long avail = valueForMeminfoType(buffer, free_key);
	return (total - avail)*100.0/total;
}

static void printValue(const char* prefix, double percentage, const char* suffix, const char* color) {
	if (!color)
		color = "#FFFFFF";
	if (prefix)
		printf("{\"name\":\"Mem\", \"full_text\":\"%s:%5.1f%s\", \"short_text\":\"%s%3.0f%s\", \"color\":\"%s\"}\n",
			prefix, percentage, suffix, prefix, percentage, suffix, color);
	else
		printf("{\"name\":\"Mem\", \"full_text\":\"%5.1f%s\", \"short_text\":\"%3.0f%s\", \"color\":\"%s\"}\n",
			percentage, suffix, percentage, suffix, color);
}

static void printUsage(void) {
	prepare_meminfo();

	char buffer[1024*6];
	ssize_t r = read(meminfo_fd, buffer, sizeof(buffer)-1);
	if (r < 0) {
		appendLogf(LOG_FATAL, logFile, "Could not read from /proc/meminfo: %s. Terminating.", strerror(errno));
		exit(1);
	}
	buffer[r] = '\0';

	unsigned long total = valueForMeminfoType(buffer, "MemTotal:");
	unsigned long avail = valueForMeminfoType(buffer, "MemAvailable:");
	unsigned long cached = valueForMeminfoType(buffer, "Cached:");
	cached += valueForMeminfoType(buffer, "SwapCached:");
	cached += valueForMeminfoType(buffer, "Buffers:");
	double percentage = (total - avail - cached)*100.0/total;
	printValue("RAM", percentage, "%", color(percentage, 95.0, 90.0, 85.0));
}
static void printUsageWithCache(void) {
	double percentage = calc_percentage("MemTotal:", "MemAvailable:");
	printValue("RAM(cache)", percentage, "%", color(percentage, 95.0, 90.0, 85.0));
}
static void printSwapUsage(void) {
	double percentage = calc_percentage("SwapTotal:", "SwapFree:");
	printValue("SWP", percentage, "%", color(percentage, 95.0, 90.0, 85.0));
}

int main(void) {
	logFile = getLog();

	{
		struct sigaction act;
		act.sa_handler = handleSIGINT;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGINT, &act, NULL);
	}

	initColor(logFile, LOG_PROG_NAME);

	fd_set set;

	struct timespec timeout = {2, 0};
	char* c = getenv("SLEEP");
	if (c) {
		char *c_ptr;
		double parsed = strtod(c, &c_ptr);
		if (c_ptr == c || *c_ptr) {
			appendLogf(LOG_WARN, logFile, LOG_PROG_NAME "Could not parse sleep parameter: \"%s\"", c);
		} else {
			timeout.tv_sec = parsed;
			timeout.tv_nsec = (parsed - timeout.tv_sec) * 1e9;
			if (timeout.tv_sec == 0 && timeout.tv_nsec < 1e8) {
				appendLogf(LOG_WARN, logFile, LOG_PROG_NAME "Specified time is too small: %lf",
						timeout.tv_sec + timeout.tv_nsec * 1e-9);
				timeout.tv_sec = 1;
				timeout.tv_nsec = 0;
			}
		}
	}

	const int countdown_set = 8/(timeout.tv_sec + timeout.tv_nsec*1e-9);

	printUsage();

	int countdown=0;
	int type = 1;
	while(1) {
		FD_ZERO(&set);
		FD_SET(STDIN_FILENO, &set);
		int val = pselect(STDIN_FILENO + 1, &set, NULL, NULL, &timeout, NULL);
		if (val == -1) {
			if(errno == EINTR) {
				appendLog(LOG_WARN, logFile, LOG_PROG_NAME "Interrupted while selecting");
			} else if (errno == ENOMEM) {
				appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Out of memory");
				return EXIT_FAILURE;
			} else {
				appendLogf(LOG_FATAL, logFile, LOG_PROG_NAME "Could not select, %s",
						strerror(errno));
				return EXIT_FAILURE;
			}
		}

		if (val != 0) { // not caused by timeout
			int button = readAndParseButton(logFile, LOG_PROG_NAME);
			if (button < -1)
				break;
			if (button == -1)
				continue;


#ifdef DEBUG
			appendLogf(LOG_INFO, logFile, LOG_PROG_NAME "Button press: %d", button);
#endif
			switch(button) {
				case 1:
				case 3:
					countdown = countdown_set;
					type = button;
					break;
				default:
					countdown = 0;
					break;
			}
			// fall thru: update directly
		}
		// update
		if (countdown)
			countdown--;
		else
			type = 0;

#ifdef DEBUG
		ASSERT(type >= 0 && type <= 1);
#endif
		switch(type) {
			case 0:
				printUsage();
				break;
			case 1:
				printUsageWithCache();
				break;
			case 3:
				printSwapUsage();
				break;
		}
		fflush(stdout);
	}
	if(logFile) fclose(logFile);
	return EXIT_SUCCESS;
}
