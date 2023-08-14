#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/unique_id.h>
#include "pico/multicore.h"
#include <hardware/adc.h>
#include <tusb.h>
#include <RF24.h>

const int GPIO_MODE = 1;

#define SPI0_MISO 4
#define SPI0_MOSI 3
#define SPI0_SCK 2
#define SPI0_CSN 5
#define SPI0_CE 6

#define SPI1_MISO 12
#define SPI1_MOSI 11
#define SPI1_SCK 10
#define SPI1_CSN 13
#define SPI1_CE 14

char sid[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {0};
pico_unique_board_id_t id;

SPI spi_0;
SPI spi_1;
RF24 radio_tx(SPI0_CE, SPI0_CSN);
RF24 radio_rx(SPI1_CE, SPI1_CSN);


struct address_t {
    uint8_t rx[5];
    uint8_t tx[5];
};

struct message_t {
    uint32_t cnt;
    uint8_t payload[32];
};

struct address_t address[] = {
    {.rx = {0xE1, 0xF0, 0xF0, 0xF0, 0xF0}, .tx = {0xD2, 0xF0, 0xF0, 0xF0, 0xF0}},
    {.rx = {0xD2, 0xF0, 0xF0, 0xF0, 0xF0}, .tx = {0xE1, 0xF0, 0xF0, 0xF0, 0xF0}}
};

struct message_t tx_msg = {0};
struct message_t rx_msg = {0};

void radio_init(bool receiver, RF24* radio, SPI* spi_nrf, spi_inst_t* spi_pico, uint8_t spi_sck, uint8_t spi_miso, uint8_t spi_mosi, uint8_t spi_ce, uint8_t spi_csn) {
    printf("radio_init(%d, %p, %p, %p, %d, %d, %d, %d, %d)\n", receiver, radio, spi_nrf, spi_pico, spi_sck, spi_miso, spi_mosi, spi_ce, spi_csn);
    uint8_t device_id = gpio_get(GPIO_MODE);
    spi_nrf->begin(spi_pico, spi_sck, spi_miso, spi_mosi);
    while (!radio->begin(spi_nrf, spi_ce, spi_csn))
    {
        printf("radio.begin failed\n");
    }
    radio->failureDetected = 0;
    radio->setPALevel(RF24_PA_MIN);
    radio->setDataRate(RF24_1MBPS);
    radio->enableDynamicPayloads();
    radio->setAutoAck(true);
    radio->enableAckPayload();
    if (receiver) {
        radio->openReadingPipe(1 /* pipe */, address[device_id].rx);
        radio->startListening();
    } else {
        radio->openWritingPipe(address[device_id].tx);
        radio->stopListening();
    }
}

void radio_init_rx() {
    printf("radio_init_rx\n");
    radio_init(true, &radio_rx, &spi_1, spi1, SPI1_SCK, SPI1_MISO, SPI1_MOSI, SPI1_CE, SPI1_CSN);
}

void radio_init_tx() {
    printf("radio_init_tx\n");
    radio_init(false, &radio_tx, &spi_0, spi0, SPI0_SCK, SPI0_MISO, SPI0_MOSI, SPI0_CE, SPI0_CSN);
}

void tx_action() {
    memset(&tx_msg.payload, 0, sizeof(tx_msg.payload));
    tx_msg.cnt++;
    snprintf((char*)tx_msg.payload, sizeof(tx_msg.payload), "msg from %s", sid);
    if (radio_tx.write(&tx_msg, sizeof(tx_msg))) /* send OK */
    {
        printf("Tx (me:%s):  payload=%s cnt=%d\n", sid, tx_msg.payload, tx_msg.cnt);
    }
    else {
        if (radio_tx.failureDetected) {
            printf("/!\\ radio_tx.failureDetected\n");
            radio_init_tx();
        }
        
    } /* send KO */
}

void tx_thread()
{
    printf("TX Thread started\n");
    while (true)
    {
        tx_action();
    }
}

void rx_action() {
    uint8_t pipe;
    if (radio_rx.available(&pipe))
    {
        memset(&rx_msg, 0, sizeof(rx_msg));
        radio_rx.read(&rx_msg, radio_rx.getDynamicPayloadSize());
        printf("Rx (me:%s): pipe=%d, payload=%s cnt=%d\n", sid, pipe, rx_msg.payload, rx_msg.cnt);
    }
    else {
        if (radio_rx.failureDetected) {
            printf("/!\\ radio_rx.failureDetected\n");
            radio_init_rx();
        } else {} /* all good */
    }
}

void rx_thread()
{
    printf("RX Thread started\n");
    while (true)
    {
        rx_action();
    }
}

void setup()
{
    uint8_t device_id = gpio_get(GPIO_MODE);
    radio_init_tx();
    radio_init_rx();
    printf("device_id=%d\n", device_id);
    printf("tx_address=%llX, rx_address=%llX\n", address[device_id].tx, address[device_id].rx);
}

void wait_usb_read() {
    while (!tud_cdc_connected()) {
        sleep_ms(500);
    }
}

int main()
{
    stdio_init_all();
    pico_get_unique_board_id(&id);
    pico_get_unique_board_id_string(sid, sizeof(sid));
    wait_usb_read();
    printf("--------\n");
    printf("Board ID: %s\n", sid);

    setup();

    multicore_launch_core1(rx_thread);
    tx_thread();
    // shouldn't reach here
    while (1)
    {
        printf("Main loop\n");
        sleep_ms(1000);
    }
}