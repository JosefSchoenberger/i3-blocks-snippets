#define _POSIX_C_SOURCE 200809L // for getline
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>

#include <signal.h>

#ifdef DEBUG
#include <proto/error.h>
#endif

#include "log.h"
#include "color.h"

#define LOG_PROG_NAME "\033[33mCPU Usage\033[0m: "
#define IN_BUFFER_SIZE 8

static FILE* logFile;

struct state {
	unsigned long lastTotal, lastIdle;
	FILE *stat, *freq;
	unsigned termLength;
	double (*term[])();
};

double recalculateLastCPU(struct state* state) {
	char buf[192];
	if(!fgets(buf,sizeof(buf),state->stat)) {
		appendLog(LOG_ERROR, logFile, LOG_PROG_NAME "Could not read /proc/stat");
		return -1;
	}

	unsigned long newIdle = state->lastIdle, newTotal = 0;
	char* c = buf;
	for (int i = 0; isalnum(*c); c++) {
		if(isdigit(*c)) {
			int val = strtol(c, &c, 10);
			newTotal += val;
			if (i++ == 3)
				newIdle = val;
		}
	}
	if(fseek(state->stat, +5, SEEK_SET)) {
		appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Could not seek in /proc/stat");
		return EXIT_FAILURE;
	}

	double result = 1 - (newIdle - state->lastIdle)/(double)(newTotal - state->lastTotal);
	state->lastIdle = newIdle;
	state->lastTotal = newTotal;
	return result;
}

int init(struct state* state) {
	if(!(state->stat = fopen("/proc/stat", "r"))) {
		appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Could not load usage file!");
		return EXIT_FAILURE;
	}
	if(setvbuf(state->stat, NULL, _IONBF, 0)) {
		appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Could not disable buffer for /proc/stat");
		return EXIT_FAILURE;
	}
	if(fseek(state->stat, +5, SEEK_SET)) {
		appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Could not seek in /proc/stat");
		return EXIT_FAILURE;
	}

	state->termLength = 0;

	if(!(state->freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r"))) {
		appendLog(LOG_ERROR, logFile, LOG_PROG_NAME "Could not open freqency file\n\t\tDisabling the frequency feature.");
		state->freq = NULL;
	}
	if(state->freq && setvbuf(state->freq, NULL, _IONBF, 0)) {
		appendLog(LOG_ERROR, logFile, LOG_PROG_NAME "Could not disable buffer for /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq.\n\t\tDisabling the frequency feature.");
		fclose(state->freq);
		state->freq = NULL;
	}

	state->lastTotal = state->lastIdle = 0;
	return EXIT_SUCCESS;
}

void printUsage(struct state* state) {
	double usage = 100 * recalculateLastCPU(state);
	if(usage == -100)
		exit(EXIT_FAILURE);
	char *col = color(usage, 95, 90, 80);
	printf("{\"name\":\"CPU\", \"full_text\":\"CPU:%5.1f%%\", \"short_text\":\"CPU%3.0f%%\", \"color\":\"%s\"}\n",
			usage, usage, col);
}

void printTemp(struct state* state) {
	// TODO FIXME implement
	printUsage(state);
}

int error_rep_count = 0;
void printFreq(struct state* state) {
	if(!state->freq) {
		printUsage(state);
		return;
	}

	if(-1 == recalculateLastCPU(state))
		exit(EXIT_FAILURE);

	char buf[16];
	double val;
	if(!fgets(buf,sizeof(buf),state->freq) || !(val=atof(buf))) {
		appendLog(LOG_ERROR, logFile, LOG_PROG_NAME "Could not read/parse freqency file");
		if (error_rep_count >= 5) {
			fclose(state->freq);
			state->freq = 0;
			appendLog(LOG_ERROR, logFile, LOG_PROG_NAME "Could not read/parse freqency file 5 times in a row. Disabling frequency feature.");
		}
		error_rep_count ++;
		return;
	}

	rewind(state->freq);
	error_rep_count--;
	if(error_rep_count < 0)
		error_rep_count = 0;

	val /= 1e6; // From kHz to GHz

	char* color;
	if (val > 2.3)
		color = "#FF3030";
	else if (val > 1.8)
		color = "#FFFF80";
	else if (val > 1.45)
		color = "#FFFFFF";
	else if (val > 1.35)
		color = "#70FF70";
	else
		color = "#7070FF";

	printf("{\"name\":\"CPU\", \"full_text\":\"%.2f GHz\", \"short_text\":\"%.2f GHz\", \"color\":\"%s\"}\n",
			val, val, color);
}

void handleSIGINT(int v) {
	(void) v;
	appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Recieved SIGINT");
	exit(EXIT_SUCCESS);
}

int main() {
	logFile = getLog();

	{
		struct sigaction act;
		act.sa_handler = handleSIGINT;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGINT, &act, NULL);
	}


	int stdinno = fileno(stdin);
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
				appendLogf(LOG_WARN, logFile, LOG_PROG_NAME "Specified time is too small: %lf", timeout.tv_sec + timeout.tv_nsec * 1e-9);
				timeout.tv_sec = 1;
				timeout.tv_nsec = 0;
			}
		}
	}

	const int countdown_set = 8/(timeout.tv_sec + timeout.tv_nsec*1e-9);

	struct state state;
	if(init(&state) == EXIT_FAILURE)
		return EXIT_FAILURE;

	initColor(logFile, LOG_PROG_NAME);

	int countdown=0;
	int type = 1;
	while(1) {
		FD_ZERO(&set);
		FD_SET(stdinno, &set);
		int val = pselect(stdinno + 1, &set, NULL, NULL, &timeout, NULL);
		if (val == -1) {
			if(errno == EINTR) {
				appendLog(LOG_WARN, logFile, LOG_PROG_NAME "Interrupted while selecting");
			} else if (errno == ENOMEM) {
				appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Out of memory");
				return EXIT_FAILURE;
			} else {
				char buff[1024];
				snprintf(buff, 1024, LOG_PROG_NAME "Could not select, %s", strerror(errno));
				appendLog(LOG_FATAL, logFile, buff);
				return EXIT_FAILURE;
			}
			continue;
		}
		if (val != 0) { // not caused by timeout
			char c = fgetc(stdin);
			if (c == EOF) {
				appendLog(LOG_FATAL, logFile, LOG_PROG_NAME "Recievend EOF; Terminating.");
				break;
			}
			if(!isdigit(c))
				continue;
#ifdef DEBUG
			appendLogf(LOG_INFO, logFile, LOG_PROG_NAME "Got %c", c);
#endif
			switch(c) {
				case '1':
				case '3':
					countdown = countdown_set;
					type = c - '0';
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
		ASSERT(type >= 0 && type <= 3 && type != 2);
#endif
		switch(type) {
			case 0:
				printUsage(&state);
				break;
			case 1:
				printTemp(&state);
				break;
			case 3:
				printFreq(&state);
				break;
		}
	}
	fclose(logFile);
	fclose(state.freq);
	fclose(state.stat);
	return EXIT_SUCCESS;
}
