from pyrookietrader import TradeApi, Protocol
import time
import json

class MDReceiver(TradeApi.MDApi):
    def __init__(self, eventUrl, rpcUrl, loggerName, logMode):
        super().__init__(eventUrl, rpcUrl, loggerName, logMode)
        self.Subscribe(Protocol.EventType.EventMDReady)
        self.Subscribe(Protocol.EventType.EventTick)
        self.Init()
        self.logger.Debug("hello")

    def OnMDApiStart(self):
        self.logger.Debug("python OnMDReady")
        self.SendPrepareMDReq()

    def OnMDReady(self, event):
        self.logger.Debug("python OnMDReady")
        subInstruments = ["rb2407C3150", "rb2407C3200"]
        self.SendSubTickReq(Protocol.ExchangeID.SHFE, subInstruments)

    def OnTick(self, event: Protocol.Tick):
        self.logger.Debug(f"python OnTick {event.instrumentID} {event.lastPrice}")
    


if __name__ == "__main__":
    
    with open("config.json") as j:
        config = json.load(j)['MDApi']
        
    receiver = MDReceiver(config["eventUrl"],
                          config["rpcUrl"],
                          config["loggerName"],
                          config["logMode"])
    # receiver.Join()
    while True:
        time.sleep(10)
