/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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


#include "esp_err.h"
#include "driver/ledc.h"
//FFT
#include "fft.c"
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


static const int i2s_num = 0; // i2s port number

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

//RGB LED TWO DEFINES
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

/*static bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg)
{
    portBASE_TYPE taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT) {
        SemaphoreHandle_t counting_sem = (SemaphoreHandle_t) user_arg;
        xSemaphoreGiveFromISR(counting_sem, &taskAwoken);
    }

    return (taskAwoken == pdTRUE);
}
*/

#define HOST_IP_ADDR ""//broadcast local ipv4 for your router
#define  CONFIG_EXAMPLE_IPV4

#define PORT //magic packet wake port usually 9


/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static const char *TAG = "example";

char setting1 = 'a';
//No comments since this is just mashing two espressif example programs together http_server for WAN control(home routers often do not act as broadcast servers so you need a device like an RPI or ESP32) and udp_client to send the magic packet and boot 
#define BUF_SIZE 1024*32
char* buf;

#if CONFIG_EXAMPLE_BASIC_AUTH

typedef struct {
    char    *username;
    char    *password;
} basic_auth_info_t;

#define HTTPD_401      "401 UNAUTHORIZED"           /*!< HTTP Response 401 */

static char *http_auth_basic(const char *username, const char *password)
{
    int out;
    char *user_info = NULL;
    char *digest = NULL;
    size_t n = 0;
    asprintf(&user_info, "%s:%s", username, password);
    if (!user_info) {
        ESP_LOGE(TAG, "No enough memory for user information");
        return NULL;
    }
    esp_crypto_base64_encode(NULL, 0, &n, (const unsigned char *)user_info, strlen(user_info));

    /* 6: The length of the "Basic " string
     * n: Number of bytes for a base64 encode format
     * 1: Number of bytes for a reserved which be used to fill zero
    */
    digest = calloc(1, 6 + n + 1);
    if (digest) {
        strcpy(digest, "Basic ");
        esp_crypto_base64_encode((unsigned char *)digest + 6, n, (size_t *)&out, (const unsigned char *)user_info, strlen(user_info));
    }
    free(user_info);
    return digest;
}

/* An HTTP GET handler */
static esp_err_t basic_auth_get_handler(httpd_req_t *req)
{
    char *buf = NULL;
    size_t buf_len = 0;
    basic_auth_info_t *basic_auth_info = req->user_ctx;

    buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
    if (buf_len > 1) {
        buf = calloc(1, buf_len);
        if (!buf) {
            ESP_LOGE(TAG, "No enough memory for basic authorization");
            return ESP_ERR_NO_MEM;
        }

        if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Authorization: %s", buf);
        } else {
            ESP_LOGE(TAG, "No auth value received");
        }

        char *auth_credentials = http_auth_basic(basic_auth_info->username, basic_auth_info->password);
        if (!auth_credentials) {
            ESP_LOGE(TAG, "No enough memory for basic authorization credentials");
            free(buf);
            return ESP_ERR_NO_MEM;
        }

        if (strncmp(auth_credentials, buf, buf_len)) {
            ESP_LOGE(TAG, "Not authenticated");
            httpd_resp_set_status(req, HTTPD_401);
            httpd_resp_set_type(req, "application/json");
            httpd_resp_set_hdr(req, "Connection", "keep-alive");
            httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Hello\"");
            httpd_resp_send(req, NULL, 0);
        } else {
            ESP_LOGI(TAG, "Authenticated!");
            char *basic_auth_resp = NULL;
            httpd_resp_set_status(req, HTTPD_200);
            httpd_resp_set_type(req, "application/json");
            httpd_resp_set_hdr(req, "Connection", "keep-alive");
            asprintf(&basic_auth_resp, "{\"authenticated\": true,\"user\": \"%s\"}", basic_auth_info->username);
            if (!basic_auth_resp) {
                ESP_LOGE(TAG, "No enough memory for basic authorization response");
                free(auth_credentials);
                free(buf);
                return ESP_ERR_NO_MEM;
            }
            httpd_resp_send(req, basic_auth_resp, strlen(basic_auth_resp));
            free(basic_auth_resp);
        }
        free(auth_credentials);
        free(buf);
    } else {
        ESP_LOGE(TAG, "No auth header received");
        httpd_resp_set_status(req, HTTPD_401);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_set_hdr(req, "Connection", "keep-alive");
        httpd_resp_set_hdr(req, "WWW-Authenticate", "Basic realm=\"Hello\"");
        httpd_resp_send(req, NULL, 0);
    }

    return ESP_OK;
}

static httpd_uri_t basic_auth = {
    .uri       = "/basic_auth",
    .method    = HTTP_GET,
    .handler   = basic_auth_get_handler,
};

