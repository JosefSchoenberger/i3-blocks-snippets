#include <stdio.h>
#include <time.h>

void format_time(char buf[256], const time_t time) {
	const struct tm* const t = localtime(&time);
	strftime(buf, 256, "{\"name\":\"Clock\", \"full_text\":\"%a, %d. %b %Y %T\", \"short_text\":\"%a, %d.%m.%y %T\", \"color\":\"#FFFFFF\"}", t);
}

int main(void) {
	struct timespec time;
	char buf[256];

	clock_gettime(CLOCK_REALTIME, &time);
	format_time(buf, time.tv_sec);
	puts(buf);
	fflush(stdout);

	for(;;) {
		time.tv_sec += 1;
		time.tv_nsec = 0;
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &time, NULL);
		clock_gettime(CLOCK_REALTIME, &time);
		format_time(buf, time.tv_sec);
		puts(buf);
		fflush(stdout);
		//printf("%ld\n", time.tv_nsec);
	}
}
