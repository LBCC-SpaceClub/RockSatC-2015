import os, sys
import matplotlib.pyplot as plt

flight_log = "logs/LBCC_00_Final.TXT"
radar_log = "logs/RSC2015_RadarData/NRW-5600 Terrier Improved" \
            " Orion(RockOn-41.113 Koehler) R3 POSDAT 06252015.txt"

coTimes = []
coAltitudes = []
radTimes = []
radAltitudes = []

# Parse our flight data
print "Opening flight log: " + flight_log
with open(flight_log, "r") as f:
    for hit in f:
        '''
        Sample data: 234440923\t111010
        Data is recorded as microseconds, tube status
        There are 6 tubes, each 0 represents a hit on that tube
        More than one zero represents a coincidence between tubes
        Catch: Leading zeros are omitted
        '''
        time, tubes = hit.split()
        if tubes.rjust(6,'0').count('0')>1:
            coTimes.append(round(int(time)/1000000.0,1))

# Parse the radar data
print "Opening radar log: " + radar_log
with open(radar_log, "r") as f:
    f.next()
    i = 0 # Index of coincidence
    for line in f:
        '''
        The radar log has a lot of data in it, but the important
        sections are time and altitude. (maybe velocity?)
        Time is the second col ([1]), and altitude is tenth ([9])
        '''
        line = line.split()
        # Had trouble going from '9.5' to 9 without using both?
        alt = int(float(line[9]))
        time = round(float(line[1]), 1)

        # Each radar station recorded a different flight length,
        # but all we care about is the first 10 minutes.
        if time >= 600:
            break

        if i<len(coTimes) and time >= coTimes[i]:
            coAltitudes.append(alt)
            i += 1

        radTimes.append(time)
        radAltitudes.append(alt)

print "Graphing.."
plt.scatter(coTimes, coAltitudes, label='Coincidences', color='r')
plt.plot(radTimes, radAltitudes, label='Radar Altitudes')
plt.xlabel('Time (s)')
plt.ylabel('Altitude (m)')
plt.title('LBCC RocksatC Results')
plt.axis([0, 600, 0, 120000])
plt.legend()
plt.show()