#include "sensors.h"

#define SENSOR_1 DEVICE_DT_GET(DT_NODELABEL(gpio5))
#define SENSOR_2 DEVICE_DT_GET(DT_NODELABEL(gpio6))

#define SENSOR1_PIN 5
#define SENSOR2_PIN 6

LOG_MODULE_REGISTER(sensors, LOG_LEVEL_DBG);

static int64_t sensor1_timestamp;
static int64_t sensor2_timestamp;
static int32_t latest_speed_kmh;
static bool vehicle_detected;
static bool system_initialized;

static K_THREAD_STACK_DEFINE(speed_calc_stack, CONFIG_THREAD_STACK_SIZE);

static struct k_thread speed_calc_thread;

static K_SEM_DEFINE(measurement_sem, 0, 1);

static struct gpio_callback sensor1_cb_data;
static struct gpio_callback sensor2_cb_data;

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
 * @brief Thread entry function for speed calculation.
 *
 * @param arg1 Unused argument.
 * @param arg2 Unused argument.
 * @param arg3 Unused argument.
 */
static void speed_calc_thread_entry(void *arg1, void *arg2, void *arg3);


static void sensor1_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    sensor1_timestamp = k_uptime_get();
    LOG_INF("Sensor 1 triggered at %lld ms", sensor1_timestamp);
    vehicle_detected = true;
}

static void sensor2_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    sensor2_timestamp = k_uptime_get();
    LOG_INF("Sensor 2 triggered at %lld ms", sensor2_timestamp);

    k_sem_give(&measurement_sem);
}

static void speed_calc_thread_entry(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);
	int64_t time_diff_ms = 0;
	float speed_ms = 0.0f;
	float time_diff_s = 0.0f;

	while (1) {
		k_sem_take(&measurement_sem, K_FOREVER);

		time_diff_ms = sensor2_timestamp - sensor1_timestamp;
		if (time_diff_ms > 0) {
			time_diff_s = time_diff_ms / 1000.0f;
			speed_ms = CONFIG_SENSOR_DISTANCE_MM / time_diff_s;
			latest_speed_kmh = (int32_t)(speed_ms * 3.6f);

			LOG_INF("Speed: %d km/h", latest_speed_kmh);
		} else {
			LOG_ERR("Invalid time difference");
		}
	}
}

int sensors_init(void)
{
	int ret = 0;

	if (!device_is_ready(SENSOR_1) || !device_is_ready(SENSOR_2)) {
		LOG_ERR("Sensor devices not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure(SENSOR_1, SENSOR1_PIN, GPIO_INPUT | GPIO_PULL_UP);

	if (ret < 0) {
		LOG_ERR("Failed to configure sensor 1 pin: %d", ret);
		return ret;
	}

	gpio_init_callback(&sensor1_cb_data, sensor1_callback, BIT(SENSOR1_PIN));
	ret = gpio_add_callback(SENSOR_1, &sensor1_cb_data);

	if (ret < 0) {
		LOG_ERR("Failed to add sensor 1 callback: %d", ret);
		return ret;
	}

	gpio_init_callback(&sensor1_cb_data, sensor2_callback, BIT(SENSOR2_PIN));
	ret = gpio_add_callback(SENSOR_2, &sensor2_cb_data);

	if (ret < 0) {
		LOG_ERR("Failed to add sensor 2 callback: %d", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure(SENSOR_1, SENSOR1_PIN, GPIO_INT_EDGE_FALLING);

	if (ret < 0) {
		LOG_ERR("Failed to configure sensor 1 interrupt: %d", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure(SENSOR_2, SENSOR2_PIN, GPIO_INT_EDGE_FALLING);

	if (ret < 0) {
		LOG_ERR("Failed to configure sensor 2 interrupt: %d", ret);
		return ret;
	}

	k_thread_create(&speed_calc_thread, speed_calc_stack, CONFIG_THREAD_STACK_SIZE,
			speed_calc_thread_entry, NULL, NULL, NULL, CONFIG_THREAD_PRIORITY, 0,
			K_NO_WAIT);

	LOG_INF("Sensors initialized");
	system_initialized = true;
	return 0;
}

int32_t sensors_get_speed(void)
{
    return latest_speed_kmh;
}

bool sensors_is_vehicle_detected(void)
{
    return vehicle_detected;
}