static void httpd_register_basic_auth(httpd_handle_t server)
{
    basic_auth_info_t *basic_auth_info = calloc(1, sizeof(basic_auth_info_t));
    if (basic_auth_info) {
        basic_auth_info->username = CONFIG_EXAMPLE_BASIC_AUTH_USERNAME;
        basic_auth_info->password = CONFIG_EXAMPLE_BASIC_AUTH_PASSWORD;

        basic_auth.user_ctx = basic_auth_info;
        httpd_register_uri_handler(server, &basic_auth);
    }
}
#endif

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }    

    time_t cur = time(NULL);



    if(1)
    {
        printf("boot time was <20\n");
        //xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
        
    }
    else
    {
        printf("no boot time was >20\n");
    }
    //boot_last_time = (long int)cur;
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/hello",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "<!DOCTYPE html><style>@import url(\'https://fonts.googleapis.com/css?family=Barlow|Patua+One\');\n.settingBackground\n{\n\tbackground-color: #e9e9ed;\n\tmargin-left:0;\n\tmargin-right:0;\n\twidth:50vw;\n\ttext-align: center;\n}\n\ninput[type=range] {\n\tmargin-bottom: 1vw;\n\tmargin-top: 1vw;\n\twidth:48vw;\n  }\n  input[type=range]:focus {\n\toutline: none;\n  }\n  input[type=range]::-webkit-slider-runnable-track {\n\twidth: 100%;\n\theight: 1.7vw;\n\tcursor: pointer;\n\tanimate: 0.2s;\n\n\tbackground: #bcbcbe;\n\tborder-radius: 0.3vw;\n\tborder: 0.1vw solid #000000;\n  }\n  input[type=range]::-webkit-slider-thumb {\n\n\tborder: 0.1vw solid #000000;\n\theight: 2vw;\n\twidth: 2vw;\n\tborder-radius: 0.3vw;\n\tbackground: #0066ff;;\n\tcursor: pointer;\n\t-webkit-appearance: none;\n\n  }\n  input[type=range]:focus::-webkit-slider-runnable-track {\n\tbackground: #a93085;\n  }\n  input[type=range]::-moz-range-track {\n\twidth: 100%;\n\theight: 1.7vw;\n\tcursor: pointer;\n\tanimate: 0.2s;\n\n\tbackground: #bcbcbe;\n\tborder-radius: 0.3vw;\n\tborder: 0.1vw solid #000000;\n  }\n  input[type=range]::-moz-range-thumb {\n\n\tborder: 0.1vw solid #000000;\n\theight: 2vw;\n\twidth: 2vw;\n\tborder-radius: 0.3vw;\n\tbackground: #0066ff;;\n\tcursor: pointer;\n  }\n  input[type=range]::-ms-track {\n\twidth: 100%;\n\theight: 10px;\n\tcursor: pointer;\n\tanimate: 0.2s;\n\tbackground: transparent;\n\tborder-color: transparent;\n\tcolor: transparent;\n  }\n  input[type=range]::-ms-fill-lower {\n\tbackground: #a93099;\n\tborder: 1px solid #000000;\n\tborder-radius: 10px;\n\tbox-shadow: 1px 1px 1px #000000;\n  }\n  input[type=range]::-ms-fill-upper {\n\tbackground: #a93085;\n\tborder: 1px solid #000000;\n\tborder-radius: 10px;\n\tbox-shadow: 1px 1px 1px #000000;\n  }\n  input[type=range]::-ms-thumb {\n\tbox-shadow: 1px 1px 1px #000000;\n\tborder: 1px solid #000000;\n\theight: 2vw;\n\twidth: 2vw;\n\tborder-radius: 1vw;\n\tbackground: #f01d1d;\n\tcursor: pointer;\n  }\n  input[type=range]:focus::-ms-fill-lower {\n\tbackground: #a9308b;\n  }\n  input[type=range]:focus::-ms-fill-upper {\n\tbackground: #a97f30;\n  }\n  \n\nlabel\n{\n\tfloat:left;\n\tmargin-top:auto;\n\tmargin-bottom: auto;\n\tmargin-left:0.3vw;\n\tfont-family:\'Barlow\', sans-serif;;\n\tfont-size: 2vw;\n}\n\nselect\n{\n\theight:3.125vw;\n\twidth:9.375vw;\n\tmargin-bottom: 1vw;\n\tfont-size: 2vw;\n\tfont-family:\'Barlow\', sans-serif;;\n}\nbutton\n{\n\tfont-size: 2.5vw;\n\tcolor:black;\n\twidth: 50vw;\n\theight:3.125vw;\n\tbackground-color: #97f26a;\n}\nbutton:active:hover{\n\tbackground-color: #4e8831;\n}\nbutton:hover\n{\n\tbackground-color: #7ecc56;\n}\n.hidden\n{\n\tdisplay:none;\n}\n.colorBox{\n    margin:0px;\n\tmargin-bottom: 1vw;\n    padding:0px;\n    outline:0px solid transparent;\n\twidth:3.125vw;\n\theight:3.125vw;\n}\n.colorDiv\n{\n\tmargin-left:0px;\n\tmargin-right:0px;\n\twidth:100%;\n}\n\t\tdiv{\n\t\t\t  display: block;\n\t\t\t\tmargin-left: auto;\n\t\t\t\tmargin-right: auto;\n\t\t\t\twidth: 50%;\n\t\t}\n\n figure{\n\t display:inline-block;\n\t width:10vw;\n\t height:20vw;\n\t margin:0;\n\t padding:0;\n\t vertical-align: middle\n }\n figcaption{\n\t\tdisplay:inline-block;\n\t\theight:2vw;\n }\n\t\t*{\n\t\t\tmargin:0;\n\t\t\tpadding:0;\n\t\t\tborder-width:0;\n\t\t}\n\t\tnav{\n\t\t\tposition:fixed;\n\t\t\tleft:0;\n\t\t\ttop:0;\n\t\t\theight:5vh;\n\t\t\twidth:100vw;\n\t\t\tbackground-color:black;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-color:#97f26a;\n\t\t\tborder-width:.4vh;\n\t\t}\n\t\tnav ul{\n\t\t\tlist-style-type:none;\n\t\t\tline-height:5vh;\n\t\t\tmargin:0;\n\t\t\twidth:100vw;\n\t\t\tfont-size:1em;\n\t\t}\n\t\tnav ul li{\n\t\t\tdisplay:inline-block;\n\t\t\ttext-align:center;\n\t\t\tfloat:left;\n\t\t\tmargin:0;\n\t\t}\n\t\tnav ul li a{\n\t\t\tdisplay:inline-block;\n\t\t\tcolor:white;\n\t\t\ttext-decoration:none;\n\t\t\twidth:5vh;\n\t\t\theight:5vh;\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\tfont-size:1.5vw;\n\t\t}\n\t\tnav ul li a:hover{\n\t\t\tbackground-color:grey;\n\t\t/*\tborder-top-style:outset;*/\n\t\t\tborder-top-width:1vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-bottom-width:.4vh;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t}\n\t\tnav ul li p{\n\t\t\tdisplay:inline-block;\n\t\t\tcolor:white;\n\t\t\ttext-decoration:none;\n\t\t\twidth:20vw;\n\t\t\theight:5vh;\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\tfont-size:2em;\n\t\t}\n\t\tnav ul li:last-child\n\t\t{\n\t\t\tfloat:right;\n\t\t\tmargin-right:7vw;\n\t\t}\n\t\th1{\n\t\t\tmargin-top:3vh;\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\ttext-align:center;\n\t\t\tfont-size:5em;\n\t\t\twidth:50vw;\n\t\t\tmargin:3vh auto;\n\t\t}\n\t\th2{\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\twidth:50vw;\n\t\t\tfont-size:2em;\n\t\t\tmargin:3vh auto;\n\t\t}\n\t\th3{\n\t\t\tfont-family:\'Patua One\', cursive;\n\t\t\twidth:50vw;\n\t\t\tfont-size:1.2em;\n\t\t\tmargin:3vh auto;\n\t\t}\n\t\tiframe + a{\n\t\t\tfont-family:\'Barlow\', sans-serif;;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\ttext-indent:4vw;\n\t\t\tdisplay:block;\n\t\t}\n\t\th2 + a{\n\t\t\tfont-family:\'Barlow\', sans-serif;;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\ttext-indent:4vw;\n\t\t\tdisplay:block;\n\t\t}\n\t\tbody > a{\n\t\t\tfont-family:\'Barlow\', sans-serif;;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\ttext-indent:4vw;\n\t\t\tdisplay:block;\n\t\t}\n\t\tp{\n\t\t\tfont-family:\'Barlow\', sans-serif;;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\ttext-indent:4vw;\n\t\t}\n\t\tol{\n\t\t\tfont-family:\'Barlow\', sans-serif;\n\t\t\tfont-size:1.2em;\n\t\t\twidth:50vw;\n\t\t\tmargin:auto;\n\t\t\tlist-style-type:circle;\n\t\t}\n\t\tli{\n\t\t\tmargin-top:.6vh;\n\t\t}\n\t\t\n\t\timg{\n\t\t\tdisplay:block;\n\t\t\tmargin:3vh auto;\n\t\t\twidth:50vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-top-style:solid;\n\t\t\tborder-top-color:#0066ff;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t\tborder-width:1vh;\n\t\t}\n\t\t.iframeS{\n\t\t\twidth:320;\n\t\t\theight:180;\n\t\t}\n\t\tiframe{\n\t\t\tdisplay:block;\n\t\t\tmargin:3vh auto;\n\t\t\twidth:50vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-top-style:solid;\n\t\t\tborder-top-color:#0066ff;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t\tborder-width:1vh;\n\t\t}\n\t\tvideo{\n\t\t\t\tdisplay:block;\n\t\t\tmargin:3vh auto;\n\t\t\twidth:50vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-top-style:solid;\n\t\t\tborder-top-color:#0066ff;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t\tborder-width:1vh;\n\t\t}\n\t\ttable{\n\t\t\tborder-collapse:collapse;\n\t\t\tmargin:3vh auto;\n\t\t\tuser-select:none;\n\t\t}\n\t\ttd,th{\n\t\t\tborder: .3vh solid black;\n\t\t\ttext-align: left;\n\t\t\tpadding: 1vh;\n\t\t\tfont-family:Verdana;\n\t\t}\n\t\tth{\n\t\t\tcolor:white;\n\t\t\tbackground-color:#97f26a;\t\t\n\t\t}\n\n\t\tbody:not(nav){\n\t\t\tposition:relative;\n\t\t\ttop:7vh;\n\t\t}\n\t\t#active{\n\t\t\tbackground-color:#0066ff;\n\t\t}\n\t\t#active:hover{\n\t\t\tbackground-color:#6699ff;\n\t\t}\n\t\t.ordered{\n\t\t\tlist-style-type:lower-alpha;\n\t\t}\n\t\t.imgS{\n\t\t\twidth:10vw;\n\t\t\theight:10vw;\n\t\t}\n\t\tli .imgS{\n\t\tdisplay:inline;\n\t\t\tmargin-top:3vh ;\n\t\t\tmargin-bottom:3vh;\n\t\t\tmargin-left:auto;\n\t\t\tmargin-right:auto;\n\t\t\twidth:10vw;\n\t\t\tborder-bottom-style:solid;\n\t\t\tborder-top-style:solid;\n\t\t\tborder-top-color:#0066ff;\n\t\t\tborder-bottom-color:#97f26a;\n\t\t\tborder-width:1vh;\n\t\t}\n\n</style>\n<script></script>\n\n<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">\n<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css?v=1\">\n<head>\n\t<title>LED Matrix Control</title>\n</head>\n<body>\n\t<div page-name=\"Color_Palette\">\n\t\t<h1>Color Palette</h1>\n\t\t<select id = \"color_possible\">\n\t\t\t<option value =\"def\">Default</option>\n\t\t\t<option value=\"umd\">UMD</option>\n\t\t\t<option value=\"us\">U.S.</option>\n\t\t\t<option value=\"solid\">Solid</option>\n\t\t</select>\n\t\t<br/>\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<input class = \"hidden\" type=\"text\" name = \"submit_type\" value =\"color_set\" display=\"none\"/>\n\t\t\t<div class = \"colorDiv\"><input  class = \"colorBox\" type = \"color\" id = \"c0\" name = \"c0\"/><input  class = \"colorBox\" type = \"color\" id = \"c1\" name = \"c1\"/><input  class = \"colorBox\" type = \"color\" id = \"c2\" name = \"c2\"/><input  class = \"colorBox\" type = \"color\" id = \"c3\" name = \"c3\"/><input  class = \"colorBox\" type = \"color\" id = \"c4\" name = \"c4\"/><input  class = \"colorBox\" type = \"color\" id = \"c5\" name = \"c5\"/><input  class = \"colorBox\" type = \"color\" id = \"c6\" name = \"c6\"/><input  class = \"colorBox\" type = \"color\" id = \"c7\" name = \"c7\"/><input  class = \"colorBox\" type = \"color\" id = \"c8\" name = \"c8\"/><input  class = \"colorBox\" type = \"color\" id = \"c9\" name = \"c9\"/><input  class = \"colorBox\" type = \"color\" id = \"c10\" name = \"ca\"/><input  class = \"colorBox\" type = \"color\" id = \"cv\" name = \"cs\"/><input  class = \"colorBox\" type = \"color\" id = \"c12\" name = \"cd\"/><input  class = \"colorBox\" type = \"color\" id = \"sdc\" name = \"cf\"/><input  class = \"colorBox\" type = \"color\" id = \"cg\" name = \"cg\"/><input  class = \"colorBox\" type = \"color\" id = \"cj\" name = \"cp\"/></div>\n\n\t\t\t\n\t\t\t<button onclick =submitForm ontouchstart=submitForm><i class = \"fa fa-paper-plane\"></i></button>\n\t\t</form>\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t</div>\n\n\t<div page-name=\"Animation_Settings\">\n\t\t\n\t\t<h1>Animation Settings</h1>\n\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<input class = \"hidden\" type=\"text\" name = \"submit_type\" value =\"settings_set\" display=\"none\"/>\n\t\t\t<div class = \"settingBackground\"><label for =\"sensitivity\">Sensitivity</label><input type = \"range\" min = \"0\" max = \"9\" step = \"1\"  value = \"5\" id = \"sensitivity\" name = \"sensitivity\" class = \"rangeSlider\"></div>\n\t\t\t<div class = \"settingBackground\"><label for =\"brightness\">Brightness</label><input type = \"range\" min = \"0\" max = \"9\" step = \"1\"  value = \"5\" d = \"brightness\" name = \"brightness\" class = \"rangeSlider\"></div>\n\t\t\t<div class = \"settingBackground\"><label for =\"relax\">Drop Time</label><input type = \"range\" min = \"0\" max = \"9\" step = \"1\"  value = \"5\" id= \"relax\" name = \"relax\" class = \"rangeSlider\"></div>\n\t\t\t<select name = \"mode\" id = \"color_possible\">\n\t\t\t\t<option value =\"0\">Normal Vis</option>\n\t\t\t\t<option value=\"1\">Image</option>\n\t\t\t\t<option value=\"2\">Image Vis</option>\n\n\t\t\t</select>\n\t\t\t<button onclick =submitForm ontouchstart=submitForm><i class = \"fa fa-paper-plane\"></i></button>\n\t\t</form>\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t</div>\n\n\n\t<div page-name=\"Display_Image\">\n\t\t<h1>Display Image</h1>\n\t\t<div class = \"settingBackground\"><label for =\"box_size\">Box Size</label><input type = \"range\" min = \"1\" max = \"64\" step = \"1\"  value = \"5\" id = \"box_size\" name = \"box_size\" class = \"rangeSlider\"></div>\n\t\t<input type=\"file\" id=\"image_upload\">\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<input class = \"hidden\" type=\"text\" name = \"submit_type\" value =\"image_set\" display=\"none\"/>\n\t\t\t<div class = \"hidden\" id = \"image_data\"><input  class = \"colorBox\" type = \"color\" id = \"c0\" name = \"c0\"/></div>\n\t\t\t<button onclick =submitForm ontouchstart=submitForm><i class = \"fa fa-paper-plane\"></i></button>\n\t\t</form>\n\n\t\t<div style=\"position: relative;\">\n\t\t\t<canvas style=\"position:absolute;top:0;left:0;z-index: 3;\" id= \"layer1\" width=\"500\" height=\"500\"></canvas>\n\t\t\t<canvas style=\"position:absolute;top:0;left:0;z-index: 2;\"  id= \"layer0\" width=\"500\" height=\"500\"></canvas>\n\t\t\t\n\t\t</div>\n\t\t<canvas style = \"border:solid\" width = \"256\" height = \"256\" id = \"dbg_can\"></canvas>\n\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t\t\n\t</div>\n\n\t<div page-name=\"Scroll_Text\">\n\t\t<h1>Scroll Text</h1>\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<label for =\"fname\">First nameL</label><input type = \"text\" id = \"fname\" name = \"fname\">\n\t\t\t<label for =\"lname\">Last nameL</label><input type = \"text\" id = \"lname\" name = \"lname\">\n\t\t\t<input type = \"submit\" value = \"Submit\">\n\t\t</form>\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t</div>\n\n\t<div page-name=\"API_Based\">\n\t\t<h1>API Based</h1>\n\t\t<form action = \"/echo\" target = \"hiddenFrame\" method = \"post\">\n\t\t\t<label for =\"fname\">First nameL</label><input type = \"text\" id = \"fname\" name = \"fname\">\n\t\t\t<label for =\"lname\">Last nameL</label><input type = \"text\" id = \"lname\" name = \"lname\">\n\t\t\t<input type = \"submit\" value = \"Submit\">\n\t\t</form>\n\t\t<iframe name = \"hiddenFrame\" width = \"0\" height = \"0\" border = \"0\" style = \"display:none;\"></iframe>\n\t</div>\n\n\t<nav>\n\t\t<ul>\n\t\t\t<li>\n\t\t\t\t<a id=\"active\" page-name = \"Color_Palette\" class = \"nav-button\"><i class=\"fa fa-paint-brush\"></i></a>\n\t\t\t</li>\n\t\t\t<li>\n\t\t\t\t<a page-name = \"Animation_Settings\" class = \"nav-button\"><i class=\"fa fa-cog\"></i></a>\n\t\t\t</li>\n\t\t\t<li>\n\t\t\t\t<a page-name = \"Display_Image\" class = \"nav-button\"><i class=\"fa fa-file-image-o\"></i></a>\n\t\t\t</li>\n\t\t\t<li>\n\t\t\t\t<a page-name = \"Scroll_Text\" class = \"nav-button\"><i class=\"fa fa-file-text-o\"></i></a>\n\t\t\t</li>\n\t\t\t<li>\n\t\t\t\t<a page-name = \"API_Based\" class = \"nav-button\"><i class=\"fa fa-cloud\"></i></a>\n\t\t\t</li> \n\t\t\t\n\n\n\t\t\t<li>\n\t\t\t\t<p id = \"Page_Text\">Color Palette</a>\n\t\t\t</li>\n\t\t</ul>\n\t</nav>\n\t<script>\n\t\tfunction componentToHex(c) {\n  let hex = c.toString(16);\n  return hex.length == 1 ? \"0\" + hex : hex;\n}\nfunction rgbToHex(r, g, b) {\n  return \"#\" + componentToHex(r) + componentToHex(g) + componentToHex(b);\n}\n\n\t\tvar umd = [\"#c17d11\",  \n\"#c17d11\",  \n\"#555753\",  \n\"#555753\",  \n\"#a40000\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#eeeeec\",  \n\"#c17d11\",  \n\"#c17d11\",  \n\"#555753\",  \n\"#555753\",  \n\"#a40000\",  \n\"#a40000\", \n\"#eeeeec\", \n\"#eeeeec\",]\n\n\t\tvar default_color = [\"#5c3566\",  \n\"#a900ff\",  \n\"#5100ff\",  \n\"#204a87\",  \n\"#00b1ff\",  \n\"#c17d11\",  \n\"#c4a000\",  \n\"#eeeeec\",  \n\"#555753\",  \n\"#73d216\",  \n\"#4e9a06\",  \n\"#f57900\",  \n\"#ce5c00\",  \n\"#b55151\",  \n\"#ef2929\",  \n\"#a40000\"];\n\n\t\tvar us = [\n\t\t\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",  \n\"#eeeeec\",  \n\"#00b1ff\",  \n\"#a40000\",\n\t\t]\n\t\tfunction colorUpdate(e)\n\t\t{\n\t\t\tvar choice = e.srcElement.value;\n\t\t\tvar arr = [];\n\t\t\tif(choice === \"umd\")\n\t\t\t{\n\t\t\t\tarr = umd;\n\t\t\t}\n\t\t\tif(choice === \"def\")\n\t\t\t{\n\t\t\t\tarr = default_color;\n\t\t\t\t//console.log(\"DEF\");\n\t\t\t}\n\t\t\tif(choice=== \"us\")\n\t\t\t{\n\t\t\t\tarr = us;\n\t\t\t}\n\t\t\tif(choice ===\"solid\")\n\t\t\t{\n\t\t\t\tvar cb = document.getElementsByClassName(\"colorDiv\")[0].children;\n\t\t\tfor(var i = 0;i < 16;i++)\n\t\t\t{\n\t\t\t\tcb[i].value = cb[0].value;\n\t\t\t}\n\t\t\t\treturn;\n\t\t\t}\n\t\t\tvar cb = document.getElementsByClassName(\"colorDiv\")[0].children;\n\t\t\tfor(var i = 0;i < 16;i++)\n\t\t\t{\n\t\t\t\tcb[i].value = arr[i];\n\t\t\t}\n\t\t}\n\n\t\tfunction getElementByAttribute(attribute, value)\n\t\t{\n\t\t\treturn document.querySelectorAll(\"[\" + attribute + \"=\" + value + \"]\")[0];\n\t\t}\n\t\tfunction submitForm(e)\n\t\t{\n\t\t\te.srcElement.parentNode.submit();\n\t\t}\n\t\tfunction navClick(e)\n\t\t{\n\t\t\tvar old = document.getElementById(\"active\");\n\t\t\told.id = null;\n\t\t\tconsole.log(e.srcElement.parentNode);\n\t\t\tconsole.log(e.srcElement);\n\t\t\tvar properSource;\n\t\t\tif(e.srcElement.tagName === \"A\")\n\t\t\t{\n\t\t\t\tproperSource = e.srcElement;\n\n\t\t\t}\n\t\t\telse\n\t\t\t{\n\t\t\t\tproperSource = e.srcElement.parentNode;\n\n\t\t\t}\n\n\t\t\tproperSource.id = \"active\";\n\n\t\t\tgetElementByAttribute(\"page-name\",old.getAttribute(\"page-name\")).style.display = \"none\";\n\t\t\tgetElementByAttribute(\"page-name\",properSource.getAttribute(\"page-name\")).style.display = \"block\";\n\n\t\t\t\n\t\t}\n\n\t\tfunction imageUpload(e)\n\t\t{\n    if(e.target.files) {\n      let imageFile = e.target.files[0]; //here we get the image file\n      var reader = new FileReader();\n      reader.readAsDataURL(imageFile);\n      reader.onloadend = function (e) {\n        var myImage = new Image(); // Creates image object\n        myImage.src = e.target.result; // Assigns converted image to image object\n        myImage.onload = function(ev) {\n          var myCanvas = document.getElementById(\"layer0\"); // Creates a canvas object\n          var myContext = myCanvas.getContext(\"2d\"); // Creates a contect object\n          myCanvas.width = myImage.width; // Assigns image\'s width to canvas\n          myCanvas.height = myImage.height; // Assigns image\'s height to canvas\n          myContext.drawImage(myImage,0,0); // Draws the image on canvas\n          let imgData = myCanvas.toDataURL(\"image/jpeg\",0.75); // Assigns image base64 string in jpeg format to a variable\n\t\t  document.getElementById(\"layer1\").width = myCanvas.width;\n\t\t  document.getElementById(\"layer1\").height = myCanvas.height;\n\n        }\n      }\n    }\n  }\n  var can = document.getElementById(\"layer1\");\n\t\t\tvar ctx = can.getContext(\"2d\");\n\t\t\t\n  \t\tfunction imageMove(e)\n\t\t{\n\t\t\tconsole.log(e.clientX, e.clientY);\n\t\t\tvar rect = document.getElementById(\"layer1\").getBoundingClientRect()\n\t\t\tvar x = e.clientX - rect.left;\n\t\t\tvar y = e.clientY - rect.top;\n\t\t\t//ctx.beginPath();\n\t\t\tvar boxSize = document.getElementById(\"box_size\").value;\n\t\t\tctx.clearRect(0,0,can.width,can.height);\n\t\t\tctx.strokeStyle =  \"#FF0000\";\n\t\t\tctx.strokeRect(x,y,16*boxSize,16*boxSize);\n\t\t\t//ctx.stroke();\n\t\t\t\n\t\t}\n\n\n\n\t\tfunction imageClick(e)\n\t\t{\n\t\t\tvar rect = document.getElementById(\"layer1\").getBoundingClientRect()\n\t\t\tvar cx = e.clientX - rect.left;\n\t\t\tvar cy = e.clientY - rect.top;\n\t\t\tvar boxSize = document.getElementById(\"box_size\").value;\n\t\t\tctxI = document.getElementById(\"layer0\").getContext(\"2d\");\n\t\t\t//console.log(boxSize, cx + x* boxSize, cy + y * boxSize,boxSize * 16,boxSize * 16);\n\t\t\tvar dat = ctxI.getImageData(cx, cy,boxSize * 16,boxSize * 16).data;\n\t\t\tdocument.getElementById(\"image_data\").innerHTML = \"\";\n\t\t\tfor(var x = 0;x < 16;x++)\n\t\t\t{\n\t\t\t\tfor(var y = 0; y < 16;y++)\n\t\t\t\t{\n\t\t\t\t\t\n\t\t\t\t\tvar ravg = 0.0;\n\t\t\t\t\tvar gavg = 0.0;\n\t\t\t\t\tvar bavg = 0.0;\n\t\t\t\t\tfor(var i = 0;i < boxSize;i++)\n\t\t\t\t\t{\n\t\t\t\t\t\tfor(var j = 0;j < boxSize;j++)\n\t\t\t\t\t\t{\n\t\t\t\t\t\t\travg += dat[((x  + y * boxSize*16)  * boxSize + i + j * boxSize * 16)* 4];\n\t\t\t\t\t\t\tgavg += dat[((x  + y * boxSize*16)  * boxSize + i + j * boxSize * 16)* 4 + 1];\n\t\t\t\t\t\t\tbavg += dat[((x  + y * boxSize*16)  * boxSize + i + j * boxSize * 16)* 4 + 2];\n\t\t\t\t\t\t\t\n\t\t\t\t\t\t}\n\t\t\t\t\t}\n\t\t\t\t\travg/= 1/255*(boxSize * 16) * (boxSize * 16);\n\t\t\t\t\tgavg/= 1/255*(boxSize * 16) * (boxSize * 16);\n\t\t\t\t\tbavg/= 1/255*(boxSize * 16) * (boxSize * 16);\n\t\t\t\t\travg = Math.round(ravg);\n\t\t\t\t\tgavg= Math.round(gavg);\n\t\t\t\t\tbavg = Math.round(bavg);\n\t\t\t\t\tdcan = document.getElementById(\"dbg_can\");\n\t\t\t\t\tdctx = dcan.getContext(\"2d\");\n\t\t\t\t\tdctx.fillStyle = rgbToHex(ravg,gavg,bavg);\n\t\t\t\t\tdctx.fillRect(16*x,16*y,16,16);\n\t\t\t\t\tconsole.log(ravg,gavg,bavg);\n\n\t\t\t\t\tvar node = document.createElement(\"INPUT\");\n\t\t\t\t\t\n\t\t\t\t\tnode.name = componentToHex(x + 16 * y);\n\t\t\t\t\tnode.value = rgbToHex(ravg,gavg,bavg);\n\t\t\t\t\tdocument.getElementById(\"image_data\").appendChild(node);\n\t\t\t\t}\n\t\t\t}\n\n\n\t\t\n\t\t}\n\t\tdocument.getElementById(\"layer1\").addEventListener(\"mousemove\",imageMove);\n\t\tdocument.getElementById(\"layer1\").addEventListener(\"mousedown\",imageClick);\n\t\tnavA = document.getElementsByClassName(\"nav-button\");\n\t\tfor(var i =  navA.length - 1;i >= 0;i--)\n\t\t{\n\t\t\tnavA[i].addEventListener(\"click\",navClick);\n\t\t\tnavA[i].click();\n\t\t}\n\t\tdocument.getElementById(\"color_possible\").addEventListener(\"change\",colorUpdate);\n\t\tdocument.getElementById(\"image_upload\").addEventListener(\"change\",imageUpload)\n\t\tvar e = {};\n\t\te.srcElement = document.getElementById(\"color_possible\");\n\t\tcolorUpdate(e)\n\t</script>\n</body>\n",
};
int drop = 1;
int brightness = 5;
int sensitivity =5;
int color_update = 0;
int image[3 * 16 * 16];
int image_update = 0;
int image_vis_update = 0;
int colors[48] = {235,235,57,237,223,41,238,211,22,239,200,0,240,187,0,240,175,0,240,163,0,239,150,0,238,137,0,237,124,0,235,111,0,232,97,0,229,82,0,226,65,0,222,45,3,217,13,13};
int mode = 0;
/* An HTTP POST handler */
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

