#pragma once

#include "json.hpp"
#include "SdkGenerator.h"


namespace BackendBf4
{
    SDK_GENERATOR::SDK_GENERATOR()
    {
        m_typeInfos = new std::vector<fb::TypeInfo*>();
        m_classInfos = new std::vector<fb::ClassInfo*>();
        m_enumInfos = new std::vector<fb::EnumFieldInfo*>();
        m_valueInfos = new std::vector<fb::ValueTypeInfo*>();
        m_sdkClasses = new std::vector<PSDK_CLASS>();
        m_sdkEnums = new std::vector<PSDK_ENUM>();
        m_sdkStructs = new std::vector<PSDK_STRUCT>();
    }


    SDK_GENERATOR::~SDK_GENERATOR()
    {
        delete m_typeInfos;
        delete m_classInfos;
        delete m_enumInfos;
        delete m_valueInfos;
        for (std::vector<PSDK_CLASS>::iterator iter = m_sdkClasses->begin(); iter != m_sdkClasses->end(); iter++) delete* iter;
        delete m_sdkClasses;
        for (std::vector<PSDK_ENUM>::iterator iter = m_sdkEnums->begin(); iter != m_sdkEnums->end(); iter++) delete* iter;
        delete m_sdkEnums;
        for (std::vector<PSDK_STRUCT>::iterator iter = m_sdkStructs->begin(); iter != m_sdkStructs->end(); iter++) delete* iter;
        delete m_sdkStructs;
    }


    void SDK_GENERATOR::Register(fb::TypeInfo* typeInfo)
    {
        if (std::find(m_typeInfos->begin(), m_typeInfos->end(), typeInfo) == m_typeInfos->end())
            m_typeInfos->push_back(typeInfo);
    }


    void SDK_GENERATOR::Analyze()
    {
        if (!m_sdkClasses->empty()) for (std::vector<PSDK_CLASS>::iterator iter = m_sdkClasses->begin(); iter != m_sdkClasses->end(); iter++) delete* iter;
        if (!m_sdkEnums->empty()) for (std::vector<PSDK_ENUM>::iterator iter = m_sdkEnums->begin(); iter != m_sdkEnums->end(); iter++) delete* iter;
        if (!m_sdkStructs->empty()) for (std::vector<PSDK_STRUCT>::iterator iter = m_sdkStructs->begin(); iter != m_sdkStructs->end(); iter++) delete* iter;

        AnalyzeClasses();
        AnalyzeEnums();
        AnalyzeStructs();
    }


