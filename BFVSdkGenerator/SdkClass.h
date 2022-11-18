#pragma once


#include <string>
#include <vector>


#include "Sdk.h"


namespace BackendBf4
{
    struct SDK_CLASS
    {
        struct SDK_CLASS_MEMBER
        {
            fb::FieldInfo::FieldInfoData* m_rawFieldInfoData;

            std::string m_name;
            std::string m_type;
            unsigned int m_offset;
            unsigned int m_size;
            bool m_isPad;

            bool operator<(const SDK_CLASS_MEMBER& other);
        };
        typedef SDK_CLASS_MEMBER* PSDK_CLASS_MEMBER;

        fb::ClassInfo* m_rawClassInfo;
        fb::DataContainer* m_rawDefaultInstance;

        SDK_CLASS* m_parent;
        std::string m_name;
        unsigned int m_classId;
        unsigned int m_runtimeId;
        unsigned int m_size;
        unsigned short m_alignment;

        std::vector<SDK_CLASS_MEMBER> m_sdkClassMembers;
        std::vector<SDK_CLASS*> m_sdkClassDependencies;
    };
    typedef SDK_CLASS* PSDK_CLASS;


    struct FindByRawClassInfo
    {
        explicit FindByRawClassInfo(fb::ClassInfo* classInfo);

        fb::ClassInfo* m_classInfo;

        bool operator()(const PSDK_CLASS& other) const;
    };
}