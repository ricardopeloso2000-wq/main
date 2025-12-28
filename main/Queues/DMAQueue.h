#ifndef DMAQUEUE_H
#define DMAQUEUE_H

#include "esp_log.h"


#include "../SmartPointers/DMASmartPointer.h"

class DMAQueue
{
    public:
    DMAQueue(size_t size , spi_host_device_t Id , size_t BuffSize);
    ~DMAQueue();

    void push();
    void push(uint8_t* Ptr , size_t size);
    void pop();

    bool empty();
    size_t size();

    uint8_t* back();
    uint8_t* front();

    private:
    uint16_t m_front;
    uint16_t m_back;
    uint16_t m_maxsize;
    uint16_t m_size;
    DMASmartPointer<DMASmartPointer<uint8_t>> Queue;
};

#endif