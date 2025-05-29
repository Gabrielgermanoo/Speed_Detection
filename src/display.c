#include "display.h"

LOG_MODULE_REGISTER(display, LOG_LEVEL_INF);

const struct device *display_dev;

/**
 * @brief Render the given text on the display.
 *
 * @param text The text to display.
 * @return 0 on success, otherwise negative error code on failure.
 */
static int display_speed_write(const char *text);

/**
 * @brief Clear the speed display by writing a blank buffer to the display.
 *
 * @return 0 on success, otherwise negative error code on failure.
 */
static int display_speed_clear(void);

int display_speed_init(void)
{
	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready");
		return -ENODEV;
	}

	LOG_INF("Display device initialized: %s", display_dev->name);
	int ret = display_blanking_off(display_dev);
	if (ret < 0) {
		LOG_ERR("Failed to turn off display blanking: %d", ret);
		return ret;
	}
	LOG_INF("Display blanking turned off");
	return 0;
}

static int display_speed_clear(void)
{
	if (!display_dev) {
		LOG_ERR("Display device not initialized");
		return -ENODEV;
	}

	struct display_buffer_descriptor desc = {
		.buf_size = 1,
		.width = 320,
		.height = 240,
		.pitch = 320,
	};

	uint8_t clear_buf[1] = {0};
	return display_write(display_dev, 0, 0, &desc, clear_buf);
}

static int display_speed_write(const char *text)
{
	int err = 0;
	if (!display_dev) {
		LOG_ERR("Display device not initialized");
		return -ENODEV;
	}

	err = display_speed_clear();
	if (err < 0) {
		LOG_ERR("Failed to clear display: %d", err);
		return err;
	}

	struct display_buffer_descriptor desc = {
		.buf_size = strlen(text),
		.width = 320,
		.height = 240,
		.pitch = 320,
	};

	return display_write(display_dev, 0, 0, &desc, (const uint8_t *)text);
}

int display_speed_show(const char *text)
{
	if (!display_dev) {
		LOG_ERR("Display device not initialized");
		return -ENODEV;
	}

	int err = display_speed_write(text);
	if (err < 0) {
		LOG_ERR("Failed to write text to display: %d", err);
		return err;
	}

	LOG_INF("Speed: %s", text);
	return 0;
}