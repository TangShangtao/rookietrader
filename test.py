from pyrookietrader import EventHeader, Tick, EventType

e = EventHeader(1, EventType.EventTick)
print(e.rpcID) 

# class TestMDSpi(MDSpi):
#     def OnTick(self, event):
#         return f"event tick: {event.lastPrice}"
# t = TestMDSpi()
# test(t)