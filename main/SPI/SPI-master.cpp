#include "SPI-master.h"

SPI_master SPI_master::SPI_03;

void IRAM_ATTR SPI_master::PreTransCB(spi_transaction_t *trans)
{
    auto *self = static_cast<SPI_master*>(trans->user);
    self->on_pre(trans);
}
void IRAM_ATTR SPI_master::PosTransCB(spi_transaction_t *trans)
{
    auto *self = static_cast<SPI_master*>(trans->user);
    self->on_post(trans);
}

SPI_master::SPI_master()
{
    spi_bus_config_t buscfg;
    buscfg.mosi_io_num = PIN_NUM_MOSI;
    buscfg.miso_io_num = PIN_NUM_MISO;
    buscfg.sclk_io_num = PIN_NUM_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;
   

    spi_device_interface_config_t devcfg;
    devcfg.clock_speed_hz = SPI_MASTER_FREQ_26M;
    devcfg.mode = 0;
    devcfg.spics_io_num = PIN_NUM_CS;
    devcfg.queue_size = 6;
    devcfg.flags = SPI_DEVICE_NO_DUMMY;
    devcfg.pre_cb = SPI_master::PreTransCB;
    devcfg.post_cb = SPI_master::PosTransCB;

    switch(spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_CH_AUTO))
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

    switch(spi_bus_add_device(VSPI_HOST, &devcfg, &SPI_Handle))
    {
        case ESP_ERR_INVALID_STATE:
            ESP_LOGE(SPI_Tag , "elected clock source is unavailable or spi bus not initialized");
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE(SPI_Tag , "host doesn't have any free CS slots");
            break;
        case ESP_ERR_NO_MEM:
            ESP_LOGE(SPI_Tag , "ESP out of memory");
            break;
        default:
            break;
    }
}

SPI_master::~SPI_master()
{
    if(spi_bus_remove_device(SPI_Handle) != ESP_OK)
    {
        ESP_LOGE(SPI_Tag , "Unable to Free Master SPI device");
    }
}

void SPI_master::SPI_LockBus()
{
    spi_device_acquire_bus(SPI_Handle , portMAX_DELAY);
}

void SPI_master::SPI_UnLockBus()
{
    spi_device_release_bus(SPI_Handle);
}

void SPI_master::GetLastRXBuf(DMASmartPointer<uint8_t>& DMAptr)
{
    DMAptr = RX_queue.front();
    RX_queue.pop();
}

bool SPI_master::QueueSPITransation(const uint8_t* TXBuf , uint8_t Flags)
{
    if(TXBuf == nullptr) return false;
    if(TXqueue >= 6) return false;
    TXqueue ++;

    uint8_t* RXBuf = (uint8_t*)spi_bus_dma_memory_alloc(VSPI_HOST , BUFFSIZE , MALLOC_CAP_INTERNAL);
    if(RXBuf == nullptr) return false;

    RX_queue.push(DMASmartPointer<uint8_t>(RXBuf));

    spi_transaction_t trans;
    trans.length = BUFFSIZE;
    trans.user = this;
    trans.tx_buffer = TXBuf;
    trans.rx_buffer = RXBuf;

    return true;
}