static esp_err_t echo_post_handler(httpd_req_t *req)
{
    
    
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, BUF_SIZE * sizeof(char)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
        printf("\n\n%c ,%d\n POST\n", setting1,xPortGetCoreID());
        setting1 = buf[1];
    }
    printf("%s",buf);
//submit_type=image_set&%23000000=%233e3e3e&%23100000=%233e3e3e&%23200000=%233e3e3e&%23300000=%233e3e3e&%23400000=%233e3e3e&%23500000=%233e3e3e&%23600000=%233e3e3e&%23700000=%233e3e3e&%23800000=%233e3e3e&%23900000=%233e3e3e&%23a00000=%233e3e3e&%23b00000=%233e3e3e&%23c00000=%233e3e3e&%23d00000=%233e3e3e&%23e00000=%233e3e3e&%23f00000=%233e3e3e&%23010000=%233e3e3e&%23110000=%233e3e3e&%23210000=%233e3e3e&%23310000=%233e3e3e&%23410000=%233e3e3e&%23510000=%233e3e3e&%23610000=%233e3e3e&%23710000=%233e3e3e&%23810000=%233e3e3e&%23910000=%233e3e3e&%23a10000=%233e3e3e&%23b10000=%233e3e3e&%23c10000=%233e3e3e&%23d10000=%233e3e3e&%23e10000=%233e3e3e&%23f10000=%233e3e3e&%23020000=%233e3e3e&%23120000=%233e3e3e&%23220000=%233e3e3e&%23320000=%233e3e3e&%23420000=%233e3e3e&%23520000=%233e3e3e&%23620000=%233e3e3e&%23720000=%233e3e3e&%23820000=%233e3e
//submit_type=image_set&00=0d120f&10=423319&20=d29f3f&30=7f2431&40=832336&50=832336&60=832336&70=832336&80=832336&90=832336&a0=832336&b0=832336&c0=832336&d0=832236&e0=7d2233&f0=7a2534&01=0f110c&11=110e10&21=7f5b2e&31=802333&41=832336&51=832336&61=832336&71=832336&81=832336&91=842236&a1=82
//submit_type=image_set&00=%23c8a2a3&10=%23c8a3a5&20=%237c666e&30=%23473036&40=%234f2d2b&50=%238e4431&60=%23c46344&70=%233a3439&80=%2328262d&90=%232a282f&a0=%232b282e&b0=%2332333c&c0=%232e2e3
    if(buf[12] == 'i')
    {
        printf("IMAGE UPDATE!!!\n\n");
        for(int i = 0;i < 256;i++)
        {
                        int r = hex_to_decimal(buf[28 + 0 + 13 * i],
            buf[28 + 1 + 13 * i]);
            int g = hex_to_decimal(buf[28 + 2 + 13 * i],
            buf[28 + 3 + 13 * i]);

            int b = hex_to_decimal(buf[28 + 4 + 13 * i],
            buf[28 + 5 + 13 * i]);
            
            image[i*3] = r;

            image[i*3 + 1] = g;
            image[i*3 + 2] = b;
            printf("IMAGE %d %d %d\n",r,g,b);

        }
        image_update = 1;
    }
    
    if(buf[12] == 'c')
    {
        for(int i = 0; i <16;i++)
        {
            int r = hex_to_decimal(buf[28 + 0 + 13 * i],
            buf[28 + 1 + 13 * i]);
            int g = hex_to_decimal(buf[28 + 2 + 13 * i],
            buf[28 + 3 + 13 * i]);

            int b = hex_to_decimal(buf[28 + 4 + 13 * i],
            buf[28 + 5 + 13 * i]);

            printf("%d, %d_%d_%d\n",i,r,g,b);
            colors[i*3] = r;

            colors[i*3 + 1] = g;
            colors[i*3 + 2] = b;
        }
        color_update = 1;
    }


    if(buf[12] == 's')
    {
        
        brightness = (int)buf[50] - 48;
        sensitivity = (int)buf[37] - 48;
        drop = (int)buf[58] - 47;//min 1
        mode = (int)buf[65] - 48;
        printf("%d %d %d BRT SENS DRP\n",brightness,sensitivity, drop);
        if(mode == 0)
        color_update  =1;
        if(mode == 1)
        image_update = 1;
        if(mode == 2)
        image_vis_update = 1;
    }
    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}


