import os, sys
import matplotlib.pyplot as plt

prefix = "LBCC_"
logs = [f for f in os.listdir(os.curdir) if f.startswith(prefix)]

for log in logs:
    print "Opening log: " + log
    with open(log, "r") as data:
        coTimes = []
        for hit in data:
            time, tubes = hit.split()
            if tubes.rjust(6,'0').count('0')>1:
                coTimes.append(round(int(time)/1000000.0,1))

        print coTimes
        print len(coTimes)