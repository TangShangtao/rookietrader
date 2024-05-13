# pip install pybind11-stubgen
from pyrookietrader.protocol import *

t = Tick(1)
print(t.generateTime) 

# class TestMDSpi(MDSpi):
#     def OnTick(self, event):
#         return f"event tick: {event.lastPrice}"
# t = TestMDSpi()
# test(t)