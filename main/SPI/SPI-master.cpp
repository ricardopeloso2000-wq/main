#include "SPI-master.h"

SPI_master& SPI_master::VSPI_Instance()
{
    static SPI_master VSPI_inst(0);
    return VSPI_inst;
}
SPI_master& SPI_master::HSPI_Instance()
{
    static SPI_master HSPI_inst(1);
    return HSPI_inst;
}

SPI_master::SPI_master(int mode)
{
    //If mode = 0 VSPI
    //If mode = 1 HSPI

    if(!mode)
    {
        VSPI_INIT();
    }
    else
    {
        HSPI_INIT();
    }
}
//Initiates the VSPI device
void SPI_master::VSPI_INIT()
{
    Spi_Id = VSPI_HOST;

    spi_bus_config_t buscfg;
    buscfg.mosi_io_num = VSPI_MOSI;
    buscfg.miso_io_num = VSPI_MISO;
    buscfg.sclk_io_num = VSPI_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;
   

    spi_device_interface_config_t devcfg;
    devcfg.clock_speed_hz = SPI_MASTER_FREQ_20M;
    devcfg.mode = 1;
    devcfg.spics_io_num = VSPI_CS;
    devcfg.queue_size = 6;
    devcfg.flags = SPI_DEVICE_NO_DUMMY;

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

//Initiates the HSPI device
void SPI_master::HSPI_INIT()
{
    Spi_Id = HSPI_HOST;

    spi_bus_config_t buscfg;
    buscfg.mosi_io_num = HSPI_MOSI;
    buscfg.miso_io_num = HSPI_MISO;
    buscfg.sclk_io_num = HSPI_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;
   

    spi_device_interface_config_t devcfg;
    devcfg.clock_speed_hz = SPI_MASTER_FREQ_20M;
    devcfg.mode = 1;
    devcfg.spics_io_num = HSPI_CS;
    devcfg.queue_size = 6;
    devcfg.flags = SPI_DEVICE_NO_DUMMY;

    switch(spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO))
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

    switch(spi_bus_add_device(HSPI_HOST, &devcfg, &SPI_Handle))
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

void SPI_master::Pos_Callback(spi_transaction_t* trans)
{
    auto inst = static_cast<SPI_master*>(trans->user);
    inst->Pos_routine();
}

void SPI_master::Pre_Callback(spi_transaction_t* trans)
{
    auto inst = static_cast<SPI_master*>(trans->user);
    inst->Pre_routine();
}

void SPI_master::Pre_routine()
{
    Transaction_ongoing = true;
}

void SPI_master::Pos_routine()
{
    Transaction_ongoing = false;
}

void SPI_master::SPI_LockBus()
{
    spi_device_acquire_bus(SPI_Handle , portMAX_DELAY);
}

void SPI_master::SPI_UnLockBus()
{
    spi_device_release_bus(SPI_Handle);
}

bool SPI_master::GetLastRecivedMessage(DMASmartPointer<uint8_t>& smt_ptr)
{
    if(RX_queue.empty()) return false;

    if(Transaction_ongoing && RX_queue.size() == 1) return false;

    smt_ptr = RX_queue.front();
    RX_queue.pop();
    return true;
}

bool SPI_master::QueueSPITransation(const uint8_t* TXBuf , uint8_t Flags)
{
    if(TXBuf == nullptr) return false;
    if(RX_queue.size() >= QUEUE_SIZE)
    {
        ESP_LOGE(SPI_Tag , "Dropping last Recived RX_buf");
        RX_queue.pop();
    }

    RX_queue.push(DMASmartPointer<uint8_t>((uint8_t*)spi_bus_dma_memory_alloc(Spi_Id , BUFFSIZE , 0)));

    spi_transaction_t trans = {};
    trans.length = BUFFSIZE * 8;
    trans.user = this;
    trans.tx_buffer = TXBuf;
    trans.rx_buffer = RX_queue.back().GetPointer();

    if(spi_device_queue_trans(SPI_Handle , &trans , portMAX_DELAY) != ESP_OK) return false;

    return true;
}