    void SDK_GENERATOR::AnalyzeClasses()
    {
        // Narrow down typeInfos to classInfos
        for (std::vector<fb::TypeInfo*>::iterator iter = m_typeInfos->begin(); iter != m_typeInfos->end(); iter++)
        {
            fb::TypeInfo* typeInfo = *iter;
            if (typeInfo != NULL && typeInfo->GetTypeCode() == fb::kTypeCode_Class) m_classInfos->push_back((fb::ClassInfo*)typeInfo);
        }

        // Create sdk classes based on classInfos
        for (std::vector<fb::ClassInfo*>::iterator iter = m_classInfos->begin(); iter != m_classInfos->end(); iter++)
        {
            // Validate pointer integrity
            fb::ClassInfo* classInfo = *iter;
            if (classInfo == NULL) continue;
            fb::ClassInfo::ClassInfoData* classInfoData = classInfo->GetClassInfoData();
            if (classInfoData == NULL) continue;

            // Validate data integrity
            if (classInfoData->m_Name[0] == NULL || strlen(classInfoData->m_Name) <= 0) continue;
            //if (strstr(classInfoData->m_Name, "::") != NULL) continue; // todo handle such classes "Class::Class"

            // Create sdk class
            PSDK_CLASS sdkClass = new SDK_CLASS();
            sdkClass->m_rawClassInfo = classInfo;
            sdkClass->m_rawDefaultInstance = classInfo->m_DefaultInstance;
            sdkClass->m_name = classInfoData->m_Name;
            sdkClass->m_classId = classInfo->m_ClassId;
            sdkClass->m_runtimeId = classInfo->m_RuntimeId;
            sdkClass->m_size = classInfoData->m_TotalSize;
            sdkClass->m_alignment = classInfoData->m_Alignment;

            // Add sdk class if not existing or discard
            if (std::find(m_sdkClasses->begin(), m_sdkClasses->end(), sdkClass) == m_sdkClasses->end()) m_sdkClasses->push_back(sdkClass);
            else
            {
                delete sdkClass;
                continue;
            }
        }

        // Add class members
        for (std::vector<PSDK_CLASS>::iterator iter = m_sdkClasses->begin(); iter != m_sdkClasses->end(); iter++)
        {
            PSDK_CLASS sdkClass = *iter;
            fb::ClassInfo::ClassInfoData* classInfoData = sdkClass->m_rawClassInfo->GetClassInfoData();

            // Create sdk class member based on fieldInfos
            if (classInfoData->m_FieldCount)
            {
                fb::FieldInfo::FieldInfoData* fields = classInfoData->m_Fields;
                if (fields)
                {
                    for (int i = 0; i < classInfoData->m_FieldCount; i++)
                    {
                        // Validate pointer integrity
                        fb::FieldInfo::FieldInfoData* fieldInfoData = &fields[i];
                        if (fieldInfoData == NULL) continue;
                        fb::TypeInfo* fieldType = fieldInfoData->m_FieldTypePtr;
                        if (fieldType == NULL) continue;
                        fb::TypeInfo::TypeInfoData* fieldTypeData = fieldType->GetTypeInfoData();
                        if (fieldTypeData == NULL) continue;

                        // Validate data integrity
                        if (fieldInfoData->m_Name[0] == NULL || strlen(fieldInfoData->m_Name) <= 0) continue;

                        // Create sdk class member
                        SDK_CLASS::SDK_CLASS_MEMBER sdkClassMember;
                        sdkClassMember.m_rawFieldInfoData = fieldInfoData;
                        sdkClassMember.m_isPad = false;
                        sdkClassMember.m_name = fieldInfoData->m_Name;
                        sdkClassMember.m_offset = fieldInfoData->m_FieldOffset;
                        sdkClassMember.m_size = GetTypeSize(fieldType);
                        sdkClassMember.m_type = GetTypeName(fieldType);
                        sdkClass->m_sdkClassMembers.push_back(sdkClassMember);
                    }

                    // Sort sdk class members by offset
                    std::sort(sdkClass->m_sdkClassMembers.begin(), sdkClass->m_sdkClassMembers.end());
                }
            }
        }

        // Link super classes
        for (std::vector<PSDK_CLASS>::iterator iter = m_sdkClasses->begin(); iter != m_sdkClasses->end(); iter++)
        {
            PSDK_CLASS sdkClass = *iter;
            fb::ClassInfo* classInfo = sdkClass->m_rawClassInfo;
            fb::ClassInfo* superClassInfo = classInfo->m_Super;
            if (superClassInfo != NULL && classInfo != superClassInfo)
            {
                std::vector<PSDK_CLASS>::iterator superSdkClassIterator = std::find_if(m_sdkClasses->begin(), m_sdkClasses->end(), FindByRawClassInfo(superClassInfo));
                if (superSdkClassIterator != m_sdkClasses->end())
                {
                    sdkClass->m_sdkClassDependencies.push_back(*superSdkClassIterator);
                    sdkClass->m_parent = *superSdkClassIterator;
                }
                else sdkClass->m_parent = NULL;
            }
            else sdkClass->m_parent = NULL;
        }

        // Add class paddings
        for (std::vector<PSDK_CLASS>::iterator iter = m_sdkClasses->begin(); iter != m_sdkClasses->end(); iter++)
        {
            PSDK_CLASS sdkClass = *iter;

            unsigned int offsetInClass = sdkClass->m_parent != NULL ? sdkClass->m_parent->m_size : 0;
            for (std::vector<SDK_CLASS::SDK_CLASS_MEMBER>::iterator iter2 = sdkClass->m_sdkClassMembers.begin(); iter2 != sdkClass->m_sdkClassMembers.end(); iter2++)
            {
                unsigned int leftPadding = iter2->m_offset - offsetInClass;
                if (leftPadding > 0)
                {
                    SDK_CLASS::SDK_CLASS_MEMBER sdkClassMember;
                    sdkClassMember.m_offset = offsetInClass;
                    sdkClassMember.m_size = leftPadding;
                    sdkClassMember.m_isPad = true;
                    iter2 = sdkClass->m_sdkClassMembers.insert(iter2, sdkClassMember)++;
                }
                offsetInClass = iter2->m_offset + iter2->m_size;
            }

            unsigned int rightPadding = sdkClass->m_size - offsetInClass;
            if (rightPadding > 0)
            {
                SDK_CLASS::SDK_CLASS_MEMBER sdkClassMember;
                sdkClassMember.m_offset = offsetInClass;
                sdkClassMember.m_size = rightPadding;
                sdkClassMember.m_isPad = true;
                sdkClass->m_sdkClassMembers.push_back(sdkClassMember);
            }
        }

        // Sort dependencies
    was_swap:
        for (std::vector<PSDK_CLASS>::iterator sdkClassIter = m_sdkClasses->begin(); sdkClassIter != m_sdkClasses->end(); sdkClassIter++)
        {
            PSDK_CLASS sdkClass = *sdkClassIter;

            for (std::vector<PSDK_CLASS>::iterator sdkClassDependencyIter = sdkClass->m_sdkClassDependencies.begin(); sdkClassDependencyIter != sdkClass->m_sdkClassDependencies.end(); sdkClassDependencyIter++)
            {
                std::vector<PSDK_CLASS>::iterator positionOfDependency = std::find(m_sdkClasses->begin(), m_sdkClasses->end(), *sdkClassDependencyIter);
                if (positionOfDependency > sdkClassIter) // is behind class containing the reference => swap
                {
                    std::iter_swap(sdkClassIter, positionOfDependency);
                    goto was_swap;
                }
            }
        }
    }


