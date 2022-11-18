#pragma once


#include <string>


#include "FrostbiteGeneral.h"


namespace fb
{
    class DataContainer;

    enum BasicTypesEnum
    {
        kTypeCode_Void,
        kTypeCode_DbObject,
        kTypeCode_ValueType,
        kTypeCode_Class,
        kTypeCode_Array,
        kTypeCode_FixedArray,
        kTypeCode_String,
        kTypeCode_CString,
        kTypeCode_Enum,
        kTypeCode_FileRef,
        kTypeCode_Boolean,
        kTypeCode_Int8,
        kTypeCode_Uint8,
        kTypeCode_Int16,
        kTypeCode_Uint16,
        kTypeCode_Int32,
        kTypeCode_Uint32,
        kTypeCode_Int64,
        kTypeCode_Uint64,
        kTypeCode_Float32,
        kTypeCode_Float64,
        kTypeCode_Guid,
        kTypeCode_SHA1,
        kTypeCode_ResourceRef,
        kTypeCode_BasicTypeCount,
        kTypeCode_TypeRef,
        kTypeCode_BoxedValueRef
    };

    class TestList
    {
    public:
        typedef void* Test;

        Test* m_Head; //0x0000 
        Test* m_Tail; //0x0008 

    };//Size=0x0010

#pragma pack(push, 2)
    struct MemberInfoFlags
    {
        unsigned short m_FlagBits; //0x0000 

    };//Size=0x0002
#pragma pack(pop)

    class ModuleInfo
    {
    public:
        char* m_ModuleName; //0x0000 
        ModuleInfo* m_NextModule; //0x0008 
        TestList* m_TestList; //0x0010 

    };//Size=0x0018

#pragma pack(push, 1)
    class MemberInfo
    {
    public:
        struct MemberInfoData
        {
            char* m_Name; //0x0000
            MemberInfoFlags m_Flags; //0x0008

        };//Size=0x000A

        MemberInfoData* m_InfoData; //0x0000

        BasicTypesEnum GetTypeCode();
        std::string GetTypeName();
        MemberInfoData* GetMemberInfoData();

    };//Size=0x0008
#pragma pack(pop)

#pragma pack(push, 1)
    class TypeInfo : public MemberInfo
    {
    public:
        struct TypeInfoData : MemberInfoData
        {
            unsigned short m_TotalSize; //0x000A 
            char _0x000C[4];
            ModuleInfo* m_Module; //0x0010 
            class ArrayTypeInfo* m_pArrayTypeInfo; // 0x018
            unsigned short m_Alignment; //0x20
            unsigned short m_FieldCount; //0x22
            char _0x001A[4];
        };//Size=0x0028
        static_assert(sizeof(TypeInfoData) == 0x28, "bad TypeInfoData");

        TypeInfo* m_Next; //0x0008
        unsigned short m_RuntimeId; //0x0010
        unsigned short m_Flags; //0x0012
#if defined(_WIN64)
        char _0x0014[4];
#endif

        TypeInfoData* GetTypeInfoData();

    };//Size=0x0018
#pragma pack(pop)

    class FieldInfo : public MemberInfo
    {
    public:
        struct FieldInfoData : MemberInfo::MemberInfoData
        {
            unsigned short m_FieldOffset; //0x000A
#if defined(_WIN64)
            char _0x000C[4];
#endif
            TypeInfo* m_FieldTypePtr; //0x0010

        };//Size=0x0018

        virtual TypeInfo* GetDeclaringType();
        virtual unsigned short GetFieldIndex();

        TypeInfo* m_DeclaringType; //0x0010 
        unsigned short m_FieldIndex; //0x0018 
        unsigned short m_AttributeMask; //0x001A 
#if defined(_WIN64)
        char _0x001C[4];
#endif

        FieldInfoData* GetFieldInfoData();

    };//Size=0x0020

#pragma pack(push, 1)
    class ClassInfo : public TypeInfo
    {
    public:
        struct ClassInfoData : TypeInfo::TypeInfoData
        {
            ClassInfo* m_SuperClass; //0x0028
            //void* m_CreateFun; //0x0030
            FieldInfo::FieldInfoData* m_Fields; //0x0030
        };//Size=0x0038
        static_assert(sizeof(ClassInfoData) == 0x38, "bad ClassInfoData");

        ClassInfo* m_Super; //0x0018 
        DataContainer* m_DefaultInstance; //0x0020 
        unsigned short m_ClassId; //0x0028 
        unsigned short m_LastSubClassId; //0x002A 
#if defined(_WIN64)
        char _0x002C[4];
#endif
        ClassInfo* m_FirstDerivedClass; //0x0030 
        ClassInfo* m_NextSiblingClass; //0x0038 
        FieldInfo** m_FieldInfos; //0x0040 
        unsigned int m_TotalFieldCount; //0x0048 
#if defined(_WIN64)
        char _0x004C[4];
#endif

        ClassInfoData* GetClassInfoData();

    };//Size=0x0050
#pragma pack(pop)

    class ArrayTypeInfo : public TypeInfo
    {
    public:
        struct ArrayTypeInfoData : TypeInfo::TypeInfoData
        {
            TypeInfo* m_ElementType; //0x0020

        };//Size=0x0028

        ArrayTypeInfoData* GetArrayTypeInfoData();

    };//Size=0x0018

    class EnumFieldInfo : public TypeInfo
    {
    public:
        struct EnumFieldInfoData : TypeInfo::TypeInfoData
        {
            FieldInfo::FieldInfoData* m_Fields; //0x0020 	

        };//Size=0x0028

        FieldInfo** m_FieldInfos; //0x0018

        EnumFieldInfoData* GetEnumInfoData();

    };//Size=0x0020

    class ValueTypeInfo : public TypeInfo
    {
    public:
        struct ValueTypeInfoData : TypeInfo::TypeInfoData
        {

            char off[40];
            FieldInfo::FieldInfoData* m_Fields; //0x0020 	

        };//Size=0x0028

        FieldInfo** m_FieldInfos; //0x0018

        ValueTypeInfoData* GetValueInfoData();

    };//Size=0x0020

    class ITypedObject
    {
    public:
        virtual TypeInfo* GetType();

    };//Size=0x0008
}