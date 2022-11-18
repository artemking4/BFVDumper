#pragma once


#include <Windows.h>


#include "Eastl.h"


namespace fb
{

    template<typename T> class WeakToken
    {
    public:
        T* m_RealPtr; //0x0000
    };//Size=0x0008

    template<typename T> class ConstWeakPtr
    {
    public:
        WeakToken<T>* m_Token; //0x0000

        T* Get()
        {
            if (m_Token && m_Token->m_RealPtr)
            {
                DWORD_PTR realPtr = (DWORD_PTR)m_Token->m_RealPtr;
                return (T*)(realPtr - sizeof(DWORD_PTR));
            }
            return NULL;
        }
    };//Size=0x0008

    template<typename T> class WeakPtr : public ConstWeakPtr<T>
    {
    };//Size=0x0008

    template<typename T> class Array : public eastl::BasicArray<T>
    {
    };//Size=0x0008

    struct Guid
    {
        unsigned long m_Data1; //0x0000
        unsigned short m_Data2; //0x0004
        unsigned short m_Data3; //0x0006
        unsigned char m_Data4[8]; //0x0008
    };//Size=0x0010

    struct Color32
    {
        union
        {
            struct
            {
                unsigned char m_R; //0x0000
                unsigned char m_G; //0x0001
                unsigned char m_B; //0x0002
                unsigned char m_A; //0x0003
            };

            unsigned int m_Data; //0x0000
        };

        Color32(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1);
        Color32(unsigned int col);
    };//Size=0x0004

    template<class T> class Tuple2
    {
    public:
        T m_Element1;
        T m_Element2;

        Tuple2(T element1, T element2)
        {
            m_Element1 = element1;
            m_Element2 = element2;
        }
    };//Size=2*T
}