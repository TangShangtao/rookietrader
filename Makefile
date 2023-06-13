CXX = g++
TARGET = test
SRC = test.cpp HisDataReplayer/CommonMgr.cpp DataKit/CfgLoader.cpp tools/Logger.cpp
OBJ = build/test.o build/HisDataReplayer/CommonMgr.o build/DataKit/CfgLoader.o build/tools/Logger.o

CXXFLAGS = -c -Wall -g 

$(TARGET): $(OBJ)
	$(CXX) -pthread -o $@ $^
build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	find build/ -name "*.o" -type f -delete && rm -f $(TARGET)
dir:
	mkdir -p build/HisDataReplayer/ build/DataKit/ build/tools/