#include "math_extra.h"

//Max and min floats
float mx(float a, float b)
{
    if(a > b)
        return a;
    return b;
}

float mn(float a, float b)
{
    if(a > b)
        return b;
    return a;
}

int mxi(int a, int b)
{
    if(a > b)
        return a;
    return b;
}

int mni(int a, int b)
{
    if(a > b)
        return b;
    return a;
}

//"Mod" operation on floats
float float_mod(float a, float b)
{
    int rem = (int)(a / b);
    return a - b * rem;
}

//Utility for HSL <-> RGB
float uf(float n, float h, float s, float l)
{

    float k = n + h /30.0;
    int k_12 = (int)(k / 12.0);

    k = k - k_12 * 12.0;

    float a = s * mn(l, 1 - l);

    return l - a * mx(-1, mn(mn (k - 3.0, 9.0 - k),1));
}

struct color_param* rgb_to_hsl(struct color_param* in)
{
    float max_comp = mx(mx(in->a,in->b),in->c) ;
    float min_comp = mn(mn(in->a,in->b),in->c);
    float chroma = max_comp -  min_comp;

    float hue =0.0;
    if(max_comp == in->a)
    {
      hue =  (in->b - in->c)/chroma;

      int hue_6 = (int)(hue / 6.0);

      hue = hue - 6.0 * hue_6;
    }
    else if(max_comp == in->b)
    {
      hue = (in->c - in->a)/chroma   + 2.0;
    }
    else
    {
       hue = (in->a - in->b)/chroma + 4.0;
    }
    hue = hue * 60.0;

    

    float luminance  =0.5 * (max_comp + min_comp);

    float saturation = 0.0;

    if(luminance < 0.001)
    {
        saturation = 0.0;
    }
    else if(luminance < 0.5)
    {
        saturation = chroma / (2 * luminance);
    }
    else
    {
        saturation = chroma / (2 - 2 * luminance);
    }

    in->a = hue;
    in->b = saturation;
    in->c = luminance;

    return in;
}


struct color_param* hsl_to_rgb(struct color_param* in)
{
    float h = in->a;
    float s = in->b;
    float l = in->c;
    in->a = uf(0,h,s,l);
    in->b = uf(8,h,s,l);
    in->c = uf(4,h,s,l);
    return in;
}

int char_to_int(char c)
{
    switch(c)
    {
        case '0':
            return 0;
        break;

        case '1':
            return 1;
        break;

        case '2':
            return 2;
        break;

        case '3':
            return 3;
        break;

        case '4':
            return 4;
        break;

        case '5':
            return 5;
        break;

        case '6':
            return 6;
        break;

        case '7':
            return 7;
        break;

        case '8':
            return 8;
        break;

        case '9':
            return 9;
        break;

        case 'a':
            return 10;
        break;

        case 'b':
            return 11;
        break;
        
        case 'c':
            return 12;
        break;

        case 'd':
            return 13;
        break;

        case 'e':
            return 14;
        break;

        case 'f':
            return 15;
        break;

    }
    return -22;
}

int hex_to_decimal(char l, char r)
{
    int left = char_to_int(l);
    int right = char_to_int(r);
    return 16 * left + right;
}

//2d array coordinates to 1d, with mod tweaks because the LED matrix snakes across and bacl instead of filling like text L->R, T->B
int position_to_index(struct position_xy pos)
{
    int y = pos.y;
    int x = (15 - pos.x);
    return x * 16 + (15 - y) * (x % 2)  + y * (-1 * (x % 2 - 1));
}

struct position_xy transform_identity(struct position_xy in)
{
    return in;
}

struct position_xy transform_rot_cc(struct position_xy in)
{
    int temp = in.y;
    in.y  = in.x;
    in.x = 15 - temp;
    return in;
}

uint16_t byte_combine(char msbyte, char lsbyte)
{
    return (uint16_t)(((uint16_t)lsbyte) | (((uint16_t)msbyte)<<8));
}