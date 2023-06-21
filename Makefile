CXX = g++
TARGET = test

SRC = \
test.cpp \
$(wildcard DataKit/*.cpp) \
$(wildcard tools/*.cpp) \
$(wildcard TradingSystem/TradeGateway/CTPGateway/CTPGateway_impl/*.cpp) \


OBJ = \
$(patsubst %.cpp, build/%.o, $(SRC)) \

CXXFLAGS = -c -g -Wall -std=c++11 -I./

build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ -LTradingPlatformAPI/OpenCTP6.3.15 -pthread -lthosttraderapi_se -lboost_filesystem


mkdir:
	mkdir -p build/DataKit build/tools build/TradingSystem/TradeGateway/CTPGateway/CTPGateway_impl
