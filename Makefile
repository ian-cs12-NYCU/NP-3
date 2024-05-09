CXX=g++
CXXFLAGS=-std=c++14 -Wall -pedantic -pthread -lboost_system
CXX_INCLUDE_DIRS=/usr/local/include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))


part1: CXXFLAGS += -DNDEBUG
part1: clean http_server console

part2: CXXFLAGS += -DNDEBUG
part2: cgi_server

part1_debug: CXXFLAGS += -DDEBUG 
part1_debug: clean http_server console

cgi_server: cgi_server.cpp
	g++ cgi_server.cpp -o cgi_server -lws2_32 -lwsock32 -std=c++14 -pthread

http_server: http_server.cpp
	$(CXX) http_server.cpp -o http_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

console: console.cpp
	$(CXX) console.cpp -o console.cgi $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

test: Just_for_Test.cpp
	$(CXX) Just_for_Test.cpp -o Just_for_Test $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

clean:
	rm -f http_server console.cgi cgi_server
