#ifndef SENSORS_H
#define SENSORS_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize the sensors and configure GPIO pins.
 * 
 * @return 0 on success, otherwise negative error code on failure.
 */
int sensors_init(void);


/**
 * @brief Get the latest speed in km/h.
 * 
 * @return Latest speed in km/h.
 */
int32_t sensors_get_speed(void);

/**
 * @brief Verify if a vehicle is detected.
 * 
 * @return true if a vehicle is detected, otherwise false.
 */
bool sensors_is_vehicle_detected(void);

#endif /* SENSORS_H */