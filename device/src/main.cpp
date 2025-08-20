#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/clocks.h>
#include <hardware/timer.h>
#include <hardware/dma.h>
#include <hardware/adc.h>
#include <hardware/irq.h>
#include <stdio.h>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "tusb_config.h"
#include "tusb.h"

// As long as HIGH_SPEED mode is not enabled, CFG_TUD_CDC_TX_BUFSIZE is 64 bytes
const uint32_t SAMPLE_COUNT = CFG_TUD_CDC_TX_BUFSIZE / sizeof(uint16_t);
const uint32_t SAMPLE_RATE = 16000;// In Hz

uint32_t dma_chan;
uint16_t sample_buf[SAMPLE_COUNT]; // Exactly CFG_TUD_CDC_TX_BUFSIZE bytes in size

void on_dma_transfer_finished_int() {
    if (tud_cdc_connected()) {
        tud_cdc_write(sample_buf, sizeof(sample_buf));
    }

    // Clear the interrupt request.
    dma_hw->ints0 = (1u << dma_chan);
    dma_channel_set_write_addr(dma_chan, sample_buf, false);
    dma_channel_set_transfer_count(dma_chan, SAMPLE_COUNT, true);
}

int main() {
    stdio_init_all();
    
    tusb_rhport_init_t dev_init{
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    tud_init(BOARD_TUD_RHPORT);

    adc_init();
    adc_gpio_init(27);
    adc_select_input(1);
    adc_fifo_setup(true, true, 1, false, false); // Enable FIFO, DMA requests every single sample at full 16-bit resolution.

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
    
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true); // Increment after writing each sample
    channel_config_set_dreq(&cfg, DREQ_ADC); // Pace transfers based on availability of ADC samples

    dma_channel_configure(dma_chan, &cfg, sample_buf, &adc_hw->fifo, SAMPLE_COUNT, true);
    dma_channel_set_irq0_enabled(dma_chan, true); // Tell the DMA to raise IRQ line 0 when the channel finishes a block

    irq_set_exclusive_handler(DMA_IRQ_0, on_dma_transfer_finished_int); // Run dma_handler() when DMA IRQ 0 is asserted
    irq_set_enabled(DMA_IRQ_0, true);

    // Now with no overhead it can run realtime
    adc_run(true);
    adc_fifo_drain();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while(true) {
        tud_task();
    }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void)itf;
    (void)rts;
    
    if (dtr) {
        // Connected
        /// TODO: Start sampling, DMA and the interrupts now

        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    } else {
        // Disconnected
        /// TODO: Stop sampling, DMA and the interrupts

        gpio_put(PICO_DEFAULT_LED_PIN, 0);
    }
}