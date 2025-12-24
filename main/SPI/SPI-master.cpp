#include "SPI-master.h"

SPI_master& SPI_master::VSPI_Instance()
{
    static SPI_master VSPI_inst(VSPI_HOST);
    return VSPI_inst;
}
SPI_master& SPI_master::HSPI_Instance()
{
    static SPI_master HSPI_inst(HSPI_HOST);
    return HSPI_inst;
}

SPI_master::SPI_master(spi_host_device_t Id)
{
    Spi_Id = Id;
    if(Id == VSPI_HOST)VSPI_INIT();
    if(Id == HSPI_HOST)HSPI_INIT();

   
}
//Initiates the VSPI device
void SPI_master::VSPI_INIT()
{
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = VSPI_MOSI;
    buscfg.miso_io_num = VSPI_MISO;
    buscfg.sclk_io_num = VSPI_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;
   

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = SPI_MASTER_FREQ_20M;
    devcfg.mode = 0;
    devcfg.spics_io_num = VSPI_CS;
    devcfg.duty_cycle_pos = 128;
    devcfg.cs_ena_posttrans = 3;
    devcfg.queue_size = 1;
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

    xTaskCreatePinnedToCore(SPI_master::TransmitThread , "VSPI Thread" , 512 , this , 5 , &Thread , 1);
}

//Initiates the HSPI device
void SPI_master::HSPI_INIT()
{
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = HSPI_MOSI;
    buscfg.miso_io_num = HSPI_MISO;
    buscfg.sclk_io_num = HSPI_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 0;
   

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = SPI_MASTER_FREQ_20M;
    devcfg.mode = 0;
    devcfg.spics_io_num = HSPI_CS;
    devcfg.duty_cycle_pos = 128;
    devcfg.cs_ena_posttrans = 3;
    devcfg.queue_size = 1;
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

    xTaskCreatePinnedToCore(SPI_master::TransmitThread , "HSPI Thread" , 512 , this , 5 , &Thread , 1);
}

void SPI_master::TransmitThread(void* pvParameters)
{
    auto inst = static_cast<SPI_master*>(pvParameters);
    inst->TrasmitThread_routine();
}

void SPI_master::TrasmitThread_routine()
{
    while(true)
    {
        while(!TX_queue.empty())
        {
            Transaction_ongoing = true;

            spi_transaction_t t;
            t.length = BUFFSIZE;
            t.tx_buffer = TX_queue.front().GetPointer();
            t.rx_buffer = RX_queue.back().GetPointer();

            xSemaphoreTake(rdysem, portMAX_DELAY); //Wait for Slave to be ready
            spi_device_transmit(SPI_Handle, &t);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

SPI_master::~SPI_master()
{
    if(spi_bus_remove_device(SPI_Handle) != ESP_OK)
    {
        ESP_LOGE(SPI_Tag , "Unable to Free Master SPI device");
    }
    vTaskDelete(Thread);
}

void SPI_master::Pos_Callback(spi_transaction_t* trans)
{
    auto inst = static_cast<SPI_master*>(trans->user);
    inst->Pos_routine();
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

bool SPI_master::PutMessageOnTXQueue(DMASmartPointer<uint8_t>& smt_ptr)
{
    if(smt_ptr.GetPointer() == nullptr) return false;
    if(RX_queue.size() >= MASTER_TX_QUEUE_SIZE) return false;

    RX_queue.push(smt_ptr);
    return true;
}

