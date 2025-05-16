#include <zephyr/drivers/rtc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>

#include <time.h>
#include <stdlib.h>
#include <string.h>


/**
 * @brief 
 * 
 * @param time 
 * @return int 
 */
int tracker_get_time(struct rtc_time *time);

