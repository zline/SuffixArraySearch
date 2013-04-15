#ifndef TEXT_SEARCH_HPP_2009_06_13
#define TEXT_SEARCH_HPP_2009_06_13

#include "common.hpp"

#include <boost/smart_ptr.hpp>
#include <boost/static_assert.hpp>

#include <string>
#include <vector>
#include <iterator>
#include <algorithm>

#include <cassert>
#include <cstdlib>

using boost::shared_array;


namespace ESP { namespace TextIndex {


// реализация чтения файла с загрузкой в память
class TextReaderHeap
{
private:
    shared_array<char> m_data;
    
    size_t m_fileSize;
    
public:
    TextReaderHeap(std::string const & fileName)
    {
        m_fileSize = fileToHeap<char>(fileName, m_data, MaxTextFileSize);
    }
    
    size_t fileSize() const { return m_fileSize; };
    
    char const * getData(size_t offset, 
#ifndef NDEBUG
        size_t length
#else
        size_t 
#endif
        ) const
    {
        assert(offset + length <= m_fileSize);
    
        return m_data.get() + offset;
    }
    
#ifdef NDEBUG
    void freeData(char const *, size_t) const {};
#else
    void freeData(char const *ptr, size_t length) const
    {
        assert(ptr >= m_data.get());
        assert((ptr - m_data.get()) + length <= m_fileSize);
    }
#endif
};

// TODO реализация чтения файла с использованием mmap'a


// реализация чтения индекса с загрузкой в память
class IdxReaderHeap
{
private:
    shared_array<uint32_t> m_data;
    
    size_t m_idxItems;
    
public:
    IdxReaderHeap(std::string const & fileName)
    {
        m_idxItems = fileToHeap<uint32_t>(fileName, m_data, MaxTextFileSize);
    }
    
    uint32_t const * getIdx() const { return m_data.get(); };
    
    size_t getElementsCount() const { return m_idxItems; };
};

// TODO реализация чтения индекса с использованием mmap'a


template<class TextReader = TextReaderHeap, class StrComparer = IdxStrComparerSimple>
struct IdxSearchComparer : public std::binary_function<uint32_t, uint32_t, bool>
{
public:
    enum
    {
        NeedleStringMark = uint32_t(-1)
    };

private:
    TextReader m_text;
    
    // искомая строка
    std::string m_needle;
    
    // объект, выполняющий сравнение строк
    StrComparer m_strcmp;

    // при поиске в индексе используем хак: "позицию" искомой строки в индексе обозначим
    // как uint32_t(-1), поэтому чтобы это работало..
    BOOST_STATIC_ASSERT(NeedleStringMark > uint32_t(MaxTextFileSize));

public:
    IdxSearchComparer(std::string const & needle, TextReader const & textrd, StrComparer comparer): 
            m_text(textrd), 
            m_needle(needle),
            m_strcmp(comparer)
    {
        if (m_needle.empty())
            throw std::runtime_error("empty string");
        if (m_needle.size() > MaxCharsToCompareLimit)
            throw std::runtime_error("string is too long");
    }

    bool operator()(uint32_t strPosX, uint32_t strPosY) const
    {
        assert(strPosX == NeedleStringMark || size_t(strPosX) < m_text.fileSize());
        assert(strPosY == NeedleStringMark || size_t(strPosY) < m_text.fileSize());
    
        assert(! (strPosX == NeedleStringMark && strPosY == NeedleStringMark));
        if (strPosX == strPosY)
            return false;
        
        char const *pX, *pY;
        size_t lenX, lenY;
        
        if (strPosX == NeedleStringMark)
        {
            pX = m_needle.c_str();
            lenX = m_needle.size();
        }
        else
        {
            // сравниваем не более m_needle.size() символов
            lenX = std::min(m_text.fileSize() - strPosX, m_needle.size());
            pX = m_text.getData(strPosX, lenX);
        }
        
        if (strPosY == NeedleStringMark)
        {
            pY = m_needle.c_str();
            lenY = m_needle.size();
        }
        else
        {
            lenY = std::min(m_text.fileSize() - strPosY, m_needle.size());
            pY = m_text.getData(strPosY, lenY);
        }

        bool result = m_strcmp.compare(pX, lenX, pY, lenY);
        
        if (strPosX != NeedleStringMark)
            m_text.freeData(pX, lenX);
        if (strPosY != NeedleStringMark)
            m_text.freeData(pY, lenY);
        
        return result;
    }
};


template<class TextReader = TextReaderHeap, class IdxReader = IdxReaderHeap, class StrComparer = IdxStrComparerSimple>
class IdxSearcher
{
public:
    typedef IdxSearchComparer<TextReader, StrComparer> SearchCmp;

private:
    TextReader m_text;
    IdxReader m_idx;
    StrComparer m_strcmp;

public:
    IdxSearcher(TextReader const & textrd, IdxReader const & idxrd, StrComparer comparer = StrComparer()):
            m_text(textrd),
            m_idx(idxrd),
            m_strcmp(comparer) {};
    
    void search(std::string const & needle, std::vector<uint32_t> & result)
    {
        uint32_t const * idxBegin = m_idx.getIdx();
        uint32_t const * idxEnd = idxBegin + m_idx.getElementsCount();
        
        SearchCmp cmp(needle, m_text, m_strcmp);
        
        uint32_t const * lower = std::lower_bound(idxBegin, idxEnd, uint32_t(SearchCmp::NeedleStringMark), cmp);
        uint32_t const * upper = std::upper_bound(idxBegin, idxEnd, uint32_t(SearchCmp::NeedleStringMark), cmp);
        
        assert(lower <= upper);
        
        result.clear();
        if (lower == upper)
            return;
        
        std::copy(lower, upper, std::back_inserter(result));
    }
};


}}


#endif
