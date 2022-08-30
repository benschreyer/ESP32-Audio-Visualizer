#include "strip_sixteen_render.h"

static const char *TAG = "example";
void setup_sixteen_render()
{

    
    



    //strip strip
    
    // set counter clock to 40MHz
    strip_rmt_config.clk_div = 2;
    strip_rmt_config.channel = 1;
    ESP_ERROR_CHECK(rmt_config(&strip_rmt_config));

    ESP_ERROR_CHECK(rmt_driver_install(strip_rmt_config.channel, 0, 0));

    // install ws2812 driver

    strip = led_strip_new_rmt_ws2812(&strip_led_config);


 

    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }

}

void * render_step_sixteen_render(void *settings_v, void *stack_heights_v)
{
    int *stack_heights = (int *) stack_heights_v;
    struct device_settings *settings = (struct device_settings *)settings_v;
    
    int counter = 0;
    
        for(int i = 1;i < 17;i++)
        {
                        float brt_strip = ((float) settings->strip_brightness )/ 1.0 / 9.0;
                        int rs = (int)(settings->strip_color_palette[(i-1)].a * brt_strip);
                        int gs = (int)(settings->strip_color_palette[(i-1)].b * brt_strip);
                        int bs = (int)(settings->strip_color_palette[(i-1)].c * brt_strip);

                        struct  color_param color_to_transforms;
                        color_to_transforms.a = rs / 255.0;
                        color_to_transforms.b = gs/ 255.0;
                        color_to_transforms.c = bs / 255.0;

                        rgb_to_hsl(&color_to_transforms);

                        color_to_transforms.a = color_to_transforms.a + settings->strip_hue_angle;

                        hsl_to_rgb(&color_to_transforms);

                        rs = (int)(color_to_transforms.a * 255.0);
                        gs = (int) (color_to_transforms.b * 255.0);
                        bs = (int)(color_to_transforms.c * 255.0);

                        rs = mxi(mni(255,rs),0);
                        gs = mxi(mni(255,gs),0);
                        bs = mxi(mni(255,bs),0);
                   
                    
                    int bound = 0;
                    if(i <= 6)
                    {
                        bound = 10;
                    }
                    else
                    {
                        bound = 9;
                    }

                         


                    float bar_scale = 1.01f;//(float) (decay_stack[i - 1] + 8) / 16.5f;
                    for(int z =0; z < bound;z++)
                    {
                        strip->set_pixel(strip, counter,(int)(rs * bar_scale),(gs * bar_scale),(bs * bar_scale));//150 + (int)(0.5*(float)r),150 + (int)(0.5*(float)g),150 + (int)(0.5*(float)b));
                        counter++;
                    }

        }

        ESP_ERROR_CHECK(strip->refresh(strip, 10));
        return 1;
}