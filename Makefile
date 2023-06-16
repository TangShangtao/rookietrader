CXX = g++
TARGET = test
SRC = test.cpp tools/CommonMgr.cpp DataKit/CfgLoader.cpp tools/Logger.cpp
OBJ = build/test.o build/tools/CommonMgr.o build/DataKit/CfgLoader.o build/tools/Logger.o

CXXFLAGS =  -c -Wall -g 

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ -ldl -pthread
build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	find build/ -name "*.o" -type f -delete && rm -f $(TARGET)
dir:
	mkdir -p build/BacktestSystem/HisDataReplayer/ build/DataKit/ build/tools/