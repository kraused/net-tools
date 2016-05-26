
# ibutil005: Combine node record and LFTs. Run with python2 ibutil005.py <lid list>

import sys
import os
import ibutilpy


lids = [int(x) for x in sys.argv[1:]]
data = zip(ibutilpy.Get_NodeRecord(lids), ibutilpy.Get_LinearForwardingTable(lids))

print(data)


