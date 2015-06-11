__author__ = 'levi'
import os, sys
prefix = "LBCC_"

# Check for log files in the current directory
logs = [f for f in os.listdir(os.curdir) if f.startswith(prefix)]
if(len(logs) is 0):
    print "No logs found."
    sys.exit("Log files must begin with \""+prefix+"\"")

for log in logs:
    print "Opening log: " + log
    count = 0
    with open(log, "r") as data:
        # For each line in the file
        for line in data.readlines():
            # Break into time, and tubes
            time, tubes = line.split()
            # Pad tubes with leading zeros if needed
            while len(tubes) < 6:
                tubes = "0" + tubes
            # Display information if there's a coincidence
            if(tubes.count("0") > 1):
                print " +++ Coincidence @ "+time, tubes
                count += 1
    # Summarize results
    if(count is 0):
        print " - " + log + " contained no coincidences."
    else:
        print " +++ " + 'count' + " coincidences found in " + log