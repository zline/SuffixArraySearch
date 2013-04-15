#ifndef TEXT_IDX_HPP_2009_06_13
#define TEXT_IDX_HPP_2009_06_13

#include "common.hpp"

#include <functional>
#include <algorithm>
#include <stdexcept>

#include <cassert>


namespace ESP { namespace TextIndex {


template<class StrComparer = IdxStrComparerSimple> struct IdxSorter : public std::binary_function<uint32_t, uint32_t, bool>
{
private:
    // индексируемый текст
    char const * m_text;
    // длина
    size_t m_length;
    
    // максимальное число сравниваемых символов
    size_t m_maxCharsToCompare;
    
    // объект, выполняющий сравнение строк
    StrComparer m_strcmp;

public:
    IdxSorter(char const *text, size_t length, StrComparer comparer = StrComparer(), size_t maxCharsToCompare = MaxCharsToCompareLimit): 
            m_text(text), 
            m_length(length),
            m_maxCharsToCompare(maxCharsToCompare),
            m_strcmp(comparer)
    {
        if (m_maxCharsToCompare == 0 || m_maxCharsToCompare > MaxCharsToCompareLimit)
            throw std::runtime_error("bad maxCharsToCompare value");
    }

    bool operator()(uint32_t strPosX, uint32_t strPosY) const
    {
        assert(size_t(strPosX) < m_length);
        assert(size_t(strPosY) < m_length);
    
        if (strPosX == strPosY)
            return false;
        
        size_t lengthX = std::min(size_t(MaxCharsToCompareLimit), m_length - size_t(strPosX));
        size_t lengthY = std::min(size_t(MaxCharsToCompareLimit), m_length - size_t(strPosY));
        
        return m_strcmp.compare(m_text + strPosX, lengthX, m_text + strPosY, lengthY);
    }
};


}}


#endif
