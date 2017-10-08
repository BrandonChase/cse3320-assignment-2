CXX= g++ -std=c++11
CXXFLAGS= -Wall -g
LDFLAGS= -lpthread
INCLUDE= 

SRCDIR= ./src
BINDIR= ./bin
TARGET= $(BINDIR)/part2

SOURCE= $(wildcard $(SRCDIR)/*.cpp)

all: 
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SOURCE) $(LDFLAGS) -o $(TARGET)

clean:
	rm $(TARGET)