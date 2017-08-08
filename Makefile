
INCLUDES     = -I. 

DESTNAME = orderprocessor 

CPPFILES = orderprocessor.cpp \
	   orderbook.cpp \
	   orderbookmanager.cpp \

OFILES   = $(CPPFILES:.cpp=.o)
CXXFLAGS   = -static -g -Wall -Werror -std=c++11 $(INCLUDES)

all : 	$(DESTNAME)

$(DESTNAME) : $(OFILES)
	g++ $(OFILES) -g -o $(DESTNAME) 


clean:
	@(rm -f $(OFILES) $(DESTNAME))

