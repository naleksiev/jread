/*
 * Copyright 2016 Nikolay Aleksiev. All rights reserved.
 * License: https://github.com/naleksiev/jread/blob/master/LICENSE
 */

#ifndef __JREAD_H__
#define __JREAD_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum jr_type {
    jr_type_error,
    jr_type_null,
    jr_type_true,
    jr_type_false,
    jr_type_number,
    jr_type_string,
    jr_type_array_start,
    jr_type_array_end,
    jr_type_object_start,
    jr_type_object_end,
} jr_type;

typedef struct jr_str {
    const char* cstr;
    int32_t     len;
} jr_str;

typedef void (*jr_callback)(jr_type type, const jr_str* data, void* user_data);

void jr_read(jr_callback cb, const char* doc, void* user_data);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __JREAD_H__

