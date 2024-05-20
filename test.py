from pyrookietrader import TradeApi, Protocol
import time

class MDReceiver(TradeApi.MDApi):
    def __init__(self):
        super().__init__("./config.json")
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
    
    receiver = MDReceiver()
    # receiver.Join()
    while True:
        time.sleep(10)

