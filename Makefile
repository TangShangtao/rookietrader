CXX = g++
TARGET = test
SRC = test.cpp tools/CommonMgr.cpp DataKit/CfgLoader.cpp tools/Logger.cpp TradingSystem/TradingPlatformConnector/TradeGateway/CTPGateway/CTPGateway.cpp
OBJ = build/test.o build/tools/CommonMgr.o build/DataKit/CfgLoader.o build/tools/Logger.o build/TradingSystem/TradingPlatformConnector/TradeGateway/CTPGateway/CTPGateway.o

CXXFLAGS =  -c -Wall -g 

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ -LTradingPlatformAPI/CTP6.3.15/linux -lthosttraderapi_se -ldl -pthread -L/home/mydeps/lib -lboost_filesystem
build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	find build/ -name "*.o" -type f -delete && rm -f $(TARGET)
dir:
	mkdir -p build/BacktestSystem/HisDataReplayer/ build/DataKit/ build/tools/ build/TradingSystem/TradingPlatformConnector/TradeGateway/CTPGateway/