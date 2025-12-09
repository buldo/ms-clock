#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

// SPI Defines
#define SPI_PORT spi0
#define PIN_SCK  2
// TX PIN
#define PIN_MOSI 3

#define PIN_LE   4
#define PIN_OE   5

#define OUTPUT_ENABLE 0
#define OUTPUT_DISABLE 1

// Data will be copied from src to dst
const char src[] = "Hello, world! (from DMA)";
char dst[count_of(src)];

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    return 0;
}

uint8_t frame_buffer[4] = {0};
uint8_t frame_buffer_count = 4;

uint8_t segment_map[16] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b10000000, // dot
};

void display_framebuffer(){
    // Start with LE low and OE disabled
    gpio_put(PIN_LE, 0);
    gpio_put(PIN_OE, OUTPUT_DISABLE);

    spi_write_blocking(SPI_PORT, frame_buffer, frame_buffer_count);

    // Latch the data - pulse LE high
    sleep_us(1); // Small delay to ensure data is stable
    gpio_put(PIN_LE, 1);
    sleep_us(1); // Hold LE high briefly
    gpio_put(PIN_LE, 0);

    // Enable outputs
    gpio_put(PIN_OE, OUTPUT_ENABLE);
}

void display_number(uint32_t number)
{
    // Assuming a 4-digit 7-segment display
    for(int i = 0; i < 4; i++)
    {
        uint8_t digit = number % 10;
        frame_buffer[3 - i] = segment_map[digit];
        number /= 10;
    }

    display_framebuffer();
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Initialize LE and OE pins
    gpio_init(PIN_LE);
    gpio_init(PIN_OE);
    gpio_set_dir(PIN_LE, GPIO_OUT);
    gpio_set_dir(PIN_OE, GPIO_OUT);





    // Get a free channel, panic() if there are none
    //int chan = dma_claim_unused_channel(true);

    // 8 bit transfers. Both read and write address increment after each
    // transfer (each pointing to a location in src or dst respectively).
    // No DREQ is selected, so the DMA transfers as fast as it can.

    // dma_channel_config c = dma_channel_get_default_config(chan);
    // channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    // channel_config_set_read_increment(&c, true);
    // channel_config_set_write_increment(&c, true);

    // dma_channel_configure(
    //     chan,          // Channel to be configured
    //     &c,            // The configuration we just created
    //     dst,           // The initial write address
    //     src,           // The initial read address
    //     count_of(src), // Number of transfers; in this case each is 1 byte.
    //     true           // Start immediately.
    // );

    // We could choose to go and do something else whilst the DMA is doing its
    // thing. In this case the processor has nothing else to do, so we just
    // wait for the DMA to finish.
    //dma_channel_wait_for_finish_blocking(chan);

    // The DMA has now copied our text from the transmit buffer (src) to the
    // receive buffer (dst), so we can print it out from there.
    //puts(dst);

    // Timer example code - This example fires off the callback after 2000ms
    //add_alarm_in_ms(2000, alarm_callback, NULL, false);
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    //printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    //printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
    // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

    uint64_t target_time = time_us_64() + 100;
    uint32_t counter = 0;
    while (true)
    {
        display_number(counter);
        counter++;
        if(counter > 9999)
        {
            counter = 0;
        }

        busy_wait_until(target_time);
        target_time += 100;
    }
}
