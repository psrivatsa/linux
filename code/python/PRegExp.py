#*****************************************************************************
# Filename    : StrategyIdName.py                                                
# Author      : psrivatsa
#
# Description : Parse the output of file and get StrategyId and StrategyName list
#
# 1. Use of regular expression library
#*****************************************************************************

import sys
import re

#print("------------------------------------------------------------------------")
#print("%%%%%%           Get Strategy Name and Stragey ID List         %%%%%%%%%")
#print("------------------------------------------------------------------------")

#########################################################################
# Common Definitions/Tasks
#########################################################################


#########################################################################
# Main Function
#########################################################################
# Open Streams file to read
#########################################################################
with open(sys.argv[1], 'r') as file:

    # Empty List
    StreamD = {}
    
    # read first line
    line = file.readline()
    
    # Start while loop till end of file
    while line:
        # Strip of newline character
        line = line.strip()
        # Get CSV Fields
        Fields = line.split(",")
        # Get token name which is key
        Key = Fields[1]
        # Set value of key as stream number
        StreamD[Key] = Fields[0]
        # read next line
        line = file.readline()
    #endwhile

# Close file
file.close()

#print(StreamD)
#print(len(StreamD))
#sys.exit()

#########################################################################
# Open IOCManager file to read
#########################################################################
with open(sys.argv[2], 'r') as file:

    # Empty List
    StrList = []
    StrIdx     = 0
    StrIdx_3L  = 0
    StrIdx_2L  = 0
    # read first line
    line = file.readline()
    
    # Start while loop till end of file
    while line:
        if "strategyName=" and "active=1" in line: 
            #print(line, end='')
            # search for =StrName followed by space
            m = re.search('=(.+?) ', line)
            if m:
                StrName = m.group(1)
                #print(StrName)
                # Check if StrName is already in List
                if StrName not in StrList:
                    # Check if strategy got rejected
                    line = file.readline()
                    if "error" not in line:
                        # Get Stream Number from token name
                        Fields = StrName.split("_")
                        if Fields[0] == "F2F":
                            StrType = 2
                            StrIdx = StrIdx_2L
                            StrIdx_2L += 1
                        else:
                            StrType = 3 
                            StrIdx = StrIdx_3L
                            StrIdx_3L += 1
                        #endif
                        TokenName = Fields[1]
                        StreamNum = StreamD[TokenName]
                        #print(StrIdx,",",StrName)
                        print(StreamNum,StrType,StrIdx,StrName,sep=",")
                        StrList.append(StrName)
                    #endif
                #endif
            else:
                print("ERROR - incompatible input string on line")
                print(line, end='')
                sys.exit()
            #endif
        #endif
        
        line = file.readline()
    #endwhile

# Close file
file.close()

#print("------------------------------------------------------------------------")
#print("%%%%%%           Strategy Name and Stragey ID List Done        %%%%%%%%%")
#print("------------------------------------------------------------------------")
