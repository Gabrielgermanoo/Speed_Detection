#ifndef DISPLAY_H
#define DISPLAY_H
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>

/**
 * @brief Initialize the display device and turn off display blanking.
 *
 * @return 0 on success, otherwise negative error code on failure.
 */
int display_speed_init(void);

/**
 * @brief Show the speed on the display.
 *
 * @param text The text to display, typically the speed in km/h.
 * @return 0 on success, otherwise negative error code on failure.
 */
int display_speed_show(const char *text);

#endif /* DISPLAY_H */