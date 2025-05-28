#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/http/client.h>
#include <zephyr/net/socket.h>

#include "data_http_client.h"

LOG_MODULE_REGISTER(data_http_client, LOG_LEVEL_INF);

static char recv_buf_data[CONFIG_RECV_BUF_LEN];
static char response_buf[CONFIG_CLIENT_MAX_RECV_BUF_LEN];

/**
 * @brief Callback function to handle HTTP responses
 *
 * @param rsp Pointer to the HTTP response structure
 * @param final_data Final call status of the HTTP response
 * @param user_data User data pointer (not used in this case)
 */
static void response_cb(struct http_response *rsp, enum http_final_call final_data,
			void *user_data);

/**
 * @brief Prepare JSON data for sending infraction data
 *
 * @param buffer pointer to the buffer where JSON data will be stored
 * @param len size of the buffer
 * @param speed Speed in km/h
 * @param plate Plate number of the vehicle
 * @param country Respective country of the vehicle's plate
 * @param timestamp Timestamp of the infraction in "HH:MM:SS DD/MM/YYYY" format, or NULL if not
 * available
 * @return 0 if successful, otherwise negative error code
 */
static int prepare_json_data(char *buffer, size_t len, int32_t speed, const char *plate,
			     const char *country, const char *timestamp);

/**
 * @brief Fuction to connect to the HTTP server
 * 
 * @return 0 if successful, otherwise negative error code
 */
static int connect_http_client(void);

static void response_cb(struct http_response *rsp, enum http_final_call final_data, void *user_data)
{
	if (final_data == HTTP_DATA_FINAL) {
		LOG_INF("Response status %s (%d)", rsp->http_status, rsp->http_status_code);
	}

	if (rsp->body_frag_len > 0) {
		LOG_INF("Response body: %.*s", rsp->body_frag_len, (char *)rsp->body_frag_start);
	}
}

static int prepare_json_data(char *buffer, size_t len, int32_t speed, const char *plate,
			     const char *country, const char *timestamp)
{
	int offset = 0;

	offset = snprintf(buffer, len, "{\"speed\":%d,\"plate\":\"%s\",\"country\":\"%s\"", speed,
			  plate, country);

	if (timestamp != NULL) {
		offset +=
			snprintf(buffer + offset, len - offset, ",\"timestamp\":\"%s\"", timestamp);
	}

	offset += snprintf(buffer + offset, len - offset, "}");

	LOG_INF("offset: %d, buffer: %s", offset, buffer);

	return offset;
}

static int connect_http_client(void)
{
	int sock;
	int ret;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		LOG_ERR("Failed to create socket: %d", errno);
		return -errno;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(CONFIG_SERVER_PORT);

	ret = inet_pton(AF_INET, CONFIG_SERVER_HOSTNAME, &addr.sin_addr);
	if (ret <= 0) {
		LOG_ERR("Invalid server address: %s", CONFIG_SERVER_HOSTNAME);
		close(sock);
		return -EINVAL;
	}

	ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		LOG_ERR("Cannot connect to %s:%d (%d)", CONFIG_SERVER_HOSTNAME, CONFIG_SERVER_PORT,
			-errno);
		close(sock);
		return -errno;
	}

	return sock;
}

int send_infraction_data(int32_t speed, const char *plate, const char *country,
			 const char *timestamp)
{
	struct http_request req;
	int sock;
	int ret;
	char json_data[512];
	size_t json_len;

	json_len =
		prepare_json_data(json_data, sizeof(json_data), speed, plate, country, timestamp);

	LOG_INF("Sending infraction data: %s", json_data);

	sock = connect_http_client();
	if (sock < 0) {
		return sock;
	}

	static const char *headers[] = {"Content-Type: application/json\r\n",
					"Connection: close\r\n", "Accept: application/json\r\n",
					NULL};

	memset(&req, 0, sizeof(req));
	req.method = HTTP_POST;
	req.url = CONFIG_SERVER_URL;
	req.host = CONFIG_SERVER_HOSTNAME;
	req.protocol = "HTTP/1.1";
	req.payload = json_data;
	req.payload_len = json_len;
	req.header_fields = headers;
	req.response = response_cb;
	req.recv_buf = recv_buf_data;
	req.recv_buf_len = sizeof(recv_buf_data);

	ret = http_client_req(sock, &req, CONFIG_RECV_TIMEOUT_MS, response_buf);

	close(sock);

	if (ret < 0) {
		LOG_ERR("HTTP request failed: %d", ret);
		return ret;
	}

	LOG_INF("HTTP request completed, response length: %d", ret);
	return 0;
}