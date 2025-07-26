#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/clocks.h>
#include <hardware/timer.h>
#include <hardware/adc.h>
#include <stdio.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "tusb.h"

// As long as HIGH_SPEED mode is not enabled, keep 64 bytes
const uint32_t SAMPLE_COUNT = 64 / sizeof(uint16_t);
const uint32_t SAMPLE_RATE = 16000;// In Hz

int main() {
    stdio_init_all();
    tusb_init();
    tud_init(0);

    adc_init();
    adc_gpio_init(27);
    adc_select_input(1);

    adc_fifo_setup(true, false, 0, false, false);

    //multicore_launch_core1(core1_main);
    
    // Taken from rp2040 datasheet:
    /// ""
    // By default (DIV = 0), new conversions start immediately upon the previous conversion finishing, so a new sample is
    // produced every 96 cycles. At a clock frequency of 48MHz, this produces 500ksps.
    // Setting DIV.INT to some positive value n will trigger the ADC once per n + 1 cycles, though the ADC ignores this if a
    // conversion is currently in progress, so generally n will be >= 96. For example, setting DIV.INT to 47999 will run the ADC
    // at 1ksps, if running from a 48MHz clock.
    /// ""

    // The datasheet and SDK docs are unclear whether I need to divide by 96 or not.
    // Not dividing gives desired results so that's what I stick to.
    const uint32_t ADC_SAMPLE_CYCLES = 96; // <- according to the documentation

    uint32_t adc_clock_hz = clock_get_hz(clk_adc);
    uint32_t adc_clkdiv = adc_clock_hz / (/*ADC_SAMPLE_CYCLES **/ SAMPLE_RATE); 
    adc_set_clkdiv((float)adc_clkdiv); // should collect SAMPLE_RATE samples per second
    
    // Now with no overhead it can run realtime
    adc_run(true);
    adc_fifo_drain();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    static uint16_t sample_buf[SAMPLE_COUNT];
    uint32_t i{};

    while(true) {
        tud_task();

        //if(multicore_fifo_pop_timeout_us(1, &sample)) {
        if(!adc_fifo_is_empty()) {
            uint16_t sample = adc_fifo_get();

            if (tud_cdc_ready() && tud_cdc_connected()) {
                gpio_put(PICO_DEFAULT_LED_PIN, 1);
                sample_buf[i++] = sample;
            }
        }

        if (!tud_cdc_connected()) {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            continue;
        }
        
        if(i == SAMPLE_COUNT) {
            // Make sure to write only when host reads
            while(!tud_cdc_ready());
            
            //fwrite(sample_buf, 1, SAMPLE_COUNT * sizeof(uint16_t), stdout);
            //fflush(stdout);
            tud_cdc_write(sample_buf, SAMPLE_COUNT * sizeof(uint16_t));
            //tud_cdc_write_flush(); // no need to flush because tusb will do it anyway each 64 bytes
            i = 0;  
        }
    }
}

// Invoked when device is mounted
void tud_mount_cb(void) {

}

// Invoked when device is unmounted
void tud_umount_cb(void) {

}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;

}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void) itf;
    (void) rts;
    
    if (dtr) {
      // Terminal connected
    } else {
      // Terminal disconnected
    }
}