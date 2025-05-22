#ifndef SENSORS_H
#define SENSORS_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include "zephyr/zbus/zbus.h"

#include <stdint.h>
#include <stdbool.h>

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

#endif /* SENSORS_H */