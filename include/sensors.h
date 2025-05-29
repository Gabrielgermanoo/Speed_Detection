#ifndef SENSORS_H
#define SENSORS_H

#include "zephyr/zbus/zbus.h"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <stdbool.h>
#include <stdint.h>

struct sensor_event {
	int32_t speed_kmh;
	bool vehicle_detected;
};

/**
 * @brief Construct a new zbus chan declare object
 *
 */
ZBUS_CHAN_DECLARE(chan_sensors_evt);

/**
 * @brief Initialize the sensors and configure GPIO pins.
 *
 * @return 0 on success, otherwise negative error code on failure.
 */
int sensors_init(void);

/**
 * @brief Trigger a sensor manually for testing purposes.
 *
 * @param sensor_num Sensor number to trigger (1 or 2).
 * @return 0 on success, otherwise negative error code on failure.
 */
int sensors_simulate_vehicle_detection(int32_t speed_kmh);

#endif /* SENSORS_H */