    void SDK_GENERATOR::AnalyzeEnums()
    {
        // Narrow down typeInfos to enumInfos
        for (std::vector<fb::TypeInfo*>::iterator iter = m_typeInfos->begin(); iter != m_typeInfos->end(); iter++)
        {
            fb::TypeInfo* typeInfo = *iter;
            if (typeInfo != NULL && typeInfo->GetTypeCode() == fb::kTypeCode_Enum) m_enumInfos->push_back((fb::EnumFieldInfo*)typeInfo);
        }

        // Create sdk enums based on enumInfos
        for (std::vector<fb::EnumFieldInfo*>::iterator iter = m_enumInfos->begin(); iter != m_enumInfos->end(); iter++)
        {
            // Validate pointer integrity
            fb::EnumFieldInfo* enumInfo = *iter;
            if (enumInfo == NULL) continue;
            fb::EnumFieldInfo::EnumFieldInfoData* enumInfoData = enumInfo->GetEnumInfoData();
            if (enumInfoData == NULL) continue;

            // Validate data integrity
            if (enumInfoData->m_Name[0] == NULL || strlen(enumInfoData->m_Name) <= 0) continue;

            // Create sdk enum
            PSDK_ENUM sdkEnum = new SDK_ENUM();
            sdkEnum->m_rawEnumInfo = enumInfo;
            sdkEnum->m_name = enumInfoData->m_Name;
            sdkEnum->m_runtimeId = enumInfo->m_RuntimeId;

            // Add sdk enum if not existing or discard
            if (std::find(m_sdkEnums->begin(), m_sdkEnums->end(), sdkEnum) == m_sdkEnums->end()) m_sdkEnums->push_back(sdkEnum);
            else
            {
                delete sdkEnum;
                continue;
            }
        }

        // Add enum fields
        for (std::vector<PSDK_ENUM>::iterator iter = m_sdkEnums->begin(); iter != m_sdkEnums->end(); iter++)
        {
            PSDK_ENUM sdkEnum = *iter;
            fb::EnumFieldInfo::EnumFieldInfoData* enumInfoData = sdkEnum->m_rawEnumInfo->GetEnumInfoData();

            // Create sdk enum field based on fieldInfos
            if (enumInfoData->m_FieldCount)
            {
                fb::FieldInfo::FieldInfoData* fields = enumInfoData->m_Fields;
                if (fields)
                {
                    for (int i = 0; i < enumInfoData->m_FieldCount; i++)
                    {
                        // Validate pointer integrity
                        fb::FieldInfo::FieldInfoData* fieldInfoData = &fields[i];
                        if (fieldInfoData == NULL) continue;

                        // Validate data integrity
                        if (fieldInfoData->m_Name[0] == NULL || strlen(fieldInfoData->m_Name) <= 0) continue;

                        // Create sdk class member
                        SDK_ENUM::SDK_ENUM_FIELD sdkEnumField;
                        sdkEnumField.m_rawFieldInfoData = fieldInfoData;
                        sdkEnumField.m_name = fieldInfoData->m_Name;
                        sdkEnumField.m_offset = (unsigned int)i;
                        sdkEnum->m_sdkEnumFields.push_back(sdkEnumField);
                    }

                    // Sort sdk enum fields by offset
                    std::sort(sdkEnum->m_sdkEnumFields.begin(), sdkEnum->m_sdkEnumFields.end());
                }
            }
        }
    }


