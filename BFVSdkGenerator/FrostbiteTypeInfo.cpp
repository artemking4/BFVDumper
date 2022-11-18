#include "FrostbiteTypeInfo.h"


namespace fb
{

    BasicTypesEnum MemberInfo::GetTypeCode()
    {
        fb::MemberInfo::MemberInfoData* memberInfoData = GetMemberInfoData();
        if (memberInfoData)
        {
            BasicTypesEnum Type = kTypeCode_Void;
            switch (memberInfoData->m_Flags.m_FlagBits & 0x3E0) {
            case 0x0000:
                Type = kTypeCode_Void;
                break;
            case 0x0020:
                Type = kTypeCode_DbObject;
                break;
            case 0x0040:
                Type = kTypeCode_ValueType;
                break;
            case 0x0060:
                Type = kTypeCode_Class;
                break;
            case 0x0080:
                Type = kTypeCode_Array;
                break;
            case 0x00A0:
                Type = kTypeCode_FixedArray;
                break;
            case 0x00C0:
                Type = kTypeCode_String;
                break;
            case 0x00E0:
                Type = kTypeCode_CString;
                break;
            case 0x0100:
                Type = kTypeCode_Enum;
                break;
            case 0x0120:
                Type = kTypeCode_FileRef;
                break;
            case 0x0140:
                Type = kTypeCode_Boolean;
                break;
            case 0x0160:
                Type = kTypeCode_Int8;
                break;
            case 0x0180:
                Type = kTypeCode_Uint8;
                break;
            case 0x01A0:
                Type = kTypeCode_Int16;
                break;
            case 0x01C0:
                Type = kTypeCode_Uint16;
                break;
            case 0x01E0:
                Type = kTypeCode_Int32;
                break;
            case 0x0200:
                Type = kTypeCode_Uint32;
                break;
            case 0x0220:
                Type = kTypeCode_Int64;
                break;
            case 0x0240:
                Type = kTypeCode_Uint64;
                break;
            case 0x0260:
                Type = kTypeCode_Float32;
                break;
            case 0x0280:
                Type = kTypeCode_Float64;
                break;
            case 0x02A0:
                Type = kTypeCode_Guid;
                break;
            case 0x02C0:
                Type = kTypeCode_SHA1;
                break;
            case 0x02E0:
                Type = kTypeCode_ResourceRef;
                break;
            case 0x0300:
                Type = kTypeCode_BasicTypeCount;
                break;
            case 0x0320:
                Type = kTypeCode_TypeRef;
                break;
            case 0x0340:
                Type = kTypeCode_BoxedValueRef;
                break;
            }
            return Type;
        }
        return kTypeCode_BasicTypeCount;
    }


    std::string MemberInfo::GetTypeName()
    {
        switch (GetTypeCode())
        {
        case kTypeCode_Void: return "Void";
        case kTypeCode_DbObject: return "DbObject";
        case kTypeCode_ValueType: return "ValueType";
        case kTypeCode_Class: return "Class";
        case kTypeCode_Array: return "Array";
        case kTypeCode_FixedArray: return "FixedArray";
        case kTypeCode_String: return "String";
        case kTypeCode_CString: return "CString";
        case kTypeCode_Enum: return "Enum";
        case kTypeCode_FileRef: return "FileRef";
        case kTypeCode_Boolean: return "Boolean";
        case kTypeCode_Int8: return "Int8";
        case kTypeCode_Uint8: return "Uint8";
        case kTypeCode_Int16: return "Int16";
        case kTypeCode_Uint16: return "Uint16";
        case kTypeCode_Int32: return "Int32";
        case kTypeCode_Uint32: return "Uint32";
        case kTypeCode_Int64: return "Int64";
        case kTypeCode_Uint64: return "Uint64";
        case kTypeCode_Float32: return "Float32";
        case kTypeCode_Float64: return "Float64";
        case kTypeCode_Guid: return "Guid";
        case kTypeCode_SHA1: return "SHA1";
        case kTypeCode_ResourceRef: return "ResourceRef";
        default:
            char buffer[32];
            sprintf_s(buffer, "Undefined[%i]", GetTypeCode());
            return buffer;
        }
    }


    MemberInfo::MemberInfoData* MemberInfo::GetMemberInfoData()
    {
        return ((MemberInfoData*)m_InfoData);
    }


    TypeInfo::TypeInfoData* TypeInfo::GetTypeInfoData()
    {
        return ((TypeInfoData*)m_InfoData);
    }


    FieldInfo::FieldInfoData* FieldInfo::GetFieldInfoData()
    {
        return ((FieldInfoData*)m_InfoData);
    }


    ClassInfo::ClassInfoData* ClassInfo::GetClassInfoData()
    {
        return ((ClassInfoData*)m_InfoData);
    }


    ArrayTypeInfo::ArrayTypeInfoData* ArrayTypeInfo::GetArrayTypeInfoData()
    {
        return ((ArrayTypeInfoData*)m_InfoData);
    }


    EnumFieldInfo::EnumFieldInfoData* EnumFieldInfo::GetEnumInfoData()
    {
        return ((EnumFieldInfoData*)m_InfoData);
    }


    ValueTypeInfo::ValueTypeInfoData* ValueTypeInfo::GetValueInfoData()
    {
        return ((ValueTypeInfoData*)m_InfoData);
    }
}