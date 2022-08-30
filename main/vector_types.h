#ifndef VECTOR_TYPES_H
#define VECTOR_TYPES_H
#include <stdint.h>
//Render functions
enum mode {FFT_BARS, IMAGE, FFT_BARS_IMAGE};

//Settings application
enum led_hardware{MATRIX, STRIP, MATRIX_STRIP};

/*RGB or HSL*/
struct color_param
{
    float a;
    float b;
    float c;
};
/* 2d coords*/
struct position_xy
{
    int x;
    int y;
};

/*Settings for the device*/
struct device_settings
{
    //HSL color rotation angles
    float strip_hue_angle;
    int drop_time;
    int strip_hue_speed;

     //HSL color rotation angles
    float matrix_hue_angle;

    int matrix_hue_speed;

    //LED lights brightness
    float strip_brightness;
    //Matrix display brightness
    float matrix_brightness;
    //Microphone sensitivity when converting to drawable figures
    float sensitivity;

    //Current time since boot
    int64_t time_us;
    //Time of last render
    int64_t render_previous_time_us;
    //Time of lass mic read/process
    int64_t process_previous_time_us;
    //Color palette for respective devices
    struct color_param matrix_color_palette[16];
    struct color_param strip_color_palette[16];

    //Transform can be used by rendering
    struct position_xy (*transform_function)(struct position_xy);

    //Tasks to run as processing and drawing task
    void * (*process_task)(void *, void *, void *);
    void (*render_task)(void *, void *);

};
#endif