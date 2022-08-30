#ifndef WEBSITE_CONTROLLER_H
#define WEBSITE_CONTROLLER_H
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

#include "freertos/queue.h"
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

#include "vector_types.h"
#include "math_extra.h"


#define BUF_SIZE 4096

#define  CONFIG_EXAMPLE_IPV4

typedef struct {
    char    *username;
    char    *password;
} basic_auth_info_t;

#define HTTPD_401      "401 UNAUTHORIZED"           



#ifndef PROG_MAIN
static struct device_settings *settings_handle;

static char *http_auth_basic(const char *username, const char *password);
static esp_err_t basic_auth_get_handler(httpd_req_t *req);
static esp_err_t hello_get_handler(httpd_req_t *req);
static esp_err_t echo_post_handler(httpd_req_t *req);
char* buf;
static void httpd_register_basic_auth(httpd_handle_t server);


esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);

static httpd_handle_t start_webserver(void);
static void stop_webserver(httpd_handle_t server);
static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);
static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
#endif


void website_controller_init(struct device_settings *handle);

#endif