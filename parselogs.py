__author__ = 'Levi Willmeth'
'''
  This program checks all log files in the current
directory, 2015 LBCC RockSat-C experiment.
  Each log file is scanned for tube activity and coincidences, and a report
is generated.
'''

import os, sys
import plotly.plotly as py
from plotly.graph_objs import *

prefix = "LBCC_"

# Check for log files in the current directory
logs = [f for f in os.listdir(os.curdir) if f.startswith(prefix)]
if(len(logs) is 0):
    print "No logs found."
    sys.exit("Log files must begin with \""+prefix+"\"")

# Aggregate data to determine if a tube is under/over performing
tubeTotals = [0,0,0,0,0,0]

for log in logs:
    print "Opening log: " + log
    with open(log, "r") as data:
        # Store time of hit for each tube, to use while graphing
        tubeResults = [ [], [], [], [], [], [] ]
        # Store time and tube status for all coincidences
        tubeCoincidences = []
        # For each line in the file
        for line in data.readlines():
            # Break into time, and tubes
            time, tubes = line.split()
            # Time may as well be a number instead of a string
            time = int(time)
            # Arduino code doesn't include leading zeros, so
            # we have to pad the beginning on short results.
            tubes = tubes.rjust(6, '0')
            # Log information if there's a coincidence
            if(tubes.count("0") > 1):
                tubeCoincidences.append([time, tubes])
                # print "\t+++ Coincidence @ "+time, tubes
            # Record which tube(s) received a hit during this event
            for i in range(6):
                if(tubes[i] == '0'):
                    tubeResults[i].append(time/1000000)
        # Graph results using plotly web interface
        # I'm thinking x is time, y is hits
        # with one graph showing all 6 tubes side by side.
        # Simultaneous events should be visible as a rise in 2 tubes.
        tubeData = []
        for i in range(6):
            tubeData.append(
                Scatter(
                    x=tubeResults[i],
                    y=range( 1, len(tubeResults[i])+1 ),
                    name="Tube #"+str(i)
                )
            )

        # Summarize single log results:
        logTotalHits = 0
        for t in tubeResults:
            logTotalHits += len(t)
        print "\tTotal of ",len(tubeCoincidences),\
            "coincidences and ",logTotalHits," hits."

        # Send data to graph
        data = Data( tubeData )
        layout = Layout(
            title = '2015 LBCC RockSat-C Flight Data',
            # autosize = False,
            xaxis = XAxis(title='Time (s)', autorange=False),
            yaxis = YAxis(title='Events', autorange=False)
            # xaxis = XAxis(type='log', autorange=True),
            # yaxis = YAxis(type='log', autorange=False)
        )
        # print tubeData #debug only
        #fig = Figure(data=data, layout=layout)
        #plot_url = py.plot(fig, filename=log)

    # Summarize results
    # Display the results per tube:
    for i in range(6):
        print "\tTube {} measured {} hits.".format(i, len(tubeResults[i]))
        # Add this log's results to the totals
        tubeTotals[i] += len(tubeResults[i])
    # Display information about coincidences, if any:
    if(len(tubeCoincidences) is 0):
        print "\t" + log + " contained no coincidences."
    else:
        for event in tubeCoincidences:
            print "\tCoincidence at {} with tubes " \
                  "{}".format(event[0], event[1])

# Summarize tube performance across all logs
print "\nSummarizing results across all logs:"
totalHits = sum(tubeTotals)
for i in range(6):
    print "\tTube {} measured {} hits, {}% of all hits."\
        .format(i, tubeTotals[i], 100*(tubeTotals[i]/float(totalHits)))