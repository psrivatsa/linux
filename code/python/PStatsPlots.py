#*****************************************************************************
# Filename    : PStatsPlots.py                                                
# Author      : psrivatsa
#
# Description : Parse the P Merged file (OPP + HWSTATUS + ORDERS) for Satistics and graphs 
# 1. Open and close txt file to read
# 2. Open and close txt file to write
# 3. call simple bar plot with subplots
# 4. Logger features
# 5. Usage of lists and Dict
# 6. strip and split use cases
#*****************************************************************************
# usage::
# time python3 ./scripts/PStatsPlots.py POppHwMap_2018_12_17.csv

import os
import sys
import re
import tarfile
from collections import Counter
import logging
import logging.handlers
import matplotlib.pyplot as plt

logger = logging.getLogger()
LOG_FORMATTER = logging.Formatter(
    "%(asctime)s.%(msecs)03d - %(name)s - %(levelname)s - " +
    "%(lineno)s - %(funcName)s - " +
    "%(message)s",
    "%Y%m%d %H:%M:%S")


def setup_logging(level=logging.INFO, enable_console=True):
    file_log_handler = logging.handlers.RotatingFileHandler(
        "__" + os.path.basename(__file__) + ".main__" + ".log",
        maxBytes=1000000,
        backupCount=5)
    console_log_handler = logging.StreamHandler()
    logger.addHandler(file_log_handler)
    if enable_console:
        logger.addHandler(console_log_handler)
    logger.setLevel(level)
    for handler in logging.root.handlers:
        handler.setFormatter(fmt=LOG_FORMATTER)

#########################################################################
# Common Definitions/Tasks
#########################################################################

