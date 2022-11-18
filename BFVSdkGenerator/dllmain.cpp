#include <Windows.h>
#include <thread>
#include "SdkGenerator.h"
#include "json.hpp"
#include <fstream>
#include <optional>

using namespace BackendBf4;
using namespace fb;
using namespace nlohmann;

std::string hexifyAddress(void* address) {
    char buf[64];
    sprintf_s(buf, "0x%p", address);
    return std::string(buf);
}

int lookupType(std::vector<fb::TypeInfo*> typeinfos, fb::TypeInfo* lt) {
    for (int i = 0; i < typeinfos.size(); i++)
        if (typeinfos[i] == lt) return i + 1;

    throw std::runtime_error("Could not find a type's internal id");
};

// EW EW EW EW
json SerializeEnumFieldInfoData(std::vector<fb::TypeInfo*>& typeinfos, EnumFieldInfo::EnumFieldInfoData::EnumFieldInfoDataField* t) {
    json j;

    j["name"] = std::string(t->m_Name);
    j["flags"] = t->m_Flags.m_FlagBits;
    j["offset"] = t->m_FieldOffset;

    return j;
}

json SerializeFieldInfoData(std::vector<fb::TypeInfo*>& typeinfos, FieldInfo::FieldInfoData* t) {
    json j;

    j["name"] = std::string(t->m_Name);
    j["flags"] = t->m_Flags.m_FlagBits;
    j["offset"] = t->m_FieldOffset;
    if(t->m_FieldTypePtr) j["type"] = lookupType(typeinfos, t->m_FieldTypePtr);

    return j;
}

json SerializeTypeInfo(std::vector<fb::TypeInfo*>& typeinfos, fb::TypeInfo* current) {
    json typeinfo;

    typeinfo["typeinfo"] = hexifyAddress(current);
    typeinfo["flags"] = current->m_Flags;
    typeinfo["runtimeId"] = current->m_RuntimeId;
    if (current->m_Next)
        typeinfo["next"] = lookupType(typeinfos, current->m_Next);

    json infoData;
    infoData["flags"] = current->m_InfoData->m_Flags.m_FlagBits;
    infoData["name"] = current->m_InfoData->m_Name;
    infoData["type"] = current->GetTypeCode();
    infoData["typename"] = current->GetTypeName();
    switch (current->GetTypeCode()) {
    case kTypeCode_Class: {
        auto ci = (ClassInfo*)current;
        typeinfo["classId"] = ci->m_ClassId;
        if (ci->m_DefaultInstance) typeinfo["defaultInstance"] = hexifyAddress(ci->m_DefaultInstance);
        //if (ci->m_FirstDerivedClass) typeinfo["firstDerivedClass"] = lookupType(typeinfos, ci->m_FirstDerivedClass);
        typeinfo["lastSubClassId"] = ci->m_LastSubClassId;
        if (ci->m_NextSiblingClass) typeinfo["nextSiblingClass"] = lookupType(typeinfos, ci->m_NextSiblingClass);
        if (ci->m_Super) typeinfo["superClass"] = lookupType(typeinfos, ci->m_Super);
        typeinfo["totalFieldCount"] = ci->m_TotalFieldCount;

        auto cid = ci->GetClassInfoData();
        infoData["alignment"] = cid->m_Alignment;
        infoData["fieldCount"] = cid->m_FieldCount;
        infoData["module"] = std::string(cid->m_Module->m_ModuleName);
        if (cid->m_pArrayTypeInfo) infoData["arrayTypeInfo"] = lookupType(typeinfos, cid->m_pArrayTypeInfo);
        infoData["totalSize"] = cid->m_TotalSize;
        json fields;
        for (int i = 0; i < cid->m_FieldCount; i++)
            fields.push_back(SerializeFieldInfoData(typeinfos, &cid->m_Fields[i]));
        infoData["fields"] = fields;
        break;
    }
    case kTypeCode_ValueType: {
        auto ci = (ValueTypeInfo*)current;
        auto cid = ci->GetValueInfoData();
        infoData["alignment"] = cid->m_Alignment;
        infoData["fieldCount"] = cid->m_FieldCount;
        infoData["module"] = std::string(cid->m_Module->m_ModuleName);
        if (cid->m_pArrayTypeInfo) infoData["arrayTypeInfo"] = lookupType(typeinfos, cid->m_pArrayTypeInfo);
        infoData["totalSize"] = cid->m_TotalSize;
        break;
    }
    case kTypeCode_FixedArray:
    case kTypeCode_Array: {
        auto i = (ArrayTypeInfo*)current;
        if(i->GetArrayTypeInfoData()->m_ElementType)
            infoData["arrayTypeInfo"] = lookupType(typeinfos, i->GetArrayTypeInfoData()->m_ElementType);
        infoData["module"] = std::string(i->GetArrayTypeInfoData()->m_Module->m_ModuleName);
        infoData["totalSize"] = i->GetArrayTypeInfoData()->m_TotalSize;
        break;
    }
    case kTypeCode_Enum: {
        auto ci = (EnumFieldInfo*)current;
        auto cid = ci->GetEnumInfoData();
        infoData["alignment"] = cid->m_Alignment;
        infoData["fieldCount"] = cid->m_FieldCount;
        infoData["module"] = std::string(cid->m_Module->m_ModuleName);
        if (cid->m_pArrayTypeInfo) infoData["arrayTypeInfo"] = lookupType(typeinfos, cid->m_pArrayTypeInfo);
        infoData["totalSize"] = cid->m_TotalSize;
        json fields;
        for (int i = 0; i < cid->m_FieldCount; i++)
            fields.push_back(SerializeEnumFieldInfoData(typeinfos, &cid->m_Fields[i]));
        infoData["fields"] = fields;
        break;
    }
    }

    typeinfo["info"] = infoData;
    return typeinfo;
}

void Thread(HMODULE mod) {
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);

    printf("Dumping...\n");

    fb::TypeInfo* g_firstTypeInfo = (fb::TypeInfo*)0x144F7E670; // id rather make that a sig but im too lazy

    std::vector<fb::TypeInfo*> typeinfos;

    json results;
    fb::TypeInfo* current = g_firstTypeInfo;
    do {
        typeinfos.push_back(current);
    } while (current = current->m_Next);

    int id = 0;
    for(auto current : typeinfos) {
        json d = SerializeTypeInfo(typeinfos, current);
        d["index"] = id;
        results.push_back(d);
        id++;
    }

    auto str = results.dump(4);
    std::ofstream f("dump.json");
    f << str;
    f.close();
    printf("Done\n");
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        std::thread(Thread, hModule).detach();

    return TRUE;
}

