#include "text_search.hpp"

#include <boost/foreach.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cassert>

#include <sys/time.h>

using namespace ESP::TextIndex;


inline uint64_t microseconds(struct timeval const & tv)
{
    return uint64_t( uint64_t(tv.tv_sec) * uint64_t(1000000) + uint64_t(tv.tv_usec) );
}


int main(int argc, char **argv)
{
    try
    {
        if (argc < 2)
            throw std::runtime_error("Usage: text_search file");
        
        std::string file(argv[1]);
        TextReaderHeap textrd(file);
        IdxReaderHeap idxrd(file + ".idx");
        if (textrd.fileSize() != idxrd.getElementsCount())
            throw std::runtime_error("index size mismatch");
        
        // h4ck
        size_t filesize = textrd.fileSize();
        char const * data = textrd.getData(0, filesize);
        
        IdxSearcher<> searcher(textrd, idxrd);
        std::vector<uint32_t> result;
        std::string needle;
        
        while (std::getline(std::cin, needle))
        {
            if (needle.empty() || needle.find('\n') != std::string::npos)
                throw std::runtime_error("bad parameter");
            
            struct timeval tvstart, tvend;
            
            if (0 != gettimeofday(&tvstart, 0))
                throw std::runtime_error("gettimeofday failed");
            searcher.search(needle, result);
            if (0 != gettimeofday(&tvend, 0))
                throw std::runtime_error("gettimeofday failed");
            std::cout << "Search time: " << (microseconds(tvend) - microseconds(tvstart)) << " microseconds" << std::endl;
            
            std::cout << "Results (" << result.size() << "):" << std::endl;
            BOOST_FOREACH(uint32_t pos, result)
            {
                uint32_t start = pos;
                while (start > 0 && data[start] != '\n')
                    start--;
                if (data[start] == '\n')
                    start++;
            
                uint32_t end = pos;
                while (end < filesize && data[end] != '\n')
                    end++;
                if (data[end] == '\n')
                    end--;
                
                assert(start < end);
                std::cout << "offset " << pos << ":\t";
                std::cout.write(data + start, end - start + 1);
                std::cout << std::endl;
            }
        }
        
        textrd.freeData(data, filesize);
    }
    catch (std::exception const & ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
    
    return 0;
}
