#include "SdkStruct.h"


namespace BackendBf4
{
    bool SDK_STRUCT::SDK_STRUCT_MEMBER::operator<(const SDK_STRUCT_MEMBER& other)
    {
        return m_offset < other.m_offset;
    }


    bool SortByRuntimeId(PSDK_STRUCT left, PSDK_STRUCT right)
    {
        return left->m_rawValueInfo->m_RuntimeId < right->m_rawValueInfo->m_RuntimeId;
    }


    FindByRawValueInfo::FindByRawValueInfo(fb::ValueTypeInfo* valueInfo)
    {
        m_valueInfo = valueInfo;
    }


    bool FindByRawValueInfo::operator()(const PSDK_STRUCT& other) const
    {
        return m_valueInfo == other->m_rawValueInfo;
    }
}