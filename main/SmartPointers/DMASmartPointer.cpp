#include "DMASmartPointer.h"

template<typename C>
DMASmartPointer<C>::DMASmartPointer()
{
    pointer = nullptr;
}

template<typename C>
DMASmartPointer<C>::DMASmartPointer(C* NewPtr)
{
    pointer = NewPtr;
}

template<typename C>
DMASmartPointer<C>::~DMASmartPointer()
{
    free(pointer);
}

template<typename C>
C* DMASmartPointer<C>::GetPointer()const noexcept
{
    return pointer;
}

template<typename C>
void DMASmartPointer<C>::SetPointer(C* NewPtr)noexcept
{
    this->pointer = NewPtr;
}

template<typename C>
void DMASmartPointer<C>::operator=(DMASmartPointer<C>& Old_SmartPtr) noexcept
{
    this->pointer = Old_SmartPtr.GetPointer();
    Old_SmartPtr.SetPointer(nullptr);
}