    void SDK_GENERATOR::AnalyzeStructs()
    {
        // Narrow down typeInfos to valueInfos
        for (std::vector<fb::TypeInfo*>::iterator iter = m_typeInfos->begin(); iter != m_typeInfos->end(); iter++)
        {
            fb::TypeInfo* typeInfo = *iter;
            if (typeInfo != NULL && typeInfo->GetTypeCode() == fb::kTypeCode_ValueType) m_valueInfos->push_back((fb::ValueTypeInfo*)typeInfo);
        }

        // Create sdk structs based on valueInfos
        for (std::vector<fb::ValueTypeInfo*>::iterator iter = m_valueInfos->begin(); iter != m_valueInfos->end(); iter++)
        {
            // Validate pointer integrity
            fb::ValueTypeInfo* valueInfo = *iter;
            if (valueInfo == NULL) continue;
            fb::ValueTypeInfo::ValueTypeInfoData* valueInfoData = valueInfo->GetValueInfoData();
            if (valueInfoData == NULL) continue;

            // Validate data integrity
            if (valueInfoData->m_Name[0] == NULL || strlen(valueInfoData->m_Name) <= 0) continue;

            // Create sdk struct
            PSDK_STRUCT sdkStruct = new SDK_STRUCT();
            sdkStruct->m_rawValueInfo = valueInfo;
            sdkStruct->m_name = valueInfoData->m_Name;
            sdkStruct->m_runtimeId = valueInfo->m_RuntimeId;
            sdkStruct->m_size = valueInfoData->m_TotalSize;

            // Add sdk struct if not existing or discard
            if (std::find(m_sdkStructs->begin(), m_sdkStructs->end(), sdkStruct) == m_sdkStructs->end()) m_sdkStructs->push_back(sdkStruct);
            else
            {
                delete sdkStruct;
                continue;
            }
        }

        // Add struct members
        for (std::vector<PSDK_STRUCT>::iterator iter = m_sdkStructs->begin(); iter != m_sdkStructs->end(); iter++)
        {
            PSDK_STRUCT sdkStruct = *iter;
            fb::ValueTypeInfo::ValueTypeInfoData* valueInfoData = sdkStruct->m_rawValueInfo->GetValueInfoData();

            // Create sdk struct member based on fieldInfos
            if (valueInfoData->m_FieldCount)
            {
                fb::FieldInfo::FieldInfoData* fields = valueInfoData->m_Fields;
                if (fields)
                {
                    for (int i = 0; i < valueInfoData->m_FieldCount; i++)
                    {
                        // Validate pointer integrity
                        fb::FieldInfo::FieldInfoData* fieldInfoData = &fields[i];
                        if (fieldInfoData == NULL) continue;
                        fb::TypeInfo* fieldType = fieldInfoData->m_FieldTypePtr;
                        if (fieldType == NULL) continue;
                        fb::TypeInfo::TypeInfoData* fieldTypeData = fieldType->GetTypeInfoData();

                        // Validate data integrity
                        if (fieldInfoData->m_Name[0] == NULL || strlen(fieldInfoData->m_Name) <= 0) continue;

                        // Create sdk class member
                        SDK_STRUCT::SDK_STRUCT_MEMBER sdkStructMember;
                        sdkStructMember.m_rawFieldInfoData = fieldInfoData;
                        sdkStructMember.m_isPad = false;
                        sdkStructMember.m_name = fieldInfoData->m_Name;
                        sdkStructMember.m_offset = fieldInfoData->m_FieldOffset;
                        sdkStructMember.m_size = GetTypeSize(fieldType);
                        sdkStructMember.m_type = GetTypeName(fieldType);
                        sdkStruct->m_sdkStructMembers.push_back(sdkStructMember);

                        // Dependency mangement
                        GetDependenciesForStructMember(fieldType, sdkStruct->m_sdkStructDependencies);
                    }

                    // Sort sdk struct members by offset
                    std::sort(sdkStruct->m_sdkStructMembers.begin(), sdkStruct->m_sdkStructMembers.end());
                }
            }
        }

        // Add struct paddings
        for (std::vector<PSDK_STRUCT>::iterator iter = m_sdkStructs->begin(); iter != m_sdkStructs->end(); iter++)
        {
            PSDK_STRUCT sdkStruct = *iter;

            unsigned int offsetInStruct = 0;
            for (std::vector<SDK_STRUCT::SDK_STRUCT_MEMBER>::iterator iter2 = sdkStruct->m_sdkStructMembers.begin(); iter2 != sdkStruct->m_sdkStructMembers.end(); iter2++)
            {
                unsigned int leftPadding = iter2->m_offset - offsetInStruct;
                if (leftPadding > 0)
                {
                    SDK_STRUCT::SDK_STRUCT_MEMBER sdkStructMember;
                    sdkStructMember.m_offset = offsetInStruct;
                    sdkStructMember.m_size = leftPadding;
                    sdkStructMember.m_isPad = true;
                    iter2 = sdkStruct->m_sdkStructMembers.insert(iter2, sdkStructMember)++;
                }
                offsetInStruct = iter2->m_offset + iter2->m_size;
            }

            unsigned int rightPadding = sdkStruct->m_size - offsetInStruct;
            if (rightPadding > 0)
            {
                SDK_STRUCT::SDK_STRUCT_MEMBER sdkStructMember;
                sdkStructMember.m_offset = offsetInStruct;
                sdkStructMember.m_size = rightPadding;
                sdkStructMember.m_isPad = true;
                sdkStruct->m_sdkStructMembers.push_back(sdkStructMember);
            }
        }

        // Sort dependencies
    was_swap:
        for (std::vector<PSDK_STRUCT>::iterator sdkStructIter = m_sdkStructs->begin(); sdkStructIter != m_sdkStructs->end(); sdkStructIter++)
        {
            PSDK_STRUCT sdkStruct = *sdkStructIter;

            for (std::vector<PSDK_STRUCT>::iterator sdkStructDependencyIter = sdkStruct->m_sdkStructDependencies.begin(); sdkStructDependencyIter != sdkStruct->m_sdkStructDependencies.end(); sdkStructDependencyIter++)
            {
                std::vector<PSDK_STRUCT>::iterator positionOfDependency = std::find(m_sdkStructs->begin(), m_sdkStructs->end(), *sdkStructDependencyIter);
                if (positionOfDependency > sdkStructIter) // is behind struct containing the reference => swap
                {
                    std::iter_swap(sdkStructIter, positionOfDependency);
                    goto was_swap;
                }
            }
        }
    }

