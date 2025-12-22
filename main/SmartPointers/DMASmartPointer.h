#ifndef SMARTPOINTER_H
#define SMARTPOINTER_H

#include "driver/spi_master.h"

template <typename C>
class DMASmartPointer
{
    public:
    DMASmartPointer();
    DMASmartPointer(C* NewPtr);
    ~DMASmartPointer();

    C* GetPointer() const noexcept;
    void SetPointer(C* NewPtr) noexcept;

    void operator=(DMASmartPointer<C>& Old_SmartPtr) noexcept;

    private:
    C* pointer;
};


#endif