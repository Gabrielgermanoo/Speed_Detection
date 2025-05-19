#include "rtc.h"
#include "zephyr/init.h"
#include <zephyr/posix/arpa/inet.h>

LOG_MODULE_REGISTER(rtc, LOG_LEVEL_DBG);

#define RTC_DEVICE DEVICE_DT_GET(DT_NODELABEL(rtc))

#define SYS_TIME_SETTINGS_KEY "data"

#define SYS_TIME_SETTINGS_PREFIX "sys_time"

static struct sys_time {
	char server[64];
	int8_t timezone;
} self = {.server = "pool.ntp.org", .timezone = 0};

int tracker_get_time(struct rtc_time *time)
{
	struct sntp_ctx ctx;
	struct sntp_time datetime;
	struct sockaddr_in server_addr;
	int err = 0;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(123);

	if (strlen(self.server) == 0) {
		strcpy(self.server, "pool.ntp.org");

		err = settings_save_one(SYS_TIME_SETTINGS_PREFIX "/" SYS_TIME_SETTINGS_KEY, &self,
					sizeof(self));
		if (err < 0) {
			LOG_ERR("Cannot store sys_time on flash: %d", err);
			return err;
		}
	}

	inet_pton(AF_INET, self.server, &server_addr.sin_addr);

	err = sntp_init(&ctx, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));

	if (err < 0) {
		LOG_ERR("Failed to initialize SNTP: %d", err);
		return err;
	}

	err = sntp_query(&ctx, 4 * MSEC_PER_SEC, &datetime);

	if (err < 0) {
		LOG_ERR("Failed to query SNTP: %d", err);
		return err;
	}

	sntp_close(&ctx);

	return 0;
}

K_THREAD_DEFINE(rtc_thread, 1024, tracker_get_time, NULL, NULL, NULL, 3, 0, 0);