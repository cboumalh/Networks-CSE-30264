# Christopher Boumalhab - cboumalh - 902138925
# CSE-30264

CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=123456789
CLASSES=

all: server client

server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client *.tar.gz

dist: tarball
tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .
