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

void SPI_Slave::Pos_Callback(spi_slave_transaction_t* trans)
{
    auto Inst = static_cast<SPI_Slave*>(trans->user);
    Inst->Pos_routine();
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

void SPI_Slave::Pos_routine()
{
    SPI_transaction_ongoing = false;
}


bool SPI_Slave::PutMessageOnTXQueue(uint8_t* TX_buf)
{
    if(TX_buf == nullptr) return false;
    if(RX_queue.size() >= SLAVE_RX_QUEUE_SIZE)
    {
        ESP_LOGI(SPI_Tag , "Queue Full droping last RX_buf");
        RX_queue.pop();
    }

    RX_queue.push(DMASmartPointer<uint8_t>((uint8_t*)spi_bus_dma_memory_alloc(Slave_Id , BUFFSIZE , 0)));

    spi_slave_transaction_t trans;
    trans.length = BUFFSIZE * 8;
    trans.rx_buffer = RX_queue.back().GetPointer();
    trans.tx_buffer = TX_buf;
    trans.user = this;

    spi_slave_queue_trans(Slave_Id , &trans , portMAX_DELAY);

    return true;
}   

bool SPI_Slave::GetMessageOnRXQueue(DMASmartPointer<uint8_t>& smt_ptr)
{
    if(RX_queue.empty()) return false;
    if(RX_queue.size() == 1 && SPI_transaction_ongoing) return false;

    smt_ptr = RX_queue.front();
    RX_queue.pop();
    return true;
}


