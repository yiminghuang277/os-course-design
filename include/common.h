#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

void print_header(const char *title);
void print_divider(void);
int prompt_int(const char *label, int min_value, int max_value);
double prompt_double(const char *label, double min_value, double max_value);
void prompt_string(const char *label, char *buffer, size_t size);
int prompt_yes_no(const char *label);

#endif
