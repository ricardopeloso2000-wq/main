#ifndef SPIM_H
#define SPIM_H

#include "driver/spi_master.h"
#include "esp_log.h"

static const char* SPI_Tag = "SPI_Master";

#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_CLK  18
#define VSPI_CS   5

#define HSPI_MOSI 13
#define HSPI_MISO 12
#define HSPI_CLK 14
#define HSPI_CS   15

#define BUFFSIZE 4096

class SPI_master
{
    public:
    static SPI_master& VSPI_Instance();
    static SPI_master& HSPI_Instance();

    SPI_master(SPI_master&) = delete;
    SPI_master operator=(SPI_master&) = delete;

    void SPI_LockBus();
    void SPI_UnLockBus();
    
    bool QueueSPITransation(const uint8_t* TXBuf , uint8_t Flags);

    private:
    SPI_master(int mode);
    ~SPI_master();

    void VSPI_INIT();
    void HSPI_INIT();
    
    bool SPI_transaction_ongoing = false;

    uint8_t RX_BUF[BUFFSIZE];
    spi_device_handle_t SPI_Handle;
};


#endif