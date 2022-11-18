#pragma once


#include <vector>
#include <algorithm>


#include "Sdk.h"


#include "SdkClass.h"
#include "SdkEnum.h"
#include "SdkStruct.h"
#include "json.hpp"

namespace BackendBf4
{
    class SDK_GENERATOR
    {
    private:
        std::vector<fb::TypeInfo*>* m_typeInfos;
        std::vector<fb::ClassInfo*>* m_classInfos;
        std::vector<fb::EnumFieldInfo*>* m_enumInfos;
        std::vector<fb::ValueTypeInfo*>* m_valueInfos;
        std::vector<PSDK_CLASS>* m_sdkClasses;
        std::vector<PSDK_ENUM>* m_sdkEnums;
        std::vector<PSDK_STRUCT>* m_sdkStructs;

    public:
        SDK_GENERATOR();
        virtual ~SDK_GENERATOR();

        void Register(fb::TypeInfo* typeInfo);
        void Analyze();
        nlohmann::json Generate();

    private:
        void AnalyzeClasses();
        void AnalyzeEnums();
        void AnalyzeStructs();
        void GenerateDeclarations(std::vector<std::string>& result);
        void GenerateClasses(std::vector<std::string>& result);
        void GenerateEnums(std::vector<std::string>& result);
        void GenerateStructs(std::vector<std::string>& result);
        std::string GetTypeName(fb::TypeInfo* typeInfo);
        unsigned int GetTypeSize(fb::TypeInfo* typeInfo);
        void GetDependenciesForStructMember(fb::TypeInfo* typeInfo, std::vector<PSDK_STRUCT>& dependencies);
    };
    typedef SDK_GENERATOR* PSDK_GENERATOR;
}