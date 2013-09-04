

import numpy
import re
import sys

N_HOST = 5
BASE_DIR = "result.network"

res_ping = numpy.zeros([N_HOST+1, N_HOST+1])
res_perf = numpy.zeros([N_HOST+1, N_HOST+1])

for i in range(1, N_HOST+1):
    for j in range(i, N_HOST+1):
        # read from result.ping.i.j
        f = open("%s/result.ping.%d.%d" % (BASE_DIR, i, j))
        pings = []
        for line in f.readlines():
            r = re.match(r".+time=(.+)ms", line)
            if r:
                time = float(r.group(1))
                pings.append(time)
        ping_avg = numpy.mean(pings)            
        #print ping_avg
        res_ping[i][j] = ping_avg

        # read from result.netperf.i.j
        f = open("%s/result.netperf.%d.%d" % (BASE_DIR, i, j))
        for line in f.readlines():
            r = re.match(r"(\s+\d+(\.\d+)?)+", line)
            if r:
                bandwidth = float(r.group(1))
        #print bandwidth
        res_perf[i][j] = bandwidth

for i in range(1, N_HOST+1):
    for j in range(1, N_HOST+1):
       sys.stdout.write("%f/%d\t" % (res_ping[i][j], res_perf[i][j]))
    sys.stdout.write("\n")
        