    using namespace nlohmann;

    template<typename T>
    json serializeDependencies(std::vector<T>& odeps) {
        json deps;
        for (auto d : odeps)
            deps.push_back(d->m_name);
        return deps;
    }

    template<typename T>
    json serializeMembers(std::vector<T>& omembers) {
        json members;
        for (auto m : omembers) {
            json j;
            j["name"] = m.m_name;
            j["pad"] = m.m_isPad;
            j["offset"] = m.m_offset;
            j["size"] = m.m_size;
            j["type"] = m.m_type;
            members.push_back(j);
        }
        return members;
    }

    json SDK_GENERATOR::Generate() {
        json j;

        json classes;
        for (auto c : *m_sdkClasses) {
            json s;
            s["alignment"] = c->m_alignment;
            s["classId"] = c->m_classId;
            s["name"] = c->m_name;
            if(c->m_parent)
                s["parent"] = c->m_parent->m_name;
            char buf[64]; sprintf_s(buf, "0x%p", c->m_rawDefaultInstance);
            s["defaultInstance"] = std::string(buf);
            s["runtimeId"] = c->m_runtimeId;
            s["size"] = c->m_size;
            s["dependencies"] = serializeDependencies(c->m_sdkClassDependencies);
            s["members"] = serializeMembers(c->m_sdkClassMembers);
            s["module"] = std::string(c->m_rawClassInfo->GetTypeInfoData()->m_Module->m_ModuleName);

            classes.push_back(s);
        }
        j["classes"] = classes;

        json structs;
        for (auto c : *m_sdkStructs) {
            json s;
            s["name"] = c->m_name;
            s["runtimeId"] = c->m_runtimeId;
            s["size"] = c->m_size;
            s["dependencies"] = serializeDependencies(c->m_sdkStructDependencies);
            s["members"] = serializeMembers(c->m_sdkStructMembers);
            s["module"] = std::string(c->m_rawValueInfo->GetTypeInfoData()->m_Module->m_ModuleName);
            structs.push_back(s);
        }
        j["structs"] = structs;

        json enums;
        for (auto c : *m_sdkEnums) {
            json s;
            s["name"] = c->m_name;
            s["runtimeId"] = c->m_runtimeId;
            s["module"] = std::string(c->m_rawEnumInfo->GetTypeInfoData()->m_Module->m_ModuleName);
            json values;
            for (auto e : c->m_sdkEnumFields) {
                json v;
                v["name"] = e.m_name;
                v["value"] = e.m_offset;
                values.push_back(v);
            }
            s["values"] = values;
            enums.push_back(s);
        }
        j["enums"] = enums;

        return j;
    }


    void SDK_GENERATOR::GenerateDeclarations(std::vector<std::string>& result)
    {
        std::string output;
        char buffer[512];

        for (std::vector<PSDK_CLASS>::iterator iter = m_sdkClasses->begin(); iter != m_sdkClasses->end(); iter++)
        {
            PSDK_CLASS sdkClass = *iter;

            // 'class XXX;'
            sprintf_s(buffer, "\nclass %s;", sdkClass->m_name.c_str());
            output += buffer;
        }

        result.push_back(output);
    }


