from pyrookietrader import TradeApi, Protocol
import time
import json

class MDReceiver(TradeApi.MDApi):
    def __init__(self, eventUrl, rpcUrl, loggerName, logMode):
        super().__init__(eventUrl, rpcUrl, loggerName, logMode)
        self.Init()
        self.SendPrepareMDReq()

    def OnMDReady(self, event):
        print("python OnMDReady")
    
    def OnTick(self, event):
        print(f"python OnTick {event.instrumentID} {event.lastPrice}")
    
    def OnPrepareMDRsp(self, rsp):
        print("python OnPrepareMDRsp")
        subInstruments = ["IH2406", "IH2407"]
        self.SendSubTickReq(Protocol.ExchangeID.SHFE, subInstruments)

    def OnSubTickRsp(self, rsp):
        print("python OnSubTickRsp")


if __name__ == "__main__":
    
    with open("config.json") as j:
        config = json.load(j)['MDApi']
        
    receiver = MDReceiver(config["eventUrl"],
                          config["rpcUrl"],
                          config["loggerName"],
                          config["logMode"])
    while True:
        time.sleep(10)

