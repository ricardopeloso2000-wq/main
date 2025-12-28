#include "DMAQueue.h"

DMAQueue::DMAQueue(size_t max_size , spi_host_device_t Id , size_t BuffSize) : m_maxsize(max_size)
{
    Queue.SetPointer((DMASmartPointer<uint8_t>*)malloc(m_maxsize*sizeof(DMASmartPointer<uint8_t>)));

    for(int i = 0 ; i < m_maxsize ; i++)
    {
        Queue.GetPointer()[i].SetPointer((uint8_t*)spi_bus_dma_memory_alloc(Id , BuffSize , 0));
    }

    m_front = 0;
    m_back = 0;
}

void DMAQueue::push()
{
    m_back++;
    if(m_back >= m_maxsize - 1) m_back -= (m_maxsize-1);
    if(m_back == m_front) m_front++;
}

void DMAQueue::push(uint8_t* Ptr , size_t size)
{
    memcpy(Queue.GetPointer()[m_back].GetPointer() , Ptr , size);

    m_back++;
    if(m_back >= m_maxsize - 1) m_back -= (m_maxsize-1);
    if(m_back == m_front) m_front++;
}

void DMAQueue::pop()
{
    m_front++;
}

uint8_t* DMAQueue::front()
{
    return Queue.GetPointer()[m_front].GetPointer();
}

uint8_t* DMAQueue::back()
{
    if(m_back - 1 > 0)
    {
    return Queue.GetPointer()[m_back-1].GetPointer();
    }
    else
    {
    return Queue.GetPointer()[m_maxsize - 1].GetPointer();
    }
}