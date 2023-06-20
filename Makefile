CXX = g++
TARGET = test
SRC = \
test.cpp \
DataKit/CfgLoader.cpp DataKit/CommonMgr.cpp \
tools/Logger.cpp \
TradingSystem/TradeGateway/CTPGateway/CTPGateway.cpp \

OBJ = \
build/test.o \
build/DataKit/CfgLoader.o build/DataKit/CommonMgr.o \
build/tools/Logger.o \
build/TradingSystem/TradeGateway/CTPGateway/CTPGateway.o \

CXXFLAGS = -c -g -Wall -I./

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ -LTradingPlatformAPI/CTP6.3.15/linux -pthread -lthosttraderapi_se -lboost_filesystem
build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	find build/ -name "*.o" -type f -delete && rm -f $(TARGET)
mkdir:
	mkdir -p build/DataKit/ build/tools/ build/TradingSystem/TradeGateway/CTPGateway/