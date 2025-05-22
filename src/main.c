#include "camera_handler.h"
#include "camera_service.h"
#include "rtc.h" 
#include "sensors.h"
#include "zephyr/kernel.h"
#include "zephyr/zbus/zbus.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

ZBUS_SUBSCRIBER_DEFINE(sensor_consumer, 4);

static struct rtc_time latest_time;
static bool time_valid = false;

int main(void)
{
	int err;
	struct sensor_event *evt;

	int ret = sensors_init();

	if (ret < 0) {
		LOG_ERR("Failed to initialize sensors");
		return 0;
	}

	LOG_INF("Vehicle detection system started");

	evt = zbus_chan_msg(&chan_sensors_evt);
	if (evt == NULL) {
		LOG_ERR("Failed to get sensor event message");
		return 0;
	}

	while (1) {
		if (k_msgq_get(&time_msgq, &latest_time, K_NO_WAIT) == 0) {
			time_valid = true;
			LOG_DBG("Time updated: %02d:%02d:%02d %02d/%02d/%04d", latest_time.tm_hour,
				latest_time.tm_min, latest_time.tm_sec, latest_time.tm_mday,
				latest_time.tm_mon + 1, latest_time.tm_year);
		}

		k_sleep(K_MSEC(500));

		if (evt->vehicle_detected) {
			int32_t speed = evt->speed_kmh;
			LOG_INF("Vehicle detected at %d km/h", speed);

			if (speed > CONFIG_SPEED_LIMIT_KMH) {
				LOG_INF("Speed violation detected!");

				if (time_valid) {
					LOG_INF("Speed violation at %02d:%02d:%02d %02d/%02d/%04d",
						latest_time.tm_hour, latest_time.tm_min,
						latest_time.tm_sec, latest_time.tm_mday,
						latest_time.tm_mon + 1, latest_time.tm_year);
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

				const char *plate = NULL;
				const char *hash = NULL;
				if (camera_handler_capture(&plate, &hash) == 0) {
					LOG_INF("Captured plate: %s", plate);
					LOG_DBG("Captured image hash: %s", hash);
					/*TODO: send to python server*/
				} else {
					LOG_ERR("Failed to capture plate");
				}
			}
			
			err = zbus_chan_claim(&chan_sensors_evt, K_NO_WAIT);
			if (err < 0) {
				LOG_ERR("Failed to claim sensor event: %d", err);
				continue;
			}
			
			evt->vehicle_detected = false;
			
			err = zbus_chan_pub(&chan_sensors_evt, evt, K_NO_WAIT);
			if (err < 0) {
				LOG_ERR("Failed to publish sensor event: %d", err);
			}

			err = zbus_chan_finish(&chan_sensors_evt);
		}
	}
}