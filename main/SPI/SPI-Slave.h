#ifndef SPI_SLAVE_H
#define SPI_SLAVE_H

#include <queue>
#include "../SmartPointers/DMASmartPointer.h"
#include "driver/spi_slave.h"
#include "esp_log.h"

static const char* SPI_Tag = "SPI_slave";

#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_CLK  18
#define VSPI_CS   5

#define HSPI_MOSI 13
#define HSPI_MISO 12
#define HSPI_CLK 14
#define HSPI_CS   15

#define SLAVE_RX_QUEUE_SIZE 6
#define BUFFSIZE 4096

class SPI_Slave
{
    public:
    static SPI_Slave& VSPI_Instance();
    static SPI_Slave& HSPI_Instance();

    SPI_Slave(SPI_Slave&) = delete;
    SPI_Slave operator=(SPI_Slave&) = delete;

    bool PutMessageOnTXQueue(uint8_t* TX_buf);
    bool GetMessageOnRXQueue(DMASmartPointer<uint8_t>& smt_ptr);

    void IRAM_ATTR Pre_Callback(spi_transaction_t* trans);
    void IRAM_ATTR Pos_Callback(spi_transaction_t* trans);

    private:
    SPI_Slave(spi_host_device_t Id);
    ~SPI_Slave();

    void VSPI_INIT();
    void HSPI_INIT();

    void Pre_routine();
    void Pos_routine();

    spi_host_device_t Slave_Id;

    bool SPI_transaction_ongoing = false;
    std::queue<DMASmartPointer<uint8_t>> RX_queue;
};

#endif