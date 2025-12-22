#ifndef SPIM_H
#define SPIM_H

#include <queue>
#include "driver/spi_master.h"
#include "esp_log.h"

#include "../SmartPointers/DMASmartPointer.h"

static const char* SPI_Tag = "SPI";

#define RX_QUEUE_SIZE 6
#define PIN_NUM_MOSI 23
#define PIN_NUM_MISO 19
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

#define BUFFSIZE 4096

class SPI_master
{
    public:
    SPI_master();
    ~SPI_master();

    void SPI_LockBus();
    void SPI_UnLockBus();

    void GetLastRXBuf(DMASmartPointer<uint8_t>& DMAPtr);
    
    bool QueueSPITransation(const uint8_t* TXBuf , uint8_t Flags);
    
    static void IRAM_ATTR PreTransCB(spi_transaction_t *trans);
    static void IRAM_ATTR PosTransCB(spi_transaction_t *trans);

    static uint8_t SPI_HOSTS;

    static SPI_master SPI_03;
    SPI_master(SPI_master&) = delete;
    SPI_master operator=(SPI_master&) = delete;

    private:
    
    uint8_t TXqueue;
    bool SPI_transaction_ongoing = false;

    void on_pre(spi_transaction_t *)
    {
       SPI_transaction_ongoing = true;
    }

    void on_post(spi_transaction_t *)
    {
        SPI_transaction_ongoing = false;
        TXqueue--;
    }

    std::queue<DMASmartPointer<uint8_t>> RX_queue;
    spi_device_handle_t SPI_Handle;
};


#endif