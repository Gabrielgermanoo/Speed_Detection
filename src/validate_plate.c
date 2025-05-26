#include "validate_plate.h"

static bool match_format(const char *plate, const char *format) {
    while (*plate && *format) {
        if (*format == 'A' && !isupper((unsigned char)*plate)) return false;
        if (*format == '9' && !isdigit((unsigned char)*plate)) return false;
        if (*format == ' ' && !isspace((unsigned char)*plate)) return false;
        plate++;
        format++;
    }
    return *plate == '\0' && *format == '\0';
}

static bool match_br(const char *plate) {
    if(strlen(plate) != 7)
    {
        return false;
    }
    return match_format(plate, "AAA9A99") || match_format(plate, "AAA9999");
}

static bool match_py(const char *plate) {
    if(strlen(plate) != 8)
    {
        return false;
    }
    return match_format(plate, "AAAA 999") || match_format(plate, "999 AAAA");
}

static bool match_ar(const char *plate) {
    if(strlen(plate) != 9)
    {
        return false;
    }
    return match_format(plate, "AA 999 AA");
}

bool is_valid_mercosul_plate(const char *plate, char *country_out) {
    if (match_br(plate)) {
        strcpy(country_out, "BR");
        return true;
    } else if (match_ar(plate)) {
        strcpy(country_out, "AR");
        return true;
    } else if (match_py(plate)) {
        strcpy(country_out, "PY");
        return true;
    }
    return false;
}