static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

/* An HTTP PUT handler. This demonstrates realtime
 * registration and deregistration of URI handlers
 */
static esp_err_t ctrl_put_handler(httpd_req_t *req)
{
    char buf;
    int ret;

    if ((ret = httpd_req_recv(req, &buf, 1)) <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    if (buf == '0') {
        /* URI handlers can be unregistered using the uri string */
        ESP_LOGI(TAG, "Unregistering /hello and /echo URIs");
        httpd_unregister_uri(req->handle, "/hello");
        httpd_unregister_uri(req->handle, "/echo");
        /* Register the custom error handler */
        httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, http_404_error_handler);
    }
    else {
        ESP_LOGI(TAG, "Registering /hello and /echo URIs");
        httpd_register_uri_handler(req->handle, &hello);
        httpd_register_uri_handler(req->handle, &echo);
        /* Unregister custom error handler */
        httpd_register_err_handler(req->handle, HTTPD_404_NOT_FOUND, NULL);
    }

    /* Respond with empty body */
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static const httpd_uri_t ctrl = {
    .uri       = "/ctrl",
    .method    = HTTP_PUT,
    .handler   = ctrl_put_handler,
    .user_ctx  = NULL
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &echo);
        httpd_register_uri_handler(server, &ctrl);
        #if CONFIG_EXAMPLE_BASIC_AUTH
        httpd_register_basic_auth(server);
        #endif
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}







void ledChaseTask(void* pvParameters)
{

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(CONFIG_EXAMPLE_RMT_TX_GPIO, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(CONFIG_EXAMPLE_STRIP_LED_NUMBER, (led_strip_dev_t)config.channel);
    led_strip_t *strip =  led_strip_new_rmt_ws2812(&strip_config);
        if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));
    // Show simple rainbow chasing pattern
    ESP_LOGI(TAG, "LED Rainbow Chase Start");
     uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;
    uint16_t start_rgb = 0;
    while (true) {
        for (int i = 0; i < CONFIG_EXAMPLE_STRIP_LED_NUMBER; i++) {

            ESP_ERROR_CHECK(strip->set_pixel(strip, i, 255, 100, 25));
            // Flush RGB values to LEDs
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            strip->clear(strip, 50);
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            printf("%d LED NUM in TASK\n", i);
        }
        start_rgb += 60;
    }
}
int position_to_index(int xi, int y)
{
    int x = (15 - xi);
    return x * 16 + (15 - y) * (x % 2)  + y * (-1 * (x % 2 - 1));
}
   uint16_t i2s_b[1024];
   float sound[1024];
   float spectrum[1024];
   float spectrum_avg[512];
   int buckets[17] = {1, 2, 3, 4, 5, 6, 7, 9, 13, 23, 46, 90, 148, 208, 268, 350,512};
   //int buckets[17] = {7, 9, 14, 21, 33, 53, 78, 106, 135, 166, 196, 226, 257, 292, 335, 397,512};
   //int buckets[17] = {1, 2, 3, 4, 5, 6, 7, 13, 25, 51, 93, 139, 187, 234, 286, 358, 512};
   //int  buckets[17] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 13, 18, 27, 47, 84, 166, 334, 512};
   //int  buckets[17] = {0, 1, 2, 3, 4, 6, 8, 10, 13, 16, 28, 40, 67, 114, 196, 360, 512};
   
   float stack_avg[16 * LOOKBACK_LED];
   int decay_stack[16];
   int64_t last_decay =0;


 
void drawTask(void* pvParameters)
{
    int first = 1;
     rmt_config_t config = RMT_DEFAULT_CONFIG_TX(CONFIG_EXAMPLE_RMT_TX_GPIO, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(256, (led_strip_dev_t)config.channel);
    led_strip_t *strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
       for(int i =0; i < 16;i++)
            {


                    float brt = ((float) brightness )/ 1.0 / 20.0;
                    int r = (int)(colors[(i)*3] * brt);
                    int g = (int)(colors[(i)*3 + 1] * brt);
                    int b = (int)(colors[(i)*3+ 2] * brt);
                    if(r > 255)
                    {
                        r=225;
                    }
                    if(r < 0)
                    {

                        r = 0;
                    }
                    if(g > 255)
                    {
                        g=225;
                    }
                    if(g < 0)
                    {

                        g = 0;
                    }
                    if(b > 255)
                    {
                        b =225;
                    }
                    if(b < 0)
                    {

                        b = 0;
                    }
                    ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(i,0),r,g,b));

               
            }
    while(1)    
    {
        if(mode == 0)
        {
            if(color_update)
            {
                color_update = 0;
                for(int i =0; i < 16;i++)
                {


                                            float brt = ((float) brightness )/ 1.0 / 20.0;
                        int r = (int)(colors[(i)*3] * brt);
                        int g = (int)(colors[(i)*3 + 1] * brt);
                        int b = (int)(colors[(i)*3+ 2] * brt);
                        if(r > 255)
                        {
                            r=225;
                        }
                        if(r < 0)
                        {

                            r = 0;
                        }
                        if(g > 255)
                        {
                            g=225;
                        }
                        if(g < 0)
                        {

                            g = 0;
                        }
                        if(b > 255)
                        {
                            b =225;
                        }
                        if(b < 0)
                        {

                            b = 0;
                        }
                        ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(i,0),r,g,b));

                
                }
            }

            //printf("DRAW TASK %lld\n",time_us);
                //ESP_ERROR_CHECK(strip->clear(strip, 10));   
        // Clear LED strip (turn off all 
               for(int i = 1; i < 17;i++)
                {
                    int low_lim = first;
                    
                    for(int k = low_lim;k < 16;k++)
                    {
                        if(k < decay_stack[i-1])
                        {
                        

                                            float brt = ((float) brightness )/ 1.0 / 20.0;
                        int r = (int)(colors[(i-1)*3] * brt);
                        int g = (int)(colors[(i-1)*3 + 1] * brt);
                        int b = (int)(colors[(i-1)*3+ 2] * brt);
                        if(r > 255)
                        {
                            r=225;
                        }
                        if(r < 0)
                        {

                            r = 0;
                        }
                        if(g > 255)
                        {
                            g=225;
                        }
                        if(g < 0)
                        {

                            g = 0;
                        }
                        if(b > 255)
                        {
                            b =225;
                        }
                        if(b < 0)
                        {

                            b = 0;
                        }
                        ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(i-1,k),r,g,b));
                        }
                        else
                        {
                            ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(i-1,k),0,0,0));
                        }
                    }
                    first = 1;
                }
               
        ESP_ERROR_CHECK(strip->refresh(strip, 10));
        }
        else if(mode == 1  )
        {
            if(image_update)
            {
                for(int x = 0;x < 16;x++)
                {
                    for(int y = 0;y < 16;y++)
                    {
                        ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(15-y,15- x),image[(x + (15-y) * 16)*3 ]/10 ,image[(x + (15-y) * 16)*3 + 1]/10 ,image[(x + (15-y) * 16)*3 + 2]/10 ));
                    }
                }
                ESP_ERROR_CHECK(strip->refresh(strip, 10));
                image_update = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        else if (mode == 2)
        {
            if(image_vis_update)
            {
                image_vis_update = 0;
                for(int i =0; i < 16;i++)
                {


                                            float brt = ((float) brightness )/ 1.0 / 20.0;
                        int r = (int)(colors[(i)*3] * brt);
                        int g = (int)(colors[(i)*3 + 1] * brt);
                        int b = (int)(colors[(i)*3+ 2] * brt);
                        if(r > 255)
                        {
                            r=225;
                        }
                        if(r < 0)
                        {

                            r = 0;
                        }
                        if(g > 255)
                        {
                            g=225;
                        }
                        if(g < 0)
                        {

                            g = 0;
                        }
                        if(b > 255)
                        {
                            b =225;
                        }
                        if(b < 0)
                        {

                            b = 0;
                        }
                        ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(i,0),image[((15 - 0) + (i) * 16)*3 ]/10 ,image[((15-0) + (i) * 16)*3 + 1]/10 ,image[((15-0) + (i) * 16)*3 + 2]/10 ));

                
                }
            }

            //printf("DRAW TASK %lld\n",time_us);
                //ESP_ERROR_CHECK(strip->clear(strip, 10));   
        // Clear LED strip (turn off all 
               for(int i = 1; i < 17;i++)
                {
                    int low_lim = first;
                    
                    for(int k = low_lim;k < 16;k++)
                    {
                        if(k < decay_stack[i-1])
                        {
                        

                                            float brt = ((float) brightness )/ 1.0 / 20.0;
                        int r = (int)(colors[(i-1)*3] * brt);
                        int g = (int)(colors[(i-1)*3 + 1] * brt);
                        int b = (int)(colors[(i-1)*3+ 2] * brt);
                        if(r > 255)
                        {
                            r=225;
                        }
                        if(r < 0)
                        {

                            r = 0;
                        }
                        if(g > 255)
                        {
                            g=225;
                        }
                        if(g < 0)
                        {

                            g = 0;
                        }
                        if(b > 255)
                        {
                            b =225;
                        }
                        if(b < 0)
                        {

                            b = 0;
                        }
                        ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(i-1,k),image[((15 - k) + (i - 1) * 16)*3 ]/10 ,image[((15-k) + (i - 1) * 16)*3 + 1]/10 ,image[((15-k) + (i - 1) * 16)*3 + 2]/10 ));
                        }
                        else
                        {
                            ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(i-1,k),0,0,0));
                        }
                    }
                    first = 1;
                }
               
        ESP_ERROR_CHECK(strip->refresh(strip, 10));
        }
    }
}
void app_main(void)
{
    buf = malloc(BUF_SIZE * sizeof(char));

    /* int ch;

    
     // Prepare and set configuration of timers
     // that will be used by LED Controller
     
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
        {
            .channel    = LEDC_HS_CH0_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH0_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER,
            .flags.output_invert = 0
        },
        {
            .channel    = LEDC_HS_CH1_CHANNEL,
            .duty       = 0,
            .gpio_num   = LEDC_HS_CH1_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_HS_TIMER,
            .flags.output_invert = 0
        },

    };*/

    //boot_last_time = (long int)time(NULL);
    static httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());



    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    /* Register event handlers to stop the server when Wi-Fi or Ethernet is disconnected,
     * and re-start it upon connection.
     */
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &server));
#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET

    /* Start the server for the first time */
    printf("STARTING HTTP SERVER\n");
    server = start_webserver();
    printf("IS THE SERVER ON THIS CORE?\n");
    printf("\n\n%c ,%d\n MAIN\n", setting1,xPortGetCoreID());
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;
    xTaskCreate(drawTask, "DRAWTASK", 4096, &ucParameterToPass, tskIDLE_PRIORITY + 10, &xHandle);


    // install ws2812 driver

    //strip_config = 





    TaskHandle_t xHandleChase = NULL;
    //xTaskCreate(ledChaseTask, "CHASETASK", 4096, &ucParameterToPass, tskIDLE_PRIORITY, &xHandleChase);



  i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
  i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_5);

  vTaskDelay(1000); //required for stability of ADC 
  i2s_adc_enable(i2s_num);
  vTaskDelay(1000);
 
    size_t i2s_b_read;
    int cntr = 0;
    fft_config_t *fft_analysis = fft_init(1024, FFT_REAL, FFT_FORWARD, &sound[0], &spectrum[0]);
    


    // Clear LED strip (turn off all LEDs)

   
    // Show simple rainbow chasing pattern
    ESP_LOGI(TAG, "LED Rainbow Chase Start");
     uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;
    uint16_t start_rgb = 0;

    int decay_cnt = 0;
    while(1)
    {

        cntr++;
        cntr = cntr % 10000;

        i2s_read(i2s_num,&i2s_b,sizeof(i2s_b),&i2s_b_read, portMAX_DELAY);
                if(i2s_b_read > 1020)// && cntr == 987)
        {
            vTaskDelay(pdMS_TO_TICKS(2));
            
            long long unsigned int avg = 0;
            uint16_t min_dat = 10000;
            uint16_t max_dat = 0;
            for(int i = 0;i < 1024;i++)
            {
                avg+= i2s_b[i];
            }
            avg = avg/1024;
            for(int i = 0;i < 1024;i++)
            {
                sound[i] = ((float)(i2s_b[i] - (uint16_t)avg));
                //printf("%.6f, ", sound[i]);
            }
            fft_execute(fft_analysis);
            for(int i = 0; i < 512;i++)
            {
                spectrum[i] = sqrt(spectrum[2*i]*spectrum[2*i]+spectrum[2*i+1]*spectrum[2*i + 1]);
                //spectrum_avg[i] = spectrum_avg[i] + spectrum[i];
            }
            for(int i = 0;i < 0;i++)
            {
                spectrum[i] = 0.0;
            }

            //Flush RGB values to LEDs
            //ESP_ERROR_CHECK(strip->refresh(strip, 10));
            //vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            //strip->clear(strip, 50);
            //vTaskDelay(pdMS_TO_TICKS(100));
            //ESP_ERROR_CHECK(strip->set_pixel(strip,0,255,255,255));
            //ESP_ERROR_CHECK(strip->refresh(strip, 100));
            //printf("%.6f\n",spectrum[max_ind]);
            /*for(int i = 0;i < 100;i++)
            {
                float brt = (255.0 * (spectrum[i] / 4000000.0));
   

                //strip->set_pixel(strip,i,(uint32_t)brt,(uint32_t)brt,(uint32_t)brt);
                //printf("%.2f\n",brt);
            }*/
            /*
            if(cntr % 5000 == 0)
                printf("AVG\n");
            for(int i = 0;i < 512;i++)
            {
                spectrum_avg[i] = spectrum_avg[i] + spectrum[i];
                if(cntr % 5000 == 0)
                {
                    printf("%0.6f, ",spectrum_avg[i]);
                }
            }
            if(cntr % 5000 == 0)
                printf("END AVG\n");*/


            //ESP_ERROR_CHECK(strip->clear(strip, 1));
            for(int k = 0;k < 16;k++)
            {
                for(int i = 0;i < LOOKBACK_LED - 1;i++)
                {
                    stack_avg[LOOKBACK_LED * k + i + 1] = stack_avg[LOOKBACK_LED * k + i]; 
                }
            }
            for(int i = 1; i < 17;i++)
            {
                float sum =0.0;
                for(int j = buckets[i - 1];j < buckets[i];j++)
                {
                    sum+=spectrum[j + i];
                }
                sum = sum / 14.0;
                float brt = (255.0 * (sum / 4000000.0)) * 18.0;
                brt = 5.0*brt * brt / 190.0 * pow(1.5,sensitivity);
                //printf("%.6f STACK: %d\n",brt, i);
                
                if(brt < 0.1)
                {
                    brt = 0.01;
                }
                float avg_s = 0.0;
                float max = 0.0;
                int max_in = 0;
                stack_avg[i * LOOKBACK_LED] = brt;
                for(int h = 0;h < LOOKBACK_LED;h++)
                {
                    avg_s += stack_avg[h + i * LOOKBACK_LED];
                    if(stack_avg[h+i * LOOKBACK_LED] > max)
                    {
                        max_in = h;
                        max = stack_avg[h+i * LOOKBACK_LED];
                    }
                }
                max = max - (float)max_in/(float) LOOKBACK_LED * 8.0;
              
 

                /*
                  if(i <= 4)
                {
                    avg_s = avg_s /2.5;
                }
                if(i>= 7 && i <=10)
                {
                    avg_s = avg_s * 2.5;
                }*/
                avg_s = avg_s / (float)LOOKBACK_LED;
                //avg_s = max;
                avg_s = avg_s ;// (2.5 - (((float) i - 8.0)/6.0)*(((float) i -8.0)/6.0));
                if(i == 1)
                {
                    avg_s/=3.0;
                }
                if(5 <= i && i <= 8)
                {
                    avg_s*=3.0;
                }


                int stack = (int)avg_s;
                //printf("BRT %d ____ %d\n",i, stack);
                if(stack > 16)
                {
                    stack = 16;
                }
                if(stack < 1)
                {
                    stack = 1;
                }
                if(decay_stack[i - 1] < stack)
                {
                    decay_stack[i-1] = stack;
                }
        }
        //IF IS BLOCKED

            
            //strip->set_pixel(strip, position_to_index(15,6),250,0,0);
            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
            int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
            
            if(time_us - last_decay >  drop * 3000)
            {
                decay_cnt++;
                decay_cnt%=400;
                last_decay = time_us;
                for(int i = 0;i < 16;i++)
                {
                    if(decay_stack[i] > 6)
                    {
                        decay_stack[i] = decay_stack[i] - 1;
                    }
                    else if(decay_stack[i] > 0 && decay_cnt % 2 == 0)
                    {
                        decay_stack[i] = decay_stack[i] - 1;
                    }

                    
                }
            }
                //

    }
            /*for(int x =0; x  <16;x++)
            {
                for (int y = 0;y < 16;y++)
                {
                    ESP_ERROR_CHECK(strip->clear(strip, 100));
                    vTaskDelay(pdMS_TO_TICKS(100));
                    ESP_ERROR_CHECK(strip->set_pixel(strip,position_to_index(x,y),255,255,255));
                    ESP_ERROR_CHECK(strip->refresh(strip, 100));
                    printf("%d INDEX\n",position_to_index(x,y));
                    vTaskDelay(pdMS_TO_TICKS(200));
                }
            }*/
            //ESP_ERROR_CHECK(strip->set_pixel(strip, max_ind, 255, 100, 25));

            //for(int i = 0;i < 1024;i++)
            //{
              //  printf("%.6f, ",spectrum[i]);
            //}
            //printf("\n\n");
        }
        //vTaskDelay(1000);
    }
    
    


   

