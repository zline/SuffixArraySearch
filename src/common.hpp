#ifndef COMMON_HPP_2009_06_13
#define COMMON_HPP_2009_06_13

#include <boost/smart_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <functional>
#include <algorithm>
#include <stdexcept>
#include <string>

#include <cassert>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using boost::shared_array;


namespace ESP { namespace TextIndex {


enum
{
    MaxTextFileSize = 125*1024*1024,
    
    // лимит, установленный на максимальные длины сравниваемых строк
    // из соображений производительности
    MaxCharsToCompareLimit = 255
};


struct IdxStrComparerSimple
{
    bool compare(char const *pX, size_t lenX, char const *pY, size_t lenY) const
    {
        for (size_t i = 0, min_len = std::min(lenX, lenY); i < min_len; i++)
        {
            if (pX[i] < pY[i])
                return true;
            if (pX[i] > pY[i])
                return false;
        }
        // общая часть строк одинакова
        if (lenX < lenY)
            return true;
        
        // X ! precedes Y
        return false;
    }
};


// возвращает число прочитанных Element'ов
template<typename Element> size_t fileToHeap(std::string const & fileName, shared_array<Element> & array, size_t limit = 0 /* 0 - disabled */)
{
    struct stat fileStat;
    
    if (0 != stat(fileName.c_str(), &fileStat))
        throw std::runtime_error("failed to stat file: " + fileName);
    if (! S_ISREG(fileStat.st_mode))
        throw std::runtime_error("file '"+fileName+"' is not regular file");
    
    size_t fileSize = size_t(fileStat.st_size);
    if (0 == fileSize)
        throw std::runtime_error("file '"+fileName+"' is empty");
    if (0 != (fileSize % sizeof(Element)))
        throw std::runtime_error("file '"+fileName+"' size is not rounded to sizeof(Element)");
    // теперь размер - в элементах..
    fileSize /= sizeof(Element);
    if (limit && limit < fileSize)
        throw std::runtime_error("file '"+fileName+"' is too big");
    
    FILE * fh = fopen(fileName.c_str(), "rb");
    if (fh == NULL)
        throw std::runtime_error("failed to open file: " + fileName);
    
    array = shared_array<Element>( new Element[fileSize] );
    size_t elemsRead = fread(array.get(), sizeof(Element), fileSize, fh);
    fclose(fh);
    
    if (elemsRead != fileSize)
        throw std::runtime_error("read '"+fileName+"' error: read "+boost::lexical_cast<std::string>(elemsRead)+
        " elements, but must be "+boost::lexical_cast<std::string>(fileSize));
        
    return fileSize;
}


}}


#endif
