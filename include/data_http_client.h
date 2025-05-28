#ifndef DATA_HTTP_CLIENT_H
#define DATA_HTTP_CLIENT_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/http/client.h>
#include <zephyr/net/socket.h>

#include <stdint.h>

/**
 * @brief Send infraction data to the server.
 *
 * @param speed Speed in km/h of the vehicle.
 * @param plate Vehicle license plate number.
 * @param country Country code of the vehicle.
 * @param timestamp Timestamp of the infraction in ISO 8601 format (YYYY-MM-DDTHH:MM:SSZ).
 * @return 0 on success, negative error code on failure.
 */
int send_infraction_data(int32_t speed, const char *plate, const char *country,
			 const char *timestamp);

#endif // DATA_HTTP_CLIENT_H