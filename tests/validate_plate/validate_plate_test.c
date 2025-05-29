#include "validate_plate.h"
#include <string.h>
#include <zephyr/ztest.h>

ZTEST_SUITE(test_br_plates, NULL, NULL, NULL, NULL, NULL);

ZTEST(test_br_plates, test_valid_br_plate)
{
	char country[3] = {0};
	bool res = is_valid_mercosul_plate("ABC1D23", country);
	zassert_true(res, "Expected valid BR plate");
	zassert_true(strcmp(country, "BR") == 0, "Expected country BR");
}

ZTEST(test_br_plates, test_invalid_br_plates)
{
	char country[3];
	bool res = is_valid_mercosul_plate("1BC1D23", country);
	zassert_false(res, "BR plate with number first should be rejected");

	res = is_valid_mercosul_plate("ABC1d23", country);
	zassert_false(res, "BR plate with lowercase should be rejected");
}

ZTEST_SUITE(test_ar_plates, NULL, NULL, NULL, NULL, NULL);

ZTEST(test_ar_plates, test_valid_ar_plate)
{
	char country[3] = {0};
	bool res = is_valid_mercosul_plate("AB 123 CD", country);
	zassert_true(res, "Expected valid AR plate");
	zassert_true(strcmp(country, "AR") == 0, "Expected country AR");
}

ZTEST(test_ar_plates, test_invalid_ar_plates)
{
	char country[3];
	bool res = is_valid_mercosul_plate("AB 123 C", country);
	zassert_false(res, "Short AR plate should be rejected");
}

ZTEST_SUITE(test_py_plates, NULL, NULL, NULL, NULL, NULL);

ZTEST(test_py_plates, test_valid_py_plate_car)
{
	char country[3] = {0};
	bool res = is_valid_mercosul_plate("ABCD 123", country);
	zassert_true(res, "Expected valid PY car plate");
	zassert_true(strcmp(country, "PY") == 0, "Expected country PY");

	res = is_valid_mercosul_plate("123 ABCD", country);
	zassert_true(res, "Expected valid PY motorcycle plate");
	zassert_true(strcmp(country, "PY") == 0, "Expected country PY");
}

ZTEST(test_py_plates, test_invalid_py_plates)
{
	char country[3];
	bool res = is_valid_mercosul_plate("ABCD 12", country);
	zassert_false(res, "Short PY plate should be rejected");
}

ZTEST_SUITE(validate_plate_tests, NULL, NULL, NULL, NULL, NULL);

ZTEST(validate_plate_tests, test_invalid_plate)
{
	char country[3] = {0};
	bool res = is_valid_mercosul_plate("INVALID", country);
	zassert_false(res, "Expected invalid plate");
}