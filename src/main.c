#include "camera_handler.h"
#include "camera_service.h"
#include "data_http_client.h"
#include "display.h"
#include "rtc.h"
#include "sensors.h"
#include "validate_plate.h"

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

	err = display_speed_init();
	if (err < 0) {
		LOG_ERR("Failed to initialize display: %d", err);
		return 0;
	}

	err = sensors_init();
	if (err < 0) {
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

			err = validate_and_process_speed(speed, time_valid, &latest_time);
			if (err < 0) {
				LOG_ERR("Failed to process speed data: %d", err);
				continue;
			}
		}
	}
}

static int validate_and_process_speed(int32_t speed, bool time_valid, struct rtc_time *latest_time)
{
	char timestamp[20];

	if (speed < 0 || speed > 500) {
		LOG_WRN("Invalid speed value: %d km/h - ignoring", speed);
		return -EINVAL;
	}

	char speed_text[32];
	snprintf(speed_text, sizeof(speed_text), "Speed: %d km/h", speed);

	int err = display_speed_show(speed_text);
	if (err < 0) {
		LOG_ERR("Failed to write speed to display: %d", err);
		return err;
	}

	if (speed > CONFIG_RADAR_SPEED_LIMIT_KMH) {
		LOG_INF("Speed violation detected! %d km/h", speed);

		if (time_valid && latest_time != NULL) {
			LOG_INF("Speed violation at %02d:%02d:%02d %02d/%02d/%04d",
				latest_time->tm_hour, latest_time->tm_min, latest_time->tm_sec,
				latest_time->tm_mday, latest_time->tm_mon + 1,
				latest_time->tm_year);
			const char *plate = NULL;
			const char *hash = NULL;
			char country[3];

			if (camera_handler_capture(&plate, &hash) == 0) {
				LOG_INF("Captured plate: %s", plate);
				LOG_DBG("Captured image hash: %s", hash);
				if (is_valid_mercosul_plate(plate, country)) {
					LOG_INF("Captured plate is valid and is from: %s", country);
					if (time_valid) {
						snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d %02d/%02d/%04d",
							latest_time->tm_hour, latest_time->tm_min, latest_time->tm_sec,
							latest_time->tm_mday, latest_time->tm_mon + 1,
							latest_time->tm_year);
					} else {
						strncpy(timestamp, "N/A", sizeof(timestamp));
					}
					err = send_infraction_data(speed, plate, country, timestamp);
					if (err < 0) {
						LOG_ERR("Failed to send infraction data: %d", err);
					} else {
						LOG_INF("Infraction data sent successfully");
					}
				} else {
					LOG_INF("Captured plate is invalid!");
					/*TODO: send to python server that the plate is not valid*/
					err = send_infraction_data(speed, plate, "N/A", "N/A");
					if (err < 0) {
						LOG_ERR("Failed to send infraction data for invalid plate: %d", err);
					} else {
						LOG_INF("Infraction data for invalid plate sent successfully");
					}
				}
			} else {
				LOG_ERR("Failed to capture plate");
				return -EIO;
			}
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
	}

	return 0;
}