#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/**
 * @brief Captures a simulated image via camera.
 *
 * Triggers the camera service via ZBUS, waits for a response, and returns the data.
 *
 * @param[out] plate Pointer to the captured license plate string.
 * @param[out] hash Pointer to the image hash string.
 * @return 0 on success, negative value on error.
 */
int camera_handler_capture(const char **plate, const char **hash);
