

import re
import sys
import os

import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import matplotlib.font_manager as font_manager
import itertools

OUTPUT = os.path.expanduser("~/Dropbox/paper/rsm/figures/mpaxos_transaction_parallel.eps")
BASE_DIR = "result.mpaxos.transaction.parallel"
node_names = ["", "TK", "SG", "SN", "IL", "CL"]
colors=["1.0", "0.0", "0.2", "0.4", "0.6", "0.8"]

mpl.rcParams['figure.figsize'] = (8,5)
mpl.rcParams['axes.labelsize'] = "large" 
mpl.rcParams['savefig.bbox'] = "tight" 

rates = []
for j in range(5+1):
    rates.append([])

step = 5
for i in range(5, 1500+1, 5):
    for j in range(1, 5+1):
        rate = None
        f = open("result.mpaxos.transaction.parallel/result.mpaxos.%d.%d" % (i, j))
        ratesum = 0.0
        rate=0.0
        for line in f.readlines():
            r = re.match(r".+ (\d+) proposals commited in (\d+)ms.+", line)
            if r:
                c = int(r.group(1))
                t = int(r.group(2))
                rate = c / (t / 1000.0)
                rates[j].append(rate)
                ratesum += rate
        sys.stdout.write("\t%f" % rate)
#    rates[0].append(ratesum)
    rates[0].append(0)
    sys.stdout.write("\n")

bottom=np.zeros(len(rates[1]))
for j in range(1, 5+1):
    bottom += rates[j]

locs = np.arange(len(rates[1])) * step
for j in range(1, 5+1):
    bottom -= rates[j]
    plt.bar(locs, rates[j], bottom=bottom, color=colors[j], label=node_names[j], width=step, linewidth=0.5, edgecolor=colors[j])
    plt.legend(loc='upper left')

plt.ylim((0, 7000))
plt.xlim((0, 1500))
plt.xlabel("number of parallel mpaxos transactions at each site.")
plt.ylabel("throughput(op/s)")
#plt.show()
plt.savefig(OUTPUT)    

