#include "rtc.h"
#include "sensors.h"
#include "syscalls/kernel.h"
#include "zephyr/kernel.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static struct rtc_time latest_time;
static bool time_valid = false;

int main(void)
{
	int ret = sensors_init();

	if (ret < 0) {
		LOG_ERR("Failed to initialize sensors");
		return 0;
	}

	LOG_INF("Vehicle detection system started");

	while (1) {
		if (k_msgq_get(&time_msgq, &latest_time, K_NO_WAIT) == 0) {
			time_valid = true;
			LOG_DBG("Time updated: %02d:%02d:%02d %02d/%02d/%04d", latest_time.tm_hour,
				latest_time.tm_min, latest_time.tm_sec, latest_time.tm_mday,
				latest_time.tm_mon + 1, latest_time.tm_year);
		}

		k_sleep(K_MSEC(500));

		if (sensors_is_vehicle_detected()) {
			int32_t speed = sensors_get_speed();
			LOG_INF("Vehicle detected at %d km/h", speed);

			if (speed > CONFIG_SPEED_LIMIT_KMH) {
				LOG_INF("Speed violation detected!");

				if (time_valid) {
					LOG_INF("Speed violation at %02d:%02d:%02d %02d/%02d/%04d",
						latest_time.tm_hour, latest_time.tm_min,
						latest_time.tm_sec, latest_time.tm_mday,
						latest_time.tm_mon + 1, latest_time.tm_year);

					/* TODO make the camera "take" a picture of car
					And send it to a web server (using python) */
				} else {
					struct rtc_time now;

					ret = tracker_get_time(&now);
					if (ret == 0) {
						LOG_INF("Speed detected at %02d:%02d:%02d",
							now.tm_hour, now.tm_min, now.tm_sec);

					} else {
						LOG_ERR("Failed to get SNTP time");
					}
				}
			}
			/* TODO show the value of the vehicle velocity */
		}
	}
}