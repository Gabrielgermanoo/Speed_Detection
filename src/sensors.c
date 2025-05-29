#include "sensors.h"
#include "zephyr/logging/log.h"
#include "zephyr/sys/util_macro.h"
#include "zephyr/zbus/zbus.h"

#define SENSOR_1 DEVICE_DT_GET(DT_ALIAS(sensor1))
#define SENSOR_2 DEVICE_DT_GET(DT_ALIAS(sensor2))

#define SENSOR1_PIN DT_GPIO_PIN(DT_NODELABEL(sensor1), gpios)
#define SENSOR2_PIN DT_GPIO_PIN(DT_NODELABEL(sensor2), gpios)

LOG_MODULE_REGISTER(sensors, LOG_LEVEL_DBG);

static int64_t sensor1_timestamp;
static int64_t sensor2_timestamp;
static int32_t latest_speed_kmh;
static bool vehicle_detected;
static bool system_initialized;

static struct gpio_callback sensor1_cb_data;
static struct gpio_callback sensor2_cb_data;

ZBUS_CHAN_DEFINE(chan_sensors_evt, struct sensor_event, NULL, NULL, ZBUS_OBSERVERS(sensor_consumer),
		 ZBUS_MSG_INIT(.speed_kmh = 0, .vehicle_detected = false));

/**
 * @brief Callback function for sensor 1.
 *
 * @param dev device pointer.
 * @param cb callback pointer.
 * @param pins pins that triggered the callback.
 */
static void sensor1_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

/**
 * @brief Callback function for sensor 2.
 *
 * @param dev device pointer.
 * @param cb callback pointer.
 * @param pins pins that triggered the callback.
 */
static void sensor2_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

/**
 * @brief Simulate vehicle detection for testing purposes.
 *
 * @param speed_kmh Speed in km/h to simulate.
 * @return 0 on success, otherwise negative error code on failure.
 */
int sensors_simulate_vehicle_detection(int32_t speed_kmh);

/**
 * @brief Trigger a sensor manually for testing purposes.
 *
 * @param sensor_num Sensor number to trigger (1 or 2).
 * @return 0 on success, otherwise negative error code on failure.
 */
int trigger_sensor_manually(int sensor_num);

static void sensor1_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	sensor1_timestamp = k_uptime_get();
	LOG_INF("Sensor 1 triggered at %lld ms", sensor1_timestamp);
	vehicle_detected = true;
}

static void sensor2_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	sensor2_timestamp = k_uptime_get();
	struct sensor_event evt = {
		.speed_kmh = latest_speed_kmh,
		.vehicle_detected = true,
	};
	int err = 0;

	LOG_INF("Sensor 2 triggered at %lld ms", sensor2_timestamp);

	int64_t time_diff_ms = sensor2_timestamp - sensor1_timestamp;

	if (time_diff_ms < 0) {
		LOG_ERR("Sensor 2 triggered before sensor 1");
		return;
	}

	if (IS_ENABLED(CONFIG_SYSTEM_SIMULATION)) {
		sensors_simulate_vehicle_detection(120);
		return;
	}

	if (time_diff_ms > 0) {
		float time_diff_s = time_diff_ms / 1000.0f;
		float speed_ms = CONFIG_RADAR_SENSOR_DISTANCE_MM / time_diff_s;
		latest_speed_kmh = (int32_t)(speed_ms * 3.6f);

		LOG_INF("Speed: %d km/h", latest_speed_kmh);

		err = zbus_chan_pub(&chan_sensors_evt, &evt, K_NO_WAIT);
		if (err < 0) {
			LOG_ERR("Failed to publish sensor event: %d", err);
		}
	} else {
		LOG_ERR("Invalid time difference");
	}
}

int sensors_init(void)
{
	int ret = 0;

	if (!device_is_ready(SENSOR_1) || !device_is_ready(SENSOR_2)) {
		LOG_ERR("Sensor devices not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure(SENSOR_1, SENSOR1_PIN, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure sensor 1 pin: %d", ret);
		return ret;
	}

	ret = gpio_pin_configure(SENSOR_2, SENSOR2_PIN, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure sensor 2 pin: %d", ret);
		return ret;
	}

	gpio_init_callback(&sensor1_cb_data, sensor1_callback, BIT(SENSOR1_PIN));
	ret = gpio_add_callback(SENSOR_1, &sensor1_cb_data);
	if (ret < 0) {
		LOG_ERR("Failed to add sensor 1 callback: %d", ret);
		return ret;
	}

	gpio_init_callback(&sensor2_cb_data, sensor2_callback, BIT(SENSOR2_PIN));
	ret = gpio_add_callback(SENSOR_2, &sensor2_cb_data);
	if (ret < 0) {
		LOG_ERR("Failed to add sensor 2 callback: %d", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure(SENSOR_1, SENSOR1_PIN, GPIO_INT_EDGE_RISING);
	if (ret < 0) {
		LOG_ERR("Failed to configure sensor 1 interrupt: %d", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure(SENSOR_2, SENSOR2_PIN, GPIO_INT_EDGE_FALLING);
	if (ret < 0) {
		LOG_ERR("Failed to configure sensor 2 interrupt: %d", ret);
		return ret;
	}

	LOG_INF("Sensors initialized");
	system_initialized = true;

	return 0;
}

int trigger_sensor_manually(int sensor_num)
{
	if (!system_initialized) {
		LOG_ERR("Sensors not initialized");
		return -EINVAL;
	}

	if (sensor_num == 1) {
		sensor1_callback(SENSOR_1, &sensor1_cb_data, BIT(SENSOR1_PIN));
	} else if (sensor_num == 2) {
		sensor2_callback(SENSOR_2, &sensor2_cb_data, BIT(SENSOR2_PIN));
	} else {
		LOG_ERR("Invalid sensor number: %d", sensor_num);
		return -EINVAL;
	}

	return 0;
}

int sensors_simulate_vehicle_detection(int32_t speed_kmh)
{
	static struct sensor_event evt_data;

	if (!system_initialized) {
		LOG_ERR("Sensors not initialized");
		return -EINVAL;
	}

	latest_speed_kmh = speed_kmh;
	vehicle_detected = true;

	evt_data.speed_kmh = latest_speed_kmh;
	evt_data.vehicle_detected = vehicle_detected;

	int err = zbus_chan_pub(&chan_sensors_evt, &evt_data, K_NO_WAIT);
	if (err < 0) {
		LOG_ERR("Failed to publish sensor event: %d", err);
		return err;
	}

	LOG_INF("Simulated vehicle detection at %d km/h", latest_speed_kmh);

	return 0;
}