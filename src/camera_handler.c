#include "camera_handler.h"
#include "camera_service.h"

LOG_MODULE_REGISTER(camera_handler, LOG_LEVEL_INF);

int camera_handler_capture(const char **plate, const char **hash)
{
	int ret = camera_api_capture(K_MSEC(500));
	if (ret < 0) {
		LOG_ERR("Failed to trigger camera capture");
		return ret;
	}

	struct msg_camera_evt msg;

	if (zbus_chan_read(&chan_camera_evt, &msg, K_MSEC(500)) != 0) {
		LOG_ERR("Timeout waiting for camera data");
		return -1;
	}

	if (msg.type == MSG_CAMERA_EVT_TYPE_DATA && msg.captured_data) {
		*plate = msg.captured_data->plate;
		*hash = msg.captured_data->hash;
		return 0;
	} else if (msg.type == MSG_CAMERA_EVT_TYPE_ERROR) {
		LOG_ERR("Camera error: %d", msg.error_code);
		return msg.error_code;
	}

	LOG_ERR("Invalid camera message type");
	return -1;
}