    void SDK_GENERATOR::GenerateClasses(std::vector<std::string>& result)
    {
        // Generate classes
        for (std::vector<PSDK_CLASS>::iterator iter = m_sdkClasses->begin(); iter != m_sdkClasses->end(); iter++)
        {
            PSDK_CLASS sdkClass = *iter;

            std::string output;
            char buffer[512];

            // '////////////////////////////////////////'
            // '// ClassId:   01337'
            // '// RuntimeId: 01337'
            // '// TypeInfo:  0xXXXXXXXXXXXXXXXX
            // #ifndef _XXX_
            // #define _XXX_
            // or =>
            // '////////////////////////////////////////'
            // '// ClassId:   01337'
            // '// RuntimeId: 01337'
            // '// TypeInfo:  0xXXXXXXXX
            // #ifndef _XXX_
            // #define _XXX_
#if defined(_WIN64)
            sprintf_s(buffer, "////////////////////////////////////////\n// ClassId:   %.5i\n// RuntimeId: %.5i\n// TypeInfo:  0x%.16llX\n#ifndef _%s_\n#define _%s_", sdkClass->m_classId, sdkClass->m_runtimeId, sdkClass->m_rawClassInfo, sdkClass->m_name.c_str(), sdkClass->m_name.c_str());
#else
            sprintf_s(buffer, "////////////////////////////////////////\n// ClassId:   %.5i\n// RuntimeId: %.5i\n// TypeInfo:  0x%.8X\n#ifndef _%s_\n#define _%s_", sdkClass->m_classId, sdkClass->m_runtimeId, sdkClass->m_rawClassInfo, sdkClass->m_name.c_str(), sdkClass->m_name.c_str());
#endif
            output += buffer;

            // => if provided
            // '#pragma pack(push, XXX)'
            if (sdkClass->m_alignment > 0)
            {
                sprintf_s(buffer, "\n#pragma pack(push, %i)", sdkClass->m_alignment);
                output += buffer;
            }

            // 'class XXX'
            // => or
            // 'class XXX : public XXX'
            if (sdkClass->m_parent == NULL) sprintf_s(buffer, "\nclass %s", sdkClass->m_name.c_str());
            else sprintf_s(buffer, "\nclass %s : public %s", sdkClass->m_name.c_str(), sdkClass->m_parent->m_name.c_str());
            output += buffer;

            // '{'
            // 'public:'
            output += "\n{\npublic:";

            // static __inline XXX* DefaultInstance() 
            // { 
            //     return (XXX*) 0xXXXXXXXXXXXXXXXX; 
            // }
            if (sdkClass->m_rawDefaultInstance != NULL)
            {
#if defined(_WIN64)
                sprintf_s(buffer, "\n    static __inline %s* DefaultInstance()\n    {\n        return (%s*) 0x%.16llX;\n    }", sdkClass->m_name.c_str(), sdkClass->m_name.c_str(), sdkClass->m_rawDefaultInstance);
#else
                sprintf_s(buffer, "\n    static __inline %s* DefaultInstance()\n    {\n        return (%s*) 0x%.8X;\n    }", sdkClass->m_name.c_str(), sdkClass->m_name.c_str(), sdkClass->m_rawDefaultInstance);
#endif
                output += buffer;
            }

            // static __inline unsigned int ClassId() 
            // { 
            //     return 1337; 
            // }
            sprintf_s(buffer, "\n    static __inline unsigned int ClassId()\n    {\n        return %i;\n    }", sdkClass->m_classId);
            output += buffer;

            for (std::vector<SDK_CLASS::SDK_CLASS_MEMBER>::iterator iter2 = sdkClass->m_sdkClassMembers.begin(); iter2 != sdkClass->m_sdkClassMembers.end(); iter2++)
            {
                SDK_CLASS::PSDK_CLASS_MEMBER sdkClassMember = &(*iter2);

                // '    XXX _XXX;
                // => or
                // '    XXX m_XXX; //0xXXXX
                if (sdkClassMember->m_isPad) sprintf_s(buffer, "\n    char _0x%.4X[%i];", sdkClassMember->m_offset, sdkClassMember->m_size);
                else sprintf_s(buffer, "\n    %s m_%s; //0x%.4X", sdkClassMember->m_type.c_str(), sdkClassMember->m_name.c_str(), sdkClassMember->m_offset);
                output += buffer;
            }

            // '};//Size=0xXXXX'
            sprintf_s(buffer, "\n};//Size=0x%.4X", sdkClass->m_size);
            output += buffer;

            // => if provided
            // '#pragma pack(pop)'
            if (sdkClass->m_alignment > 0) output += "\n#pragma pack(pop)";

            // #endif
            output += "\n#endif";

            result.push_back(output);
        }
    }


