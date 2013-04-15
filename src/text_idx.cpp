#include "text_idx.hpp"

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>

#include <cstdio>

using boost::scoped_array;
using boost::lexical_cast;

using namespace ESP::TextIndex;


int main(int argc, char **argv)
{
    try
    {
        if (argc < 2)
            throw std::runtime_error("Usage: text_idx file");
        
        std::cerr << "reading in file.." << std::endl;
        shared_array<char> memFile;
        size_t fileSize = fileToHeap<char>(argv[1], memFile, MaxTextFileSize);
        
        std::cerr << "preparing index.." << std::endl;
        scoped_array<uint32_t> idxSA( new uint32_t[fileSize] );
        uint32_t *idx = idxSA.get();
        // изначально отсортированный по возрастанию массив позиций
        for (uint32_t pos = 0; pos < uint32_t(fileSize); pos++)
            idx[pos] = pos;
    
        std::cerr << "indexing.." << std::endl;
        IdxSorter<> sorter(memFile.get(), fileSize);
        std::sort(idx, idx + fileSize, sorter);
        
        /* DEBUG
        for (uint32_t pos = 0; pos < uint32_t(fileSize); pos++)
        {
            std::cout.write(memFile.get() + idx[pos], fileSize - idx[pos]);
            std::cout << std::endl;
        }*/
    
        std::cerr << "writing index.." << std::endl;
        FILE * fh = fopen((std::string(argv[1]) + ".idx").c_str(), "wb");
        if (fh == NULL)
            throw std::runtime_error("failed to write index file");
        size_t uints = fwrite(idx, sizeof(uint32_t), fileSize, fh);
        fclose(fh);
        if (uints != fileSize)
            throw std::runtime_error("write error: written "+boost::lexical_cast<std::string>(uints*sizeof(uint32_t))+" bytes, but must be "+boost::lexical_cast<std::string>(fileSize*sizeof(uint32_t)));
        std::cerr << "complete" << std::endl;
    }
    catch (std::exception const & ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
        return 1;
    }
    
    return 0;
}
