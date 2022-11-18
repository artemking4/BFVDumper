#include "SdkClass.h"


namespace BackendBf4
{
    bool SDK_CLASS::SDK_CLASS_MEMBER::operator<(const SDK_CLASS_MEMBER& other)
    {
        return m_offset < other.m_offset;
    }


    FindByRawClassInfo::FindByRawClassInfo(fb::ClassInfo* classInfo)
    {
        m_classInfo = classInfo;
    }


    bool FindByRawClassInfo::operator()(const PSDK_CLASS& other) const
    {
        return m_classInfo == other->m_rawClassInfo;
    }
}