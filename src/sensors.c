#include "sensors.h"
#include "zephyr/logging/log_core.h"

#define SENSOR_1 DEVICE_DT_GET(DT_NODELABEL(gpio5))
#define SENSOR_2 DEVICE_DT_GET(DT_NODELABEL(gpio6))

LOG_MODULE_REGISTER(sensors, LOG_LEVEL_DBG);

#define SENSOR1_PIN 5
#define SENSOR2_PIN 6

#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY 5

static int64_t sensor1_timestamp;
static int64_t sensor2_timestamp;
static int32_t latest_speed_kmh;
static bool vehicle_detected;
static bool system_initialized;

static K_THREAD_STACK_DEFINE(sensor1_stack, THREAD_STACK_SIZE);
static K_THREAD_STACK_DEFINE(sensor2_stack, THREAD_STACK_SIZE);
static K_THREAD_STACK_DEFINE(speed_calc_stack, THREAD_STACK_SIZE);

static struct k_thread sensor1_thread;