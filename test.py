from pyrookietrader import *

class TestMDSpi(MDSpi):
    def OnTick(self, event):
        return f"event tick: {event.lastPrice}"
t = TestMDSpi()
test(t)