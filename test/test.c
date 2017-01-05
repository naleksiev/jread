/*
 * Copyright 2016-2017 Nikolay Aleksiev. All rights reserved.
 * License: https://github.com/naleksiev/jread/blob/master/LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/jread.h"

void handler(jr_type type, const jr_str* data, void* user_data) {
    switch (type) {
        case jr_type_error:
            printf("ERROR: '%.*s'\n", data->len, data->cstr);
            exit(1);
        default:
            return;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        return 1;
    FILE* fp = fopen(argv[1], "rb");
    if (!fp)
        return 1;
    fseek(fp, 0, SEEK_END);
    int32_t size = (int32_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (fread(buffer, 1, size, fp) < size)
        return 1;
    buffer[size] = '\0';
    fclose(fp);
    clock_t start = clock();
    jr_read(&handler, buffer, 0);
    printf("%.4f\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    free(buffer);
    return 0;
}
