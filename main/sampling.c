#include "sampling.h"


void i2s_init_usecase()
{
    i2s_driver_install(i2s_number, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_5);

    vTaskDelay(1000); //required for stability of ADC 
    i2s_adc_enable(i2s_number);
    vTaskDelay(1000);
}

//Read in a sample and return a pointer, put number of samples read into status
uint16_t* read_sample(int *status)
{
    size_t i2s_buffer_read_count;

    i2s_read(i2s_number, microphone_i2s_buffer, sizeof(microphone_i2s_buffer), &i2s_buffer_read_count, portMAX_DELAY);

    *status = i2s_buffer_read_count;
    
    return microphone_i2s_buffer;
}