    void SDK_GENERATOR::GenerateEnums(std::vector<std::string>& result)
    {
        // Generate enums
        for (std::vector<PSDK_ENUM>::iterator iter = m_sdkEnums->begin(); iter != m_sdkEnums->end(); iter++)
        {
            PSDK_ENUM sdkEnum = *iter;

            std::string output;
            char buffer[512];

            // '////////////////////////////////////////'
            // '// RuntimeId: 01337'
            // '// TypeInfo:  0xXXXXXXXXXXXXXXXX
            // or =>
            // '////////////////////////////////////////'
            // '// RuntimeId: 01337'
            // '// TypeInfo:  0xXXXXXXXX
#if defined(_WIN64)
            sprintf_s(buffer, "////////////////////////////////////////\n// RuntimeId: %.5i\n// TypeInfo:  0x%.16llX", sdkEnum->m_runtimeId, sdkEnum->m_rawEnumInfo);
#else
            sprintf_s(buffer, "////////////////////////////////////////\n// RuntimeId: %.5i\n// TypeInfo:  0x%.8X", sdkEnum->m_runtimeId, sdkEnum->m_rawEnumInfo);
#endif
            output += buffer;

            // 'enum XXX'
            sprintf_s(buffer, "\nenum %s", sdkEnum->m_name.c_str());
            output += buffer;

            // '{'
            output += "\n{";

            for (std::vector<SDK_ENUM::SDK_ENUM_FIELD>::iterator iter2 = sdkEnum->m_sdkEnumFields.begin(); iter2 != sdkEnum->m_sdkEnumFields.end(); iter2++)
            {
                SDK_ENUM::PSDK_ENUM_FIELD sdkEnumField = &(*iter2);

                // '    XXX, //0xXXXX
                // or
                // '    XXX //0xXXXX
                if (std::distance(iter2, sdkEnum->m_sdkEnumFields.end()) != 1)
                    sprintf_s(buffer, "\n    %s, //0x%.4X", sdkEnumField->m_name.c_str(), sdkEnumField->m_offset);
                else
                    sprintf_s(buffer, "\n    %s //0x%.4X", sdkEnumField->m_name.c_str(), sdkEnumField->m_offset);

                output += buffer;
            }

            // '};'
            output += "\n};";

            result.push_back(output);
        }
    }


    void SDK_GENERATOR::GenerateStructs(std::vector<std::string>& result)
    {
        // Generate structs
        for (std::vector<PSDK_STRUCT>::iterator iter = m_sdkStructs->begin(); iter != m_sdkStructs->end(); iter++)
        {
            PSDK_STRUCT sdkStruct = *iter;

            std::string output;
            char buffer[512];

            // '////////////////////////////////////////'
            // '// RuntimeId: 01337'
            // '// TypeInfo:  0xXXXXXXXXXXXXXXXX
            // or =>
            // '////////////////////////////////////////'
            // '// RuntimeId: 01337'
            // '// TypeInfo:  0xXXXXXXXX
#if defined(_WIN64)
            sprintf_s(buffer, "////////////////////////////////////////\n// RuntimeId: %.5i\n// TypeInfo:  0x%.16llX", sdkStruct->m_runtimeId, sdkStruct->m_rawValueInfo);
#else
            sprintf_s(buffer, "////////////////////////////////////////\n// RuntimeId: %.5i\n// TypeInfo:  0x%.8X", sdkStruct->m_runtimeId, sdkStruct->m_rawValueInfo);
#endif
            output += buffer;

            // 'struct XXX'
            sprintf_s(buffer, "\nstruct %s", sdkStruct->m_name.c_str());
            output += buffer;

            // '{'
            output += "\n{";

            for (std::vector<SDK_STRUCT::SDK_STRUCT_MEMBER>::iterator iter2 = sdkStruct->m_sdkStructMembers.begin(); iter2 != sdkStruct->m_sdkStructMembers.end(); iter2++)
            {
                SDK_STRUCT::PSDK_STRUCT_MEMBER sdkStructMember = &(*iter2);

                // '    XXX _XXX;
                // => or
                // '    XXX m_XXX; //0xXXXX
                if (sdkStructMember->m_isPad) sprintf_s(buffer, "\n    char _0x%.4X[%i];", sdkStructMember->m_offset, sdkStructMember->m_size);
                else sprintf_s(buffer, "\n    %s m_%s; //0x%.4X", sdkStructMember->m_type.c_str(), sdkStructMember->m_name.c_str(), sdkStructMember->m_offset);
                output += buffer;
            }

            // '};//Size=0xXXXX'
            sprintf_s(buffer, "\n};//Size=0x%.4X", sdkStruct->m_size);
            output += buffer;

            result.push_back(output);
        }
    }


    std::string SDK_GENERATOR::GetTypeName(fb::TypeInfo* typeInfo)
    {
        switch (typeInfo->GetTypeCode())
        {
        case fb::kTypeCode_ValueType:
            return typeInfo->GetTypeInfoData()->m_Name;
        case fb::kTypeCode_Class:
            return std::string(typeInfo->GetTypeInfoData()->m_Name) + "*";
        case fb::kTypeCode_Array:
        case fb::kTypeCode_FixedArray:
        {
            fb::ArrayTypeInfo* arrayTypeInfo = (fb::ArrayTypeInfo*)typeInfo;
            fb::ArrayTypeInfo::ArrayTypeInfoData* arrayTypeInfoData = arrayTypeInfo->GetArrayTypeInfoData();
            return std::string("Array<") + GetTypeName(arrayTypeInfoData->m_ElementType) + ">";
        }
        case fb::kTypeCode_CString:
            return "CString";
        case fb::kTypeCode_Enum:
            return typeInfo->GetTypeInfoData()->m_Name;
        case fb::kTypeCode_Boolean:
            return "Bool";
        case fb::kTypeCode_Int8:
            return "Int8";
        case fb::kTypeCode_Uint8:
            return "UInt8";
        case fb::kTypeCode_Int16:
            return "Int16";
        case fb::kTypeCode_Uint16:
            return "UInt16";
        case fb::kTypeCode_Int32:
            return "Int32";
        case fb::kTypeCode_Uint32:
            return "UInt32";
        case fb::kTypeCode_Int64:
            return "Int64";
        case fb::kTypeCode_Uint64:
            return "UInt64";
        case fb::kTypeCode_Float32:
            return "Float";
        case fb::kTypeCode_Float64:
            return "Double";
        case fb::kTypeCode_Guid:
            return "Guid";
        case fb::kTypeCode_Void:
        case fb::kTypeCode_DbObject:
        case fb::kTypeCode_String:
        case fb::kTypeCode_FileRef:
        case fb::kTypeCode_SHA1:
        case fb::kTypeCode_ResourceRef:
        default:
            return "// unhandled basic type " + typeInfo->GetTypeName();
        }
    }


