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
    devcfg.queue_size = 6;
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
    devcfg.queue_size = 6;
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

bool SPI_Slave::PutMessageOnTXQueue(uint8_t* TX_buf)
{
    if(TX_buf == nullptr) return false;
    if(RX_queue.size() >= 6)
    {
        ESP_LOGI(SPI_Tag , "Queue Full droping last RX_buf");
        RX_queue.pop();
    }

    

}   
