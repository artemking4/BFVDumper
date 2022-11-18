#pragma once


#include <string>
#include <vector>


#include "Sdk.h"


namespace BackendBf4
{
    struct SDK_STRUCT
    {
        struct SDK_STRUCT_MEMBER
        {
            fb::FieldInfo::FieldInfoData* m_rawFieldInfoData;

            std::string m_name;
            std::string m_type;
            unsigned int m_offset;
            unsigned int m_size;
            bool m_isPad;

            bool operator<(const SDK_STRUCT_MEMBER& other);
        };
        typedef SDK_STRUCT_MEMBER* PSDK_STRUCT_MEMBER;

        fb::ValueTypeInfo* m_rawValueInfo;

        std::string m_name;
        unsigned int m_runtimeId;
        unsigned int m_size;

        std::vector<SDK_STRUCT_MEMBER> m_sdkStructMembers;
        std::vector<SDK_STRUCT*> m_sdkStructDependencies;
    };
    typedef SDK_STRUCT* PSDK_STRUCT;


    struct FindByRawValueInfo
    {
        explicit FindByRawValueInfo(fb::ValueTypeInfo* valueInfo);

        fb::ValueTypeInfo* m_valueInfo;

        bool operator()(const PSDK_STRUCT& other) const;
    };
}