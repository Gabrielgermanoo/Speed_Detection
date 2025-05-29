#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include "display.h"

extern const struct device *display_dev;

static struct device test_device = {
    .name = "TEST_DISPLAY"
};

static void *display_test_setup(void)
{
    display_dev = NULL;
    return NULL;
}

ZTEST_SUITE(display_tests, NULL, display_test_setup, NULL, NULL, NULL);

ZTEST(display_tests, test_display_speed_show_not_initialized)
{
    int ret = display_speed_show("100 km/h");
    
    zassert_equal(ret, -ENODEV, 
                 "display_speed_show should return -ENODEV when device not initialized");
}

ZTEST(display_tests, test_device_name)
{
    zassert_str_equal(test_device.name, "TEST_DISPLAY", 
                     "Test device should have name TEST_DISPLAY");
}

ZTEST(display_tests, test_display_show_velocity)
{
    const char *velocity = "80 km/h";
    
    ret = display_speed_show(NULL);
    zassert_equal(ret, -ENODEV, 
                 "display_speed_show should return-ENODEV for NULL text");
    
}