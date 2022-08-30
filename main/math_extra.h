#ifndef MATH_EXTRA_H
#define MATH_EXTRA_H

#include "vector_types.h"

float mx(float a, float b);
float mn(float a, float b);

int mxi(int a, int b);
int mni(int a, int b);

float float_mod(float a, float b);

float uf(float n, float h, float s, float l);

struct color_param* rgb_to_hsl(struct color_param* in);

struct position_xy transform_identity(struct position_xy in);

struct position_xy transform_rot_cc(struct position_xy in);

struct color_param* hsl_to_rgb(struct color_param* in);

int char_to_int(char c);

int hex_to_decimal(char l, char r);

int position_to_index(struct position_xy);

uint16_t byte_combine(char msbyte, char lsbyte);


#endif