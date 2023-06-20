CXX = g++
TARGET = test
SRC = \
test.cpp \
DataKit/CfgLoader.cpp DataKit/CommonMgr.cpp \
tools/Logger.cpp \

OBJ = \
build/test.o \
build/DataKit/CfgLoader.o build/DataKit/CommonMgr.o \
build/tools/Logger.o \

CXXFLAGS = -c -g -Wall -I./

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ -L/home/mydeps/lib
build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	find build/ -name "*.o" -type f -delete && rm -f $(TARGET)
mkdir:
	mkdir -p build/DataKit build/tools 