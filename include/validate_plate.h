#ifndef VALIDATE_PLATE_H
#define VALIDATE_PLATE_H

#include <zephyr/kernel.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Validates a license plate and determines its country of origin.
 *
 * This function checks if the given license plate string matches the format
 * of a valid plate from Brazil (BR), Argentina (AR), or Paraguay (PY).
 * If valid, it copies the corresponding country code into the output parameter.
 *
 * @param plate        The license plate string to validate.
 * @param country_out  A pointer to a char array where the country code ("BR", "AR", "PY") will be stored if valid.
 *
 * @return true if the plate is valid and matches one of the known formats, false otherwise.
 */

 bool is_valid_mercosul_plate(const char *plate, char *country_out);

 #endif /* VALIDATE_PLATE_H */