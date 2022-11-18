#include "SdkEnum.h"


namespace BackendBf4
{
    bool SDK_ENUM::SDK_ENUM_FIELD::operator<(const SDK_ENUM_FIELD& other)
    {
        return m_offset < other.m_offset;
    }
}