#ifndef STRIP_SIXTEEN_RENDER_H
#define STRIP_SIXTEEN_RENDER_H

#include <esp_log.h>
#include "vector_types.h"
#include "math_extra.h"

#include "driver/rmt.h"
#include "led_strip.h"
#include "driver/ledc.h"

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

#define LOOKBACK_LED 1

#define RMT_TX_CHANNEL RMT_CHANNEL_0

#define EXAMPLE_CHASE_SPEED_MS (100)


#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (19)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE


#define LEDC_TEST_CH_NUM       (2)
#define LEDC_TEST_DUTY         (4000)
#define LEDC_TEST_FADE_TIME    (3000)


#ifndef PROG_MAIN


static rmt_config_t strip_rmt_config = RMT_DEFAULT_CONFIG_TX((unsigned int)5, RMT_TX_CHANNEL);;
static led_strip_config_t strip_led_config = LED_STRIP_DEFAULT_CONFIG(150, (led_strip_dev_t)1);;
static led_strip_t *strip;
#endif
void setup_sixteen_render();

void * render_step_sixteen_render(void *settings_v, void *stack_heights_v);


#endif