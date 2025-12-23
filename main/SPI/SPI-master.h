#ifndef SPIM_H
#define SPIM_H

#include <queue>
#include "driver/spi_master.h"
#include "esp_log.h"
#include "SmartPointers/DMASmartPointer.h"

static const char* SPI_Tag = "SPI_Master";

#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_CLK  18
#define VSPI_CS   5

#define HSPI_MOSI 13
#define HSPI_MISO 12
#define HSPI_CLK 14
#define HSPI_CS   15

#define MASTER_RX_QUEUE_SIZE 6
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
    bool GetLastRecivedMessage(DMASmartPointer<uint8_t>& smt_ptr);

    void Pre_Callback(spi_transaction_t* t);
    void Pos_Callback(spi_transaction_t* t);

    private:
    SPI_master(spi_host_device_t Id);
    ~SPI_master();

    void VSPI_INIT();
    void HSPI_INIT();

    void Pre_routine();
    void Pos_routine();
    
    spi_host_device_t Spi_Id;
    bool Transaction_ongoing = false;

    std::queue<DMASmartPointer<uint8_t>> RX_queue;
    spi_device_handle_t SPI_Handle;
};


#endif