    unsigned int SDK_GENERATOR::GetTypeSize(fb::TypeInfo* typeInfo)
    {
        switch (typeInfo->GetTypeCode())
        {
        case fb::kTypeCode_ValueType:
        {
            fb::ValueTypeInfo* valueTypeInfo = (fb::ValueTypeInfo*)typeInfo;
            fb::ValueTypeInfo::ValueTypeInfoData* valueTypeInfoData = valueTypeInfo->GetValueInfoData();
            return valueTypeInfoData->m_TotalSize;
        }
        case fb::kTypeCode_Class:
            // Create pointer to class. For instances the type is kTypeCode_ValueType
            return sizeof(void*);
        case fb::kTypeCode_Array:
        case fb::kTypeCode_FixedArray:
        {
            fb::ArrayTypeInfo* arrayTypeInfo = (fb::ArrayTypeInfo*)typeInfo;
            fb::ArrayTypeInfo::ArrayTypeInfoData* arrayTypeInfoData = arrayTypeInfo->GetArrayTypeInfoData();
            return arrayTypeInfoData->m_TotalSize;
        }
        case fb::kTypeCode_CString:
            return sizeof(char*);
        case fb::kTypeCode_Enum:
            return 4;
        case fb::kTypeCode_Boolean:
            return sizeof(bool);
        case fb::kTypeCode_Int8:
            return sizeof(__int8);
        case fb::kTypeCode_Uint8:
            return sizeof(unsigned __int8);
        case fb::kTypeCode_Int16:
            return sizeof(__int16);
        case fb::kTypeCode_Uint16:
            return sizeof(unsigned __int16);
        case fb::kTypeCode_Int32:
            return sizeof(__int32);
        case fb::kTypeCode_Uint32:
            return sizeof(unsigned __int32);
        case fb::kTypeCode_Int64:
            return sizeof(__int64);
        case fb::kTypeCode_Uint64:
            return sizeof(unsigned __int64);
        case fb::kTypeCode_Float32:
            return sizeof(float);
        case fb::kTypeCode_Float64:
            return sizeof(double);
        case fb::kTypeCode_Void:
        case fb::kTypeCode_DbObject:
        case fb::kTypeCode_String:
        case fb::kTypeCode_FileRef:
        case fb::kTypeCode_Guid:
        case fb::kTypeCode_SHA1:
        default:
            return 0;
        }
    }


    void SDK_GENERATOR::GetDependenciesForStructMember(fb::TypeInfo* typeInfo, std::vector<PSDK_STRUCT>& dependencies)
    {
        // Receive all structs used as value type within a struct. The resulting dependency tree is used for ordering the structs properly
        switch (typeInfo->GetTypeCode())
        {
        case fb::kTypeCode_ValueType:
        {
            fb::ValueTypeInfo* valueTypeInfo = (fb::ValueTypeInfo*)typeInfo;
            fb::ValueTypeInfo::ValueTypeInfoData* valueTypeInfoData = valueTypeInfo->GetValueInfoData();
            std::vector<PSDK_STRUCT>::iterator superSdkStructIterator = std::find_if(m_sdkStructs->begin(), m_sdkStructs->end(), FindByRawValueInfo(valueTypeInfo));
            if (superSdkStructIterator != m_sdkStructs->end()) dependencies.push_back(*superSdkStructIterator);
            break;
        }
        case fb::kTypeCode_Array:
        case fb::kTypeCode_FixedArray:
        {
            fb::ArrayTypeInfo* arrayTypeInfo = (fb::ArrayTypeInfo*)typeInfo;
            fb::ArrayTypeInfo::ArrayTypeInfoData* arrayTypeInfoData = arrayTypeInfo->GetArrayTypeInfoData();
            fb::TypeInfo* elementTypeInfo = arrayTypeInfoData->m_ElementType;
            GetDependenciesForStructMember(elementTypeInfo, dependencies);
            break;
        }
        }
    }
}