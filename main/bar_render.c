#include "bar_render.h"

static const char *TAG = "example";
void setup_bar_render_default()
{

    
    
    // set counter clock to 40MHz
    matrix_rmt_config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&matrix_rmt_config));
    ESP_ERROR_CHECK(rmt_driver_install(matrix_rmt_config.channel, 0, 0));

    // install ws2812 driver
    
    matrix = led_strip_new_rmt_ws2812(&matrix_led_config);
    if (!matrix) {
        ESP_LOGI(TAG, "install matrix WS2812 driver failed");
    }

/*
    //strip strip
    strip_rmt_config = RMT_DEFAULT_CONFIG_TX((unsigned int)5, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    strip_rmt_config.clk_div = 2;
    strip_rmt_config.channel = 1;
    ESP_ERROR_CHECK(rmt_config(&config1));

    ESP_ERROR_CHECK(rmt_driver_install(strip_rmt_config.channel, 0, 0));

    // install ws2812 driver
    strip_led_config = LED_STRIP_DEFAULT_CONFIG(150, (led_strip_dev_t)strip_rmt_config.channel);
    strip = led_strip_new_rmt_ws2812(&strip_led_config);


 

    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }*/

}

void * render_step_bar_render(void *settings_v, void *stack_heights_v)
{
    int *stack_heights = (int *) stack_heights_v;
    struct device_settings *settings = (struct device_settings *)settings_v;
    
    for(int i =0; i < 16;i++)
    {
            float brt = ((float) settings->matrix_brightness )/ 1.0 /8.0;

            
            //settings->matrix_brightness
            int r = (int)(settings->matrix_color_palette[i].a * brt);
            int g = (int)(settings->matrix_color_palette[i].b * brt);
            int b = (int)(settings->matrix_color_palette[i].c * brt);

            struct  color_param color_to_transform;
            color_to_transform.a = r / 255.0;
            color_to_transform.b = g/ 255.0;
            color_to_transform.c = b / 255.0;

            rgb_to_hsl(&color_to_transform);

            color_to_transform.a = color_to_transform.a + settings->matrix_hue_angle;

            hsl_to_rgb(&color_to_transform);

            r = (int)(color_to_transform.a * 255.0);
            g = (int) (color_to_transform.b * 255.0);
            b = (int)(color_to_transform.c * 255.0);

            r = mxi(mni(255,r),0);
            g = mxi(mni(255,g),0);
            b = mxi(mni(255,b),0);

            struct position_xy pos;
            pos.x = i;
            pos.y = 0;

            pos = (settings->transform_function)(pos);

            ESP_ERROR_CHECK(matrix->set_pixel(matrix,position_to_index(pos),r,g,b));

             for(int k = 1;k < 16;k++)
            {
                pos.x = i;
                pos.y = k;
                pos = (settings->transform_function)(pos);
                if(k < stack_heights[i])
                {
                

                ESP_ERROR_CHECK(matrix->set_pixel(matrix, position_to_index(pos),r,g,b));
                }
                else
                {
                    ESP_ERROR_CHECK(matrix->set_pixel(matrix, position_to_index(pos),0,0,0));
                }
            }
    }

        ESP_ERROR_CHECK(matrix->refresh(matrix, 10));
        return NULL;
}