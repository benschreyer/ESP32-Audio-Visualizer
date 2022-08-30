
#define PROG_MAIN

#include <time.h>
#include "sdkconfig.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>


#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "driver/uart.h"

#include "esp_err.h"
#include "driver/ledc.h"
//FFT
#include "fft.h"
#include "vector_types.h"
#include "math_extra.h"

#include <math.h>

//#include "freertos/semphr.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "driver/rmt.h"
#include "led_strip.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "addr_from_stdin.h"

//i2s audio sample
#include "sampling.h"
//fft proc
#include "processing_fft_default.h"
//render
#include "bar_render.h"
#include "strip_sixteen_render.h"

#include "website_controller.h"

#include "sdkconfig.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>


#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "driver/uart.h"

#include "esp_err.h"
#include "driver/ledc.h"

#include <math.h>

//#include "freertos/semphr.h"
#include <stdio.h>

#include "driver/rmt.h"
#include "led_strip.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "addr_from_stdin.h"

//i2s audio sample
#include "driver/i2s.h"
#include "freertos/queue.h"

//RGB LED TWO DEFINES


#define HOST_IP_ADDR ""//broadcast local ipv4 for your router
#define  CONFIG_EXAMPLE_IPV4

#define PORT //magic packet wake port usually 9 

static struct device_settings settings_global =
{
    0.0, 6, 9, 0.0, 9, 2.0, 2.0, 8.0, 0, 0, 0, {{ 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, }, {{ 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255}, { 50,50,255},}, NULL, NULL, NULL  };

int* stack_data_global;

void sample_process_task(void * pvParameters)
{
    

    int data_read_count = 0;
    uint16_t *i2s_read;


    while(1)
    {
        
        i2s_read = read_sample(&data_read_count);
         //ESP_LOGI("SAMPLE", "SAMPLE");
        if(data_read_count  > 1020)
        {
            //ESP_LOGI("PROC", "PROC");
            stack_data_global = (int *)(settings_global.process_task((void *)&data_read_count, (void *)i2s_read, (void *)&settings_global));//process_step_fft_default(data_read_count, i2s_read, &settings_global);
        }
    }
}

void draw_task(void * pvParameters)
{
    
    while(1)
    {
        
        //ESP_LOGI("RENDER", "RENDER");
        //printf("%f \n", settings_global.matrix_hue_angle);
        settings_global.render_task((void *)&settings_global, (void *)stack_data_global);
        render_step_sixteen_render((void *)&settings_global, (void *)stack_data_global);
    }
}

void app_main(void)
{
    TaskHandle_t xHandle = NULL;
    static uint8_t ucParameterToPass;
    website_controller_init(&settings_global);
    i2s_init_usecase();
    settings_global.process_task = process_step_fft_default;
    settings_global.render_task = render_step_bar_render;
    settings_global.transform_function = transform_rot_cc;
    stack_data_global = setup_processing_fft_default();
    setup_bar_render_default();
    setup_sixteen_render();
    xTaskCreatePinnedToCore(sample_process_task, "PROCTASK", 50096, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle, 0);
    xTaskCreatePinnedToCore(draw_task , "DRAWTASK", 10096, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle, 1);

}