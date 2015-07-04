import os, sys
import matplotlib.pyplot as plt

prefix = "LBCC_"
logs = [f for f in os.listdir(os.curdir) if f.startswith(prefix)]
radar = "NRW-5600 Terrier Improved Orion(RockOn-41.113 Koehler) R3 POSDAT 06252015.txt"

coTimes = []
coAltitudes = []
radTimes = []
radAltitudes = []


altitudes = []
combo = []

for log in logs:
    print "Opening log: " + log
    with open(log, "r") as data:
        for hit in data:
            time, tubes = hit.split()
            if tubes.rjust(6,'0').count('0')>1:
                coTimes.append(round(int(time)/1000000.0,1))

with open(radar, "r") as f:
    print "Opening radar log: " + radar
    f.next()
    i = 0 # Index of coincidence
    for line in f:
        line = line.split()
        alt = int(float(line[9]))
        time = round(float(line[1]),1)

        if time >= 600:
            break

        if i<len(coTimes) and time >= coTimes[i]:
            coAltitudes.append(alt)
            # combo.append([coTimes[i], alt])
            i += 1

        radTimes.append(time)
        radAltitudes.append(alt)
        # altitudes.append([time, alt])

print "Graphing.."
plt.scatter(coTimes, coAltitudes, label='Coincidences', color='r')
plt.plot(radTimes, radAltitudes, label='Radar Altitudes')
plt.xlabel('Time (s)')
plt.ylabel('Altitude (m)')
plt.title('LBCC RocksatC Results')
plt.axis([0, 600, 0, 120000])
plt.legend()
plt.show()