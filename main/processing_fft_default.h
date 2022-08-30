#ifndef PROCESSING_FFT_DEFAULT_H_BS
#define PROCESSING_FFT_DEFAULT_H_BS
#include "fft.h"
#include "vector_types.h"
#include "math_extra.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

#ifndef PROG_MAIN
static int fft_default_decay_stack[16];


static const int buckets[17] = {1, 2, 3, 4, 5, 6, 7, 9, 13, 23, 46, 90, 148, 208, 268, 350,512};
   //int buckets[17] = {7, 9, 14, 21, 33, 53, 78, 106, 135, 166, 196, 226, 257, 292, 335, 397,512};
   //int buckets[17] = {1, 2, 3, 4, 5, 6, 7, 13, 25, 51, 93, 139, 187, 234, 286, 358, 512};
   //int  buckets[17] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 13, 18, 27, 47, 84, 166, 334, 512};
   //int  buckets[17] = {0, 1, 2, 3, 4, 6, 8, 10, 13, 16, 28, 40, 67, 114, 196, 360, 512};

static float sound[1024];
static float spectrum[1024];

static fft_config_t *fft_analysis;

static int decay_count;
static int counter;
static int64_t last_decay;
#endif
void* setup_processing_fft_default();

void* process_step_fft_default(void*  samples_read, void* i2s_b_v, void *settings_v);
#endif
