#include "camera_handler.h"
#include "camera_service.h"
#include "rtc.h"
#include "sensors.h"
#include "zephyr/kernel.h"
#include "zephyr/zbus/zbus.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

ZBUS_MSG_SUBSCRIBER_DEFINE(sensor_consumer);

static struct rtc_time latest_time;
static bool time_valid = false;

/**
 * @brief Validate the speed data and process it if valid.
 *
 * @param speed Speed in km/h to validate.
 * @param time_valid Indicates if the time data is valid.
 * @param latest_time Pointer to the latest time structure.
 * @return int
 */
static int validate_and_process_speed(int32_t speed, bool time_valid, struct rtc_time *latest_time);

int main(void)
{
	int err;
	struct sensor_event evt;
	const struct zbus_channel *evt_chan;
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

#ifdef CONFIG_SYSTEM_SIMULATION
		err = sensors_simulate_vehicle_detection(90);
		if (err < 0) {
			LOG_ERR("Failed to simulate vehicle detection: %d", err);
		}
#endif

		err = zbus_sub_wait_msg(&sensor_consumer, &evt_chan, &evt, K_FOREVER);
		if (err < 0) {
			if (err == -EAGAIN) {
				continue;
			}

			LOG_ERR("Failed to wait for sensor event: %d", err);
			continue;
		}

		if (evt.vehicle_detected) {
			int32_t speed = evt.speed_kmh;

			ret = validate_and_process_speed(speed, time_valid, &latest_time);
		}
	}
}

static int validate_and_process_speed(int32_t speed, bool time_valid, struct rtc_time *latest_time)
{
	if (speed < 0 || speed > 500) {
		LOG_WRN("Invalid speed value: %d km/h - ignoring", speed);
		return -EINVAL;
	}

	if (speed > CONFIG_RADAR_SPEED_LIMIT_KMH) {
		LOG_INF("Speed violation detected! %d km/h", speed);

		if (time_valid && latest_time != NULL) {
			LOG_INF("Speed violation at %02d:%02d:%02d %02d/%02d/%04d",
				latest_time->tm_hour, latest_time->tm_min, latest_time->tm_sec,
				latest_time->tm_mday, latest_time->tm_mon + 1,
				latest_time->tm_year);
			/* TODO send the data to a python server */
		} else {
			struct rtc_time now;

			int ret = tracker_get_time(&now);
			if (ret == 0) {
				LOG_INF("Speed detected at %02d:%02d:%02d", now.tm_hour, now.tm_min,
					now.tm_sec);
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
			return 1;
		} else {
			LOG_ERR("Failed to capture plate");
			return -EIO;
		}
	}

	return 0;
}