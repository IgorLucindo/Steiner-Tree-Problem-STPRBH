# makefile

CXX = g++
DOXYGEN = doxygen

ifeq ($(USER),msinnl)
CXX = /opt/devel/gcc-4.9.0/bin/g++
BOOST_PATHINC=/usr/local/include
BOOST_PATHLIB =/usr/local/lib
BOOST_LDFLAGS=-L$(BOOST_PATHLIB) $(BOOST_PATHLIB)/libboost_program_options.a $(BOOST_PATHLIB)/libboost_filesystem.a $(BOOST_PATHLIB)/libboost_system.a $(BOOST_PATHLIB)/libboost_timer.a $(BOOST_PATHLIB)/libboost_thread.a $(BOOST_PATHLIB)/libboost_chrono.a -Xlinker -rpath -Xlinker $(BOOST_PATHLIB)
OGDFDIR = /opt/OGDF
CPLEXDIRBASE = /opt/ibm/ILOG/CPLEX_Studio126
CPLEXDIR = $(CPLEXDIRBASE)/cplex
CONCERTDIR= $(CPLEXDIRBASE)/concert
SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic
CCLNFLAGS = -L$(CPLEXLIBDIR) -L$(OGDFLIBDIR) -static-libstdc++ -static -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lOGDF -lm -pthread -lrt -lpthread
endif

ifeq ($(USER),markus)
BOOST_PATHINC=/usr/include
BOOST_PATHLIB =/usr/lib
BOOST_LDFLAGS=-L$(BOOST_PATHLIB) -lboost_program_options -lboost_filesystem -lboost_system -lboost_timer -lboost_thread -lboost_chrono -Xlinker -rpath -Xlinker $(BOOST_PATHLIB)
CPLEXDIRBASE = /opt/ibm/ILOG/CPLEX_Studio126
CPLEXDIR = $(CPLEXDIRBASE)/cplex
CONCERTDIR= $(CPLEXDIRBASE)/concert
OGDFDIR = /opt/OGDF
SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic
CCLNFLAGS = -L$(CPLEXLIBDIR) -L$(OGDFLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lOGDF -lm -pthread -lrt -lpthread
endif

CPLEXINCDIR   = $(CPLEXDIR)/include
CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
OGDFLIBDIR	= $(OGDFDIR)/_release

CFLAGS = -I. -I$(BOOST_PATHINC) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) -I$(OGDFDIR)  -pipe -fexceptions -fPIC -ansi -pedantic -Wno-deprecated -Wall -Wextra -Wno-unused-parameter -Werror -Wno-reorder -Wdisabled-optimization -DIL_STD -std=c++11 

TARGETS=STPRBH

all: CFLAGS+= -O3 -DNDEBUG 
all: $(TARGETS)

debug: CFLAGS+= -g3 -O0 
debug: $(TARGETS)

.SUFFIXES: .o .cpp .d
.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@

.cpp.d:
	$(CXX) -MM $(CFLAGS) -MF $@ $<
	
SRCS=$(wildcard *.cpp) $(wildcard */*.cpp)

OBJS=$(SRCS:.cpp=.o)

DEPS=$(SRCS:.cpp=.d)

STPRBH: $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(BOOST_LDFLAGS) $(CCLNFLAGS)

doc:
	$(DOXYGEN) Doxyfile
	
clean:
	rm -f $(OBJS) $(DEPS) $(TARGETS) -r doc
	
docclean:
	rm -r -f doc
	
test:
	chmod u+x ./runTest.sh
	./runTest.sh

sinclude $(DEPS)
