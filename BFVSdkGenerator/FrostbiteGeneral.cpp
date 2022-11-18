#include "FrostbiteGeneral.h"


namespace fb
{
    Color32::Color32(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1)
    {
        m_R = r1;
        m_G = g1;
        m_B = b1;
        m_A = a1;
    }


    Color32::Color32(unsigned int col)
    {
        m_Data = col;
    }
}