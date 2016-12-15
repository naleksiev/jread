#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/jread.h"

void handler(jr_type type, const jr_str* data, void* user_data) {
    switch (type) {
        case jr_type_array_start:
            printf("[\n");
            return;
        case jr_type_array_end:
            printf("]\n");
            return;
        case jr_type_object_start:
            printf("{\n");
            return;
        case jr_type_object_end:
            printf("}\n");
            return;
        case jr_type_true:
            printf("<true>\n");
            return;
        case jr_type_false:
            printf("<false>\n");
            return;
        case jr_type_null:
            printf("<null>\n");
            return;
        case jr_type_number:
            printf("(%.*s)\n", data->len, data->cstr);
            return;
        case jr_type_string:
            printf("\"%.*s\"\n", data->len, data->cstr);
            return;
        case jr_type_key:
            printf("'%.*s'\n", data->len, data->cstr);
            return;
        case jr_type_error:
            printf("ERROR: char '%.*s'\n", data->len, data->cstr);
            exit(1);
            return;
    }
}

int main() {
    FILE* fp = fopen("test/test.json", "rb");
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
}

