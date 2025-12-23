#ifndef SPI_SLAVE_H
#define SPI_SLAVE_H

#include <queue>
#include "../SmartPointers/DMASmartPointer.h"
#include "driver/spi_slave.h"

static const char* SPI_Tag = "SPI_slave";

#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_CLK  18
#define VSPI_CS   5

#define HSPI_MOSI 13
#define HSPI_MISO 12
#define HSPI_SCLK 14
#define HSPI_CS   15

class SPI_Slave
{
    public:
    SPI_Slave& VSPI_Instance();
    SPI_Slave& HSPI_Instance();

    SPI_Slave(SPI_Slave&) = delete;
    SPI_Slave operator=(SPI_Slave&) = delete;

    private:
    std::queue<DMASmartPointer<uint8_t>> RX_queue;
};

#endif