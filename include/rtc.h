#include <zephyr/kernel.h>
#include <zephyr/net/sntp.h>
#include <zephyr/net/socket.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/settings/settings.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief 
 * 
 * @param time 
 * @return int 
 */
int tracker_get_time(struct rtc_time *time);

