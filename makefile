CXX = g++ 
CXX_FLAGS = -g -Wall -std=c++17 
LFLAGS = zeroInstall/lib/libzmq.a -pthread -lstdc++fs
INC_FLAG = -IzeroInstall/include

SERVER_TARGET = serverApp

all:
	$(CXX) $(CXX_FLAGS) $(INC_FLAG) src/*.cpp -o $(SERVER_TARGET) $(LFLAGS)

clean:
	rm $(SERVER_TARGET)