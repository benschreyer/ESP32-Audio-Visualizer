#ifndef SAMPLING_H_BS
#define SAMPLING_H_BS
//i2s audio sample
#include "driver/i2s.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#ifndef PROG_MAIN
static const int i2s_number = 0; // i2s port number

static const i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
    .sample_rate = CONFIG_BI2C_SAMPLE_RATE,
    .bits_per_sample = 16, //max 12 bits
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .intr_alloc_flags = 0, // default interrupt priority
    .dma_desc_num = 8,
    .dma_frame_num = CONFIG_BI2C_NUM_SAMPLES,
    .use_apll = false
};


static uint16_t microphone_i2s_buffer[1024];
#endif
//Init for my hardware setup
void i2s_init_usecase();

//Read in a sample and return a pointer, put number of samples read into status
uint16_t* read_sample(int *status);
#endif