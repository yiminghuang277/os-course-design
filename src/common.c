#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_header(const char *title) {
    printf("\n============================================================\n");
    printf("%s\n", title);
    printf("============================================================\n");
}

void print_divider(void) {
    printf("------------------------------------------------------------\n");
}

static void read_line(char *buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
}

int prompt_int(const char *label, int min_value, int max_value) {
    char buffer[128];
    long value;
    char *endptr;

    while (1) {
        printf("%s", label);
        read_line(buffer, sizeof(buffer));
        value = strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0') {
            printf("请输入有效整数。\n");
            continue;
        }
        if (value < min_value || value > max_value) {
            printf("请输入范围 [%d, %d] 内的数值。\n", min_value, max_value);
            continue;
        }
        return (int)value;
    }
}

double prompt_double(const char *label, double min_value, double max_value) {
    char buffer[128];
    double value;
    char *endptr;

    while (1) {
        printf("%s", label);
        read_line(buffer, sizeof(buffer));
        value = strtod(buffer, &endptr);
        if (endptr == buffer || *endptr != '\0') {
            printf("请输入有效数字。\n");
            continue;
        }
        if (value < min_value || value > max_value) {
            printf("请输入范围 [%.2f, %.2f] 内的数值。\n", min_value, max_value);
            continue;
        }
        return value;
    }
}

void prompt_string(const char *label, char *buffer, size_t size) {
    while (1) {
        printf("%s", label);
        read_line(buffer, size);
        if (buffer[0] != '\0') {
            return;
        }
        printf("输入不能为空。\n");
    }
}

int prompt_yes_no(const char *label) {
    char buffer[16];

    while (1) {
        printf("%s (y/n): ", label);
        read_line(buffer, sizeof(buffer));
        if (strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0) {
            return 1;
        }
        if (strcmp(buffer, "n") == 0 || strcmp(buffer, "N") == 0) {
            return 0;
        }
        printf("请输入 y 或 n。\n");
    }
}
