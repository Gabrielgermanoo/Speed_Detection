/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

 #include <stdio.h>
 #include <zephyr/logging/log.h>
 #include "sensors.h"
 #include "rtc.h"
 
 LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);
 
 #define SPEED_LIMIT_KMH 60
 
 int main(void)
 {
	 int ret = sensors_init();
 
	 if (ret < 0) {
		 LOG_ERR("Failed to initialize sensors");
		 return 0;
	 }
 
	 LOG_INF("Vehicle detection system started");
 
	 while (1) {
		 k_sleep(K_SECONDS(1));
 
		 if (sensors_is_vehicle_detected()) {
			 int32_t speed = sensors_get_speed();
			 LOG_INF("Vehicle detected at %d km/h", speed);
 
			 if (speed > SPEED_LIMIT_KMH) {
				 struct rtc_time now;
				 LOG_INF("Speed violation detected!");
 
				 ret = tracker_get_time(&now);
				 if (ret == 0) {
					 LOG_INF("Speed detected at %02d:%02d:%02d", now.tm_hour, now.tm_min, now.tm_sec);
 
				 } else {
					 LOG_ERR("Failed to get SNTP time");
				 }
			 }
		 }
	 }
 }