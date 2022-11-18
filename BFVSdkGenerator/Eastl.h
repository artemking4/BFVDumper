#include <cstdint>
#include <windows.h>

#ifndef __EASTL_H__
#define __EASTL_H__

/*
#include "EASTL/algorithm.h"
#include "EASTL/allocator.h"
#include "EASTL/bitset.h"
//#include "EASTL/core_allocator_adapter.h"
#include "EASTL/deque.h"
//#include "EASTL/fixed_allocator.h"
#include "EASTL/fixed_hash_map.h"
#include "EASTL/fixed_hash_set.h"
#include "EASTL/fixed_list.h"
#include "EASTL/fixed_map.h"
#include "EASTL/fixed_set.h"
#include "EASTL/fixed_string.h"
#include "EASTL/fixed_substring.h"
#include "EASTL/fixed_vector.h"
#include "EASTL/functional.h"
#include "EASTL/hash_map.h"
#include "EASTL/hash_set.h"
#include "EASTL/heap.h"
#include "EASTL/iterator.h"
#include "EASTL/list.h"
#include "EASTL/map.h"
#include "EASTL/memory.h"
#include "EASTL/set.h"
#include "EASTL/sort.h"
#include "EASTL/string.h"
#include "EASTL/type_traits.h"
#include "EASTL/utility.h"
#include "EASTL/vector.h"
*/

namespace eastl {
    template <typename T>
    class WeakPtr
    {
    private:
        T** m_ptr;

    public:
        T* GetData()
        {
            if (m_ptr == NULL)
                return NULL;

            if (*m_ptr == NULL)
                return NULL;

            return (T*)((DWORD)(*m_ptr) - offsetof(T, m_weakTokenHolder));
        }
    };

    template <typename T>
    class Array
    {
    private:
        T* m_firstElement;

    public:
        T At(INT nIndex)
        {
            if (m_firstElement == NULL)
                return NULL;

            return *(T*)((DWORD)m_firstElement + (nIndex * sizeof(T)));
        };

        T operator [](INT index) { return At(index); }
    };

    template <typename T>
    class RefArray
    {
    private:
        T** m_array;

    public:
        T* At(INT nIndex)
        {
            if (m_array == NULL)
                return NULL;

            return *(T**)((DWORD)m_array + (nIndex * 4));
        }

        T* operator [](INT index) { return At(index); }
    };

    template <typename T>
    class RelocArray
    {
    private:
        UINT m_count;
        T* m_data;

    public:
        UINT Size() { return m_count; }
        T At(INT index) { return *(T*)((DWORD)m_data + (index * sizeof(T))); }
        T operator[](INT index) { return At(index); }
    };

    template< typename T >
    struct SmartRef
    {
        T* m_ptr;
    }; //sizeof() == 0x8

    template< class T >
    struct SmartPtr
    {
        T* m_ptr;
    }; //sizeof() == 0x8

    template<typename T> class BasicArray
    {
    public:
        T* mpBegin;
    };
}

#endif