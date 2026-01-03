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
    m_size = 0;
}

void DMAQueue::push()
{
    if(m_size == m_maxsize) return;

    m_size++;
    m_back++;
    if(m_back == m_maxsize) m_back = 0;
}

void DMAQueue::push(uint8_t* Ptr , size_t size)
{
    m_size++;

    memcpy(Queue.GetPointer()[m_back].GetPointer() , Ptr , size);

    push();
}

void DMAQueue::pop()
{
    if(empty()) return;

    m_size--;
    m_front++;
    if(m_front == m_maxsize) m_front = 0;
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

bool DMAQueue::empty()
{
    if(m_size == 0) return true;
    return false;
}

size_t DMAQueue::size()
{
    return m_size;
}