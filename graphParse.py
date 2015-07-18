import os, sys
import matplotlib.pyplot as plt

flight_log = "logs/LBCC_00_Final.TXT"
# radar_log = "logs/RSC2015_RadarData/NRW-5600 Terrier Improved" \
#             " Orion(RockOn-41.113 Koehler) R3 POSDAT 06252015.txt"
radar_log = "logs/RSC2015_RadarData/NRW-5600 Terrier Improved " \
            "Orion(RockOn-41.113 Koehler) R5 POSDAT 06252015.txt"

coTimes = []
coAltitudes = []
hitTimes = []
hitAltitudes = []
radTimes = []
radAltitudes = []
eachTubeCount = [[] for x in range(6)] # 6 empty lists

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
        tubes = tubes.rjust(6, '0')
        time = round(int(time)/1000000.0,1)
        if time >= 600:
            break

        hitTimes.append(time)
        if tubes.count('0')>1:
            coTimes.append(time)

        # Increment individual tube counts
        for i in range(6):
                if(tubes[i] == '0'):
                    eachTubeCount[i].append(time)

# Parse the radar data
print "Opening radar log: " + radar_log
with open(radar_log, "r") as f:
    f.next()
    i = 0 # Index of coincidence
    j = 0 # Index of hit
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
        elif j<len(hitTimes) and time >= hitTimes[j]:
            hitAltitudes.append(alt)
            j += 1

        radTimes.append(time)
        radAltitudes.append(alt)

print "Graphing.."
# subplot(212) designates room for 2 plots, vertical, with plt=1
# plt.subplot(211)
# plt.plot(radTimes, radAltitudes, label='Radar Altitudes', color='k')
plt.scatter(hitTimes, hitAltitudes, label='Hits', s=.5, color='b')
plt.scatter(coTimes, coAltitudes, label='Coincidences', color='r')
plt.xlabel('Time (s)')
plt.ylabel('Altitude (m)')
plt.title('Hits and coincidences by altitude')
plt.axis([0, 600, 0, 120000])
plt.legend()
plt.show()

#Let's try compensating for time.
def combos(min, step, list):
    ''' Returns a total count of objects in list, between min and min+step '''
    return len([x for x in list if x>=min and x<=min+step])

step = 5000
hitsPerAlt = []
cosPerAlt = []

for chunk in range(0,130000,step):
    # Find total hits at this altitude range
    hitsHere = combos(chunk, step, hitAltitudes)
    cosHere = combos(chunk, step, coAltitudes)
    # Find total time (s) spent in this altitude range,
    # and adjust because each count is 0.1 sec.
    timeHere = combos(chunk, step, radAltitudes)/10.0

    if timeHere>0:
        hitsPerAlt.append([chunk+step, hitsHere/timeHere])
        cosPerAlt.append([chunk+step, cosHere/timeHere])


# plt.subplot(212)
# plt.subplots_adjust(hspace = 0.4) # Adjusts graph downward slightly, to fix formatting
# plt.plot(altRange, hitsPerAlt, 'k-o')
plt.plot(*zip(*hitsPerAlt), label='Hits', color='b')
plt.plot(*zip(*cosPerAlt), label='Coincidences', color='r')
plt.xlabel('Altitude (m)')
plt.ylabel('Hits per Second')
plt.title('Events adjusted for time spent at 5 km altitude segments')
plt.axis([0, 120000, 0, 5])
plt.legend()
plt.show()