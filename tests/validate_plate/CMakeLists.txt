cmake_minimum_required(VERSION 3.20.0) find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
	project(validate_plate_tests)

		include_directories(../../ include)

			target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} /../../ src /
				       validate_plate.c ${CMAKE_CURRENT_SOURCE_DIR} /
				       validate_plate_test.c)
