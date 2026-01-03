#include "SPI-Slave.h"

SPI_Slave& SPI_Slave::VSPI_Instance()
{
    static SPI_Slave VSPI_Slave_Isnt(VSPI_HOST);
    return VSPI_Slave_Isnt;
}

SPI_Slave& SPI_Slave::HSPI_Instance()
{
    static SPI_Slave HSPI_Slave_Isnt(HSPI_HOST);
    return HSPI_Slave_Isnt;
}

SPI_Slave::SPI_Slave(spi_host_device_t Id)
{
    Slave_Id = Id;
    if(Id == VSPI_HOST) VSPI_INIT();
    if(Id == HSPI_HOST) HSPI_INIT();
}

void SPI_Slave::VSPI_INIT()
{
    spi_bus_config_t buscfg;
    buscfg.mosi_io_num = VSPI_MOSI;
    buscfg.miso_io_num = VSPI_MISO;
    buscfg.sclk_io_num = VSPI_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;
   
    spi_slave_interface_config_t devcfg;
    devcfg.mode = 1;
    devcfg.queue_size = 1;
    devcfg.spics_io_num = VSPI_CS;

    switch(spi_slave_initialize(Slave_Id, &buscfg, &devcfg , SPI_DMA_CH_AUTO))
    {
        case ESP_ERR_INVALID_STATE:
            ESP_LOGE(SPI_Tag , "host already is in use");
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE(SPI_Tag , "there is no available DMA channel");
            break;
        case ESP_ERR_NO_MEM:
            ESP_LOGE(SPI_Tag , "ESP out of memory");
            break;
        default:
            break;
    }
}

void SPI_Slave::HSPI_INIT()
{
    spi_bus_config_t buscfg;
    buscfg.mosi_io_num = HSPI_MOSI;
    buscfg.miso_io_num = HSPI_MISO;
    buscfg.sclk_io_num = HSPI_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;
   
    spi_slave_interface_config_t devcfg;
    devcfg.mode = 1;
    devcfg.queue_size = 1;
    devcfg.spics_io_num = HSPI_CS;

    switch(spi_slave_initialize(Slave_Id, &buscfg, &devcfg , SPI_DMA_CH_AUTO))
    {
        case ESP_ERR_INVALID_STATE:
            ESP_LOGE(SPI_Tag , "host already is in use");
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE(SPI_Tag , "there is no available DMA channel");
            break;
        case ESP_ERR_NO_MEM:
            ESP_LOGE(SPI_Tag , "ESP out of memory");
            break;
        default:
            break;
    }
}

SPI_Slave::~SPI_Slave()
{
    spi_slave_free(Slave_Id);
}

void SPI_Slave::VSPI_GPIO_Callback(void* inst)
{
    static uint32_t lasthandshaketime_us;
    uint32_t currtime_us = esp_timer_get_time();
    uint32_t diff = currtime_us - lasthandshaketime_us;
    if (diff < 1000) {
        return; //ignore everything <1ms after an earlier irq
    }
    lasthandshaketime_us = currtime_us;

    auto i = static_cast<SPI_Slave*>(inst);
    i->GPIO_routine();
}

void SPI_Slave::HSPI_GPIO_Callback(void* inst)
{
    static uint32_t lasthandshaketime_us;
    uint32_t currtime_us = esp_timer_get_time();
    uint32_t diff = currtime_us - lasthandshaketime_us;
    if (diff < 1000) {
        return; //ignore everything <1ms after an earlier irq
    }
    lasthandshaketime_us = currtime_us;

    auto i = static_cast<SPI_Slave*>(inst);
    i->GPIO_routine();
}

void SPI_Slave::GPIO_routine()
{
    if(TX_queue.empty()) Master_Sending= true;

    BaseType_t mustYield = false;
    xSemaphoreGiveFromISR(rdysem, &mustYield);
    if (mustYield) {
        portYIELD_FROM_ISR();
    }

}

void SPI_Slave::TransmitThread(void* pvParameters)
{
    auto inst =  static_cast<SPI_Slave*>(pvParameters);
    inst->TransmitThread_routine();
}

void SPI_Slave::TransmitThread_routine()
{
    while(1)
    {
        while(!TX_queue.empty() || Master_Sending)
        {



        }

    }
}

bool SPI_Slave::PutMessageOnTXQueue(const DMASmartPointer<uint8_t>& TX_ptr , size_t size)
{
    if(TX_ptr.GetPointer() == nullptr) return false;
    if(TX_queue.size() >= SLAVE_TX_QUEUE_SIZE) return false;

    TX_queue.push(TX_ptr.GetPointer() , size);
    return true;
}   

bool SPI_Slave::GetMessageOnRXQueue(DMASmartPointer<uint8_t>& smt_ptr)
{
    if(RX_queue.empty()) return false;
    if(RX_queue.size() == 1 && transaction_ongoing) return false;

    
    memcpy(smt_ptr.GetPointer() , RX_queue.front() , BUFFSIZE);
    RX_queue.pop();

    return true;
}


