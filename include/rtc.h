#ifndef RTC_H
#define RTC_H

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/sntp.h>
#include <zephyr/net/socket.h>
#include <zephyr/settings/settings.h>

#include <time.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Function to get the current time from SNTP server.
 *
 * @param time Pointer to the rtc_time structure to store the retrieved time.
 * @return int 0 on success, negative error code on failure.
 */
int tracker_get_time(struct rtc_time *time);

#endif /* RTC_H */