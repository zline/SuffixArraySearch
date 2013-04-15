CXX=g++
CXXFLAGS=-Wall -Wextra

LD=g++
LIBS=-lstdc++

ifndef _DEBUG
    CXXFLAGS += -DNDEBUG -O2
else
    CXXFLAGS += -D_DEBUG -ggdb
endif


all: text_idx text_search


text_idx: text_idx.o text_idx.hpp common.o
	$(LD) $(LIBS) text_idx.o common.o -o text_idx


text_search: text_search.o text_search.hpp common.o
	$(LD) $(LIBS) text_search.o common.o -o text_search

clean:
	rm -rf text_idx text_search *.o


common.cpp: common.hpp
	touch $@


%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<
