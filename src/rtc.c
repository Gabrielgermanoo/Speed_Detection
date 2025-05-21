#include "rtc.h"

LOG_MODULE_REGISTER(rtc, LOG_LEVEL_DBG);

#define TIME_QUEUE_SIZE          5

K_MSGQ_DEFINE(time_msgq, sizeof(struct rtc_time), TIME_QUEUE_SIZE, 4);

static struct sys_time {
	char server[64];
	int8_t timezone;
} self = {.server = CONFIG_DEFAULT_SNTP_SERVER, .timezone = 0};

/**
 * @brief Function to get the current time from SNTP server.
 *
 * @param p1 Void pointer to the first parameter (not used).
 * @param p2 Void pointer to the second parameter (not used).
 * @param p3 Void pointer to the third parameter (not used).
 */
static void time_thread_function(void *p1, void *p2, void *p3);

int tracker_get_time(struct rtc_time *time)
{
	struct sntp_ctx ctx;
	struct sntp_time datetime;
	struct sockaddr_in server_addr;
	int err = 0;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(123);

	memset(&ctx, 0, sizeof(ctx));

	err = inet_pton(AF_INET, self.server, &server_addr.sin_addr);
	
	if (err < 0) {
		LOG_ERR("Invalid SNTP server address: %d", err);
		return -EINVAL;
	}

	err = sntp_init(&ctx, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	
	if (err < 0) {
		LOG_ERR("Failed to initialize SNTP: %d", err);
		return err;
	}

	err = sntp_query(&ctx, 4 * MSEC_PER_SEC, &datetime);

	if (err < 0) {
		LOG_ERR("Failed to query SNTP: %d", err);
		goto close_sntp;
	}

close_sntp:

	sntp_close(&ctx);

	time_t unix_time = datetime.seconds;
	unix_time += self.timezone * 3600;

	struct tm tm_info;
	if (gmtime_r(&unix_time, &tm_info) == NULL) {
		LOG_ERR("Failed to convert time to UTC");
		return -EINVAL;
	}

	time->tm_sec = tm_info.tm_sec;
	time->tm_min = tm_info.tm_min;
	time->tm_hour = tm_info.tm_hour;
	time->tm_mday = tm_info.tm_mday;
	time->tm_mon = tm_info.tm_mon;
	time->tm_year = tm_info.tm_year + 1900;
	time->tm_wday = tm_info.tm_wday;
	time->tm_yday = tm_info.tm_yday;
	time->tm_isdst = tm_info.tm_isdst;

	return 0;
}

static void time_thread_function(void *p1, void *p2, void *p3)
{
	struct rtc_time time;
	int err;

	while (1) {
        err = tracker_get_time(&time);
        if (err == 0) {
            k_msgq_purge(&time_msgq);
            k_msgq_put(&time_msgq, &time, K_NO_WAIT);
        }
        k_sleep(K_SECONDS(1));
    }
}

K_THREAD_DEFINE(time_thread, 1024, time_thread_function, NULL, NULL, NULL, 3, 0, 0);