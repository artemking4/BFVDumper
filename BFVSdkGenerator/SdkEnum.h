#pragma once


#include <string>
#include <vector>


#include "Sdk.h"


namespace BackendBf4
{
    struct SDK_ENUM
    {
        struct SDK_ENUM_FIELD
        {
            fb::FieldInfo::FieldInfoData* m_rawFieldInfoData;

            std::string m_name;
            unsigned int m_offset;

            bool operator<(const SDK_ENUM_FIELD& other);
        };
        typedef SDK_ENUM_FIELD* PSDK_ENUM_FIELD;

        fb::EnumFieldInfo* m_rawEnumInfo;

        std::string m_name;
        unsigned int m_runtimeId;

        std::vector<SDK_ENUM_FIELD> m_sdkEnumFields;
    };
    typedef SDK_ENUM* PSDK_ENUM;
}