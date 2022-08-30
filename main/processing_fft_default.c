#include "processing_fft_default.h"

void *setup_processing_fft_default()
{
    fft_analysis = fft_init(1024, FFT_REAL, FFT_FORWARD, &sound[0], &spectrum[0]);
    decay_count = 0;
    counter = 0;
    return fft_default_decay_stack;
}

void* process_step_fft_default(void*  samples_read, void* i2s_b_v, void *settings_v)
{
        uint16_t *i2s_b = (uint16_t*) i2s_b_v;
        struct device_settings *settings = (struct device_settings *) settings_v;
        vTaskDelay(pdMS_TO_TICKS(1));
        counter++;
        counter = counter % 10000;

        if(1)// && cntr == 987)
        {
 
            
            long long unsigned int avg = 0;

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


            for(int i = 1; i < 17;i++)
            {
                float sum =0.0;
                for(int j = buckets[i - 1];j < buckets[i];j++)
                {
                    sum+=spectrum[j + i];
                }
                sum = sum / 14.0;
                float brt = (255.0 * (sum / 4000000.0)) * 18.0;
                brt = 5.0*brt * brt / 5890.0 * pow(2,settings->sensitivity);
                
                if(brt < 0.1)
                {
                    brt = 0.01;
                }
                float avg_s = brt;

 
                if(13 <= i && i <= 15)
                {
                    avg_s = avg_s * 5.5;
                }
                
                  if(i <= 1)
                {
                    avg_s = avg_s /2.5;
                }
                /*
                if(i>= 7 && i <=10)
                {
                    avg_s = avg_s * 2.5;
                }*/

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
                if(fft_default_decay_stack[i - 1] < stack)
                {
                    fft_default_decay_stack[i-1] = stack;
                }
        }
        //IF IS BLOCKED

            
            //strip->set_pixel(strip, position_to_index(15,6),250,0,0);
            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
            int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;


            if(settings->matrix_hue_speed > 0)
                {settings->matrix_hue_angle = float_mod((float)(time_us / 1000) / 1000.0 * 360.0 / 5.0 *  (pow(1.5,2 * settings->matrix_hue_speed) - 1.0) / 400.0, 359.95);
                
            }
            else
            {
                settings->matrix_hue_angle = 0.0;
            }

            if(settings->strip_hue_speed > 0)
                {settings->strip_hue_angle = float_mod((float)(time_us / 1000) / 1000.0 * 360.0 / 5.0 *  (pow(1.5,2 * settings->strip_hue_speed) - 1.0) / 400.0, 359.95);

            }
            else
            {
                settings->strip_hue_angle = 0.0;
            }

            if(time_us - last_decay >  settings->drop_time * 3000)
            {
                decay_count++;
                decay_count%=400;
                last_decay = time_us;
                for(int i = 0;i < 16;i++)
                {
                    if(fft_default_decay_stack[i] > 6)
                    {
                        fft_default_decay_stack[i] = fft_default_decay_stack[i] - 1;
                    }
                    else if(fft_default_decay_stack[i] > 0 && decay_count % 2 == 0)
                    {
                        fft_default_decay_stack[i] = fft_default_decay_stack[i] - 1;
                    }

                    
                }
            }
                //

    }
    return (void *)fft_default_decay_stack;
}