#########################################################################
# Main Function
#########################################################################
def main():
    logger.info("Message: %s, %s", "MAIN_START", 1)
    print("------------------------------------------------------------------------")
    print("%%%%%%  P Opportunities Statistics Generation            %%%%%%%%%")
    print("------------------------------------------------------------------------")

    #########################################################################
    # Create empty dict entries
    #########################################################################
    # Init
    MDict = {}
    hh = 9
    mm = 15
    for k in range(375):
        hh_s = str(hh)
        mm_s = str(mm)
        if len(hh_s) == 1:
            hh_s = "0"+hh_s
        #endif
        if len(mm_s) == 1:
            mm_s = "0"+mm_s
        #endif
        Key = hh_s+":"+mm_s
        MDict[Key] = []
        # Increment Minutes/hour
        if mm == 59:
            mm = 0
            hh += 1
        else:
            mm += 1
            hh = hh
        #endif
    #endfor

    #########################################################################
    # 1. Open Merged file to read
    #########################################################################
    with open(sys.argv[1], 'r') as file:

        print("%%%%%% Reading Merged File ... ")
        # Discard header
        line = file.readline()
        # Read first relevant line
        line = file.readline()
        
        # Start while loop till end of file
        while line:
            # print(line)
            # Strip of newline character and get hh:mm key
            line = line.strip()
            Fields = line.split(",")
            TimeStamp = Fields[2]
            ts0 = TimeStamp.split(" ")
            ts1 = ts0[1].split(".")
            ts2 = ts1[0].split(":")
            hh = ts2[0]
            mm = ts2[1]
            ss = ts2[2]
            Key = hh+":"+mm
            # Get the OppValid and HWStatus
            OppValid = Fields[31]
            HWStatus = Fields[37]
            # If OppValid check Status and put it in
            if OppValid == "True":
                MDict[Key].append(HWStatus) 
            #endif

            # Read next line
            line = file.readline()
        #endwhile

    # Close file
    file.close()
    print("%%%%%% Completed Reading Merged File ... ")
    #sys.exit()

    print("------------------------------------------------------------------------")
    print("%%%%%% 1. Total Genuine opportunities Fired                    %%%%%%%%%")
    print("%%%%%% 2. Total Genuine opportunities Not Fired due to OPS     %%%%%%%%%")
    print("%%%%%% 3. Total Genuine opportunities Not Fired due to CANSEND %%%%%%%%%")
    print("%%%%%% 4. Total Genuine opportunities with no HWStatus         %%%%%%%%%")
    print("------------------------------------------------------------------------")
    Val = ["1f", "1e", "1a", "-1"] # sys.argv[2]
    xa_list = []
    ya_list = []
    za_list = []
    ka_list = []
    la_list = []
    Cnt_y = 0
    Cnt_z = 0
    Cnt_k = 0
    Cnt_l = 0
    for Key in MDict:
        #print(Key, Counter(MDict[Key]))
        MDict_C = Counter(MDict[Key])
        # Time
        xa_list.append(Key)
        # Orders Fired
        ya_list.append(MDict_C[Val[0]])
        Cnt_y += MDict_C[Val[0]]
        # Orders Not Fired OPS
        za_list.append(MDict_C[Val[1]])
        Cnt_z += MDict_C[Val[1]]
        # Orders Not Fired CANSEND
        ka_list.append(MDict_C[Val[2]])
        Cnt_k += MDict_C[Val[2]]
        # No HWStatus for Opportunities
        la_list.append(MDict_C[Val[3]])
        Cnt_l += MDict_C[Val[3]]
    #endfor
    ###################################################################
    # Show Plot Induvidually
    ###################################################################
    # 1.
    #y_fig = plt.figure(1)
    #plt.xlabel('Market Time')
    #plt.ylabel('Number of Orders per Minute')
    #plt.title('Genuine Orders Fired = %d' %Cnt_y)
    #plt.bar(xa_list, ya_list, color='b', width=0.8, align='center')
    #plt.ylim(0,300)
    #y_fig.show()
    ## 2.
    #z_fig = plt.figure(2)
    #plt.xlabel('Market Time')
    #plt.ylabel('Number of Orders Not Fired due to OPS per Minute')
    #plt.title('Orders Not Fired due to OPS = %d' %Cnt_z)
    #plt.bar(xa_list, za_list, color='g', width=0.8, align='center')
    #plt.ylim(0,300)
    #z_fig.show()
    ## 3.
    #k_fig = plt.figure(3)
    #plt.xlabel('Market Time')
    #plt.ylabel('Number of Orders Not Fired due to CANSEND per Minute')
    #plt.title('Orders Not Fired due to CANSEND = %d' %Cnt_k)
    #plt.bar(xa_list, ka_list, color='r', width=0.8, align='center')
    #plt.ylim(0,300)
    #k_fig.show()
    ## 4.
    #l_fig = plt.figure(4)
    #plt.xlabel('Market Time')
    #plt.ylabel('Number of Opportunites with no HWStatus per Minute')
    #plt.title('Orders with no HWStatus = %d' %Cnt_l)
    #plt.bar(xa_list, la_list, color='r', width=0.8, align='center')
    #plt.ylim(0,300)
    #l_fig.show()
    #input() # Wait
    ###################################################################

    ###################################################################
    # Print Stats to File
    ###################################################################
    with open('./PStats.csv', 'w') as file:
        file.write("Time,OrdersFired(1F),nOrdersOPS(1E),nOrdersCANSEND(1A),nOrdersHwStatus(-1)\n")
        k = 0
        for Key in MDict:
            file.write("%s, %d, %d, %d, %d\n" %(Key, ya_list[k], za_list[k], ka_list[k], la_list[k]))
            k += 1
        #endfor
    file.close()
    #endwith

    ###################################################################
    ###################################################################
    # Show as Subplots
    ###################################################################
    plt.figure(1)
    # 1.
    plt.subplot(221)
    plt.xlabel('Market Time')
    plt.ylabel('Number of Orders per Minute')
    plt.title('Genuine Orders Fired = %d' %Cnt_y)
    plt.bar(xa_list, ya_list, color='b', width=0.8, align='center')
    plt.ylim(0,300)
    # 2.
    plt.subplot(222)
    plt.xlabel('Market Time')
    plt.ylabel('Number of Orders Not Fired due to OPS per Minute')
    plt.title('Orders Not Fired due to OPS = %d' %Cnt_z)
    plt.bar(xa_list, za_list, color='g', width=0.8, align='center')
    plt.ylim(0,300)
    # 3.
    plt.subplot(223)
    plt.xlabel('Market Time')
    plt.ylabel('Number of Orders Not Fired due to CANSEND per Minute')
    plt.title('Orders Not Fired due to CANSEND = %d' %Cnt_k)
    plt.bar(xa_list, ka_list, color='r', width=0.8, align='center')
    plt.ylim(0,300)
    # 4.
    plt.subplot(224)
    plt.xlabel('Market Time')
    plt.ylabel('Number of Opportunites with no HWStatus per Minute')
    plt.title('Orders with no HWStatus = %d' %Cnt_l)
    plt.bar(xa_list, la_list, color='r', width=0.8, align='center')
    plt.ylim(0,300)
    # show
    plt.show()

    print("------------------------------------------------------------------------")
    print("%%%%%%  Statistics Generation Complete                         %%%%%%%%%")
    print("------------------------------------------------------------------------")
    logger.info("Message: %s, %s", "MAIN_END", 1)


if __name__ == "__main__":
    setup_logging(level=logging.DEBUG, enable_console=False)
    #logger.debug("Message: %s", "debug")
    #logger.warning("warning")
    #logger.error("error")
    #logger.critical("critical")
    sys.exit(main())
