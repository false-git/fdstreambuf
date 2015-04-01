CXXFLAGS = -g --std=c++11 -pthread -Wall

all: server client

clean:
	rm -f server client *.o

server: server.o fdstreambuf.o
	$(CXX) -o $@ $^

client: client.o fdstreambuf.o
	$(CXX) -o $@ $^

fdstreambuf.o: fdstreambuf.cc fdstreambuf.h
