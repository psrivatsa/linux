#*****************************************************************************
# Filename    : POppHwMap.py                                                
# Author      : psrivatsa
#
# Description : Parse the Trace file and map HWStatus to P Opportunity
#
# 1. Open and read txt/csv file line-by-line
# 2. Open and read tarfile
# 3. Open and write txt/csv file
# 4. Functions/def
# 5. Lists and Dict
# 6. split and strip functions
#*****************************************************************************
# usage::
# time python3 ./scripts/POppHwMap.py ./inputs/StrategyIdNameStream.csv ./inputs/POpportunities.2018-11-21.log ./inputs/p.2018-11-19.log ./inputs/trace_data_aa.tar.gz

import os
import sys
import re
import tarfile
from collections import Counter
import logging
import logging.handlers

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
# Get Strategy ID
#########################################################################
def Get_StrategyIDNameStr(line):
    line_s = line.strip()
    Fields = line_s.split(",")
    return Fields

#########################################################################
# Get Hw TRACE Fields
#########################################################################
def Get_HwTraceFields(line):
    lined = line.decode() # decode from bytestream in case of line from tar
    Fields = lined.split("|")
    return Fields

#########################################################################
# Get HW Status             
#########################################################################
def Get_HwStatus(line):
    Fields = line.split("|")
    #Fields = re.findall(r'\d+', line)
    return Fields[0]

#########################################################################
# Get Platform StrategyName
#########################################################################
def Get_StrategyName(line):
    Fields = line.split(",")
    #Fields = re.findall(r'\d+', line)
    return Fields[8]


#########################################################################
# Main Function
#########################################################################
def main():
    logger.info("Message: %s, %s", "MAIN_START", 1)
    print("------------------------------------------------------------------------")
    print("%%%%%%       Map P Opportunities to HW Status            %%%%%%%%%")
    print("------------------------------------------------------------------------")

    #########################################################################
    # 1. Open StrategyIdName file to read
    #########################################################################
    with open(sys.argv[1], 'r') as file:

        StrategyIDList_3L    = []
        StrategyIDList_2L    = []
        StrategyIDStrList_3L = []
        StrategyIDStrList_2L = []
        print("%%%%%% Reading StrategyId HWId Mapping File ... ")
        # Read first relevant line
        line = file.readline()
        
        # Start while loop till end of file
        while line:
            #print(line, end='')
            StrategyID = Get_StrategyIDNameStr(line)
            #print(StrategyID)
            if StrategyID[1] == "3": # 3L Strategies
                StrategyIDList_3L.append(StrategyID[3]) # Name
                StrategyIDStrList_3L.append(StrategyID[0]) # Stream Num
            else: # 2L Strategies
                StrategyIDList_2L.append(StrategyID[3]) # Name
                StrategyIDStrList_2L.append(StrategyID[0]) # Stream Num
            #endif
            line = file.readline()
        #endwhile

    # Close file
    file.close()
    print("%%%%%% Completed Reading StrategyId HWId Mapping File ... ")
    print("//////  %05d 3-LEG Strategies Added ... " % len(StrategyIDList_3L))
    print("//////  %05d 2-LEG Strategies Added ... " % len(StrategyIDList_2L))
    #sys.exit()

    #########################################################################
    # 2. Open P Oppurtunities file to read
    #########################################################################
    with open(sys.argv[2], 'r') as file:

        # Variables
        POppListFull      = []
        RawSeqNumList_3L  = []
        POppList_3L       = []
        POppSeqNumList_3L = []
        POppKeyList_3L    = []
        RawSeqNumList_2L  = []
        POppList_2L       = []
        POppSeqNumList_2L = []
        POppKeyList_2L    = []
        for k in range(8):
            RawSeqNumList_3L.append([])
            POppList_3L.append([])
            POppSeqNumList_3L.append([])
            POppKeyList_3L.append([])
            RawSeqNumList_2L.append([])
            POppList_2L.append([])
            POppSeqNumList_2L.append([])
            POppKeyList_2L.append([])
        #endfor
        TotalOpp_3L = 0
        TotalOpp_2L = 0
        POppListIdx = 0
       
        # Start Function
        print("///////////////////////////////////////////////////////////////////////////")
        print("%%%%%% Reading P Opportunity File ... ")
        # Read first 1 lines and discard
        line = file.readline()
        # P Oppurtunites Header Full
        POppHdr = line.strip() 
        # Read first relevant line
        line = file.readline()
        
        # Start while loop till end of file
        while line:
            #print(line, end='')
            # Get CSV Fields
            Fields = line.split(",")
            StreamNum = int(Fields[2])
            SeqNum = int(Fields[3]) 
            ################################################
            # Create lists
            ################################################
            #print(StreamNum)
            line_i = str(POppListIdx)+","+line.strip() # strip newline char
            if re.search(r'F2F_.*', Fields[7]) is None: #3L CR or BFLY Strategies
                POppList_3L[StreamNum-1].append(line_i) 
                RawSeqNumList_3L[StreamNum-1].append(SeqNum)
            else: # 2L F2F Strategies
                POppList_2L[StreamNum-1].append(line_i) 
                RawSeqNumList_2L[StreamNum-1].append(SeqNum)
            #endif
            # Create Full list with index
            POppListFull.append(line_i)
            POppListIdx += 1
            
            line = file.readline()
            #sys.exit()
        #endwhile
       
        # Display Results
        print("///////////////////////////////////////////////////////////////////////////")
        print("%%%%%% Completed Reading P Opportunity File ... ")
        print("///////////////////////////////////////////////////////////////////////////")
        # 3L Opportunities
        for k in range(8):
            print("//////  %d P 3L Opportunities in STREAM %d ... " % (len(RawSeqNumList_3L[k]), k+1), flush=True)
            POppSeqNumList_3L[k] = Counter(RawSeqNumList_3L[k])
            POppKeyList_3L[k] = sorted(POppSeqNumList_3L[k])
            TotalOpp_3L += len(RawSeqNumList_3L[k])
            #print(POppSeqNumList_3L[k])
            #print(POppKeyList_3L[k])
        # 2L Opportunities
        print("///////////////////////////////////////////////////////////////////////////")
        for k in range(8):
            print("//////  %d P 2L Opportunities in STREAM %d ... " % (len(RawSeqNumList_2L[k]), k+1), flush=True)
            POppSeqNumList_2L[k] = Counter(RawSeqNumList_2L[k])
            POppKeyList_2L[k] = sorted(POppSeqNumList_2L[k])
            TotalOpp_2L += len(RawSeqNumList_2L[k])
            #print(POppSeqNumList_2L[k])
            #print(POppKeyList_2L[k])
        #endfor
        print("///////////////////////////////////////////////////////////////////////////")
        print("//////  %d TOTAL P 3L Opportunities ... " % (TotalOpp_3L), flush=True)
        print("//////  %d TOTAL P 2L Opportunities ... " % (TotalOpp_2L), flush=True)
        print("//////  %d TOTAL P Opportunities ... " % (TotalOpp_3L+TotalOpp_2L), flush=True)
        print("///////////////////////////////////////////////////////////////////////////")

    # Close file
    file.close()

    #########################################################################
    # 3. Open P Orders file to read
    #########################################################################
    with open(sys.argv[3], 'r') as file:

        # Variables
        POrdList      = {} # Dict
        POrdListKey   = []

        # Start Function
        print("///////////////////////////////////////////////////////////////////////////")
        print("%%%%%% Reading P Orders File ... ")
        # Read first line for header
        line = file.readline()
        Fields = line.split(",")
        # FillStatus,ExecutionStatus,ExchangeRTT,T2TTime
        POrdHdr = Fields[11]+","+Fields[12]+","+Fields[15]+","+Fields[16] 
        #print(POrdHdr)
        # Read first relevant line
        line = file.readline()
        
        # Start while loop till end of file
        while line:
            #print(line, end='')
            # Get CSV Fields
            Fields = line.split(",")
            # HwStreamNum,HwSequenceNum,StrategyName
            Key = Fields[17]+","+Fields[18]+","+Fields[8]
            POrdListKey.append(Key)
            # FillStatus,ExecutionStatus,ExchangeRTT,T2TTime
            Val = Fields[11]+","+Fields[12]+","+Fields[15]+","+Fields[16]
            POrdList[Key] = Val
            #print(Key)
            #print(POrdList)
            line = file.readline()
        #endwhile

    # Close file
    file.close()

    #########################################################################
    # 4. Open HW TRACE file to read
    #########################################################################
    with tarfile.open(sys.argv[4], 'r:gz') as tar:
        for member in tar:
            with tar.extractfile(member) as file:
                ################################################
                # Read first 3 lines and discard
                ################################################
                line = file.readline()
                line = file.readline()
                line = file.readline()
              
                PHwHdr = "HwOType,HwStrIdx,HwStatus,HwOIdx,HwCond,HwGP"
                ################################################
                # Read first relevant line
                ################################################
                line = file.readline()
               
                ################################################
                # init 
                ################################################
                POppListMatch     = []
                POppSeqNumIdx_3L  = []
                POppKeyListIdx_3L = []
                POppKeyListC_3L   = []
                POppSeqNumIdx_2L  = []
                POppKeyListIdx_2L = []
                POppKeyListC_2L   = []
                # Read 1st elements to start and init index
                for k in range(8):
                    POppSeqNumIdx_3L.append(0)   # Sequence number Idx is 0 to begin with for all opp in that strem
                    POppKeyListIdx_3L.append(0)  # KeyList index is 0 to begin with for all streams
                    POppKeyListC_3L.append([])
                    if len(POppKeyList_3L[k]) is not 0:
                        POppKeyListC_3L[k] = POppKeyList_3L[k][0] # Get first element
                    #endif
                    POppSeqNumIdx_2L.append(0)   # Sequence number Idx is 0 to begin with for all opp in that strem
                    POppKeyListIdx_2L.append(0)  # KeyList index is 0 to begin with for all streams
                    POppKeyListC_2L.append([])
                    if len(POppKeyList_2L[k]) is not 0:
                        POppKeyListC_2L[k] = POppKeyList_2L[k][0] # Get first element
                    #endif
                #endfor
               
                ################################################
                # Start while loop till end of file
                ################################################
                while line:
                    # print(line, end='')
                   
                    # Get all Fields in Trace line
                    HwTraceFields = Get_HwTraceFields(line)
                    # HW GoalPrice
                    HwGP = HwTraceFields[6]
                    # HW Condition
                    HwCond = HwTraceFields[5]
                    # Get Sequence number trace data line
                    HwSeqNum = int(HwTraceFields[4])
                    # HW Order Index
                    HwOIdx = HwTraceFields[3]
                    # Get HW Strategy Index from trace data line
                    HwStrIdx = HwTraceFields[2]
                    # HW Order Type 2-leg/3-leg
                    HwOType = HwTraceFields[1]
                    # Get HW Status
                    HwStatus = HwTraceFields[0]
                   
                    ################################################
                    # Check Per Stream 3-LEG
                    ################################################
                    if HwOType == "3":
                        # Get StreamId from StreamIdx
                        StreamNum = int(StrategyIDStrList_3L[int(HwStrIdx)]) - 1
                        #print(HwSeqNum, HwStrIdx, HwStatus, StreamNum)
                        if HwSeqNum > POppKeyListC_3L[StreamNum]:
                            #print(StreamNum+1, POppKeyListIdx_3L[StreamNum], POppKeyListC_3L[StreamNum])
                            while HwSeqNum > POppKeyListC_3L[StreamNum]:
                                Incr = POppSeqNumList_3L[StreamNum][POppKeyListC_3L[StreamNum]]
                                POppSeqNumIdx_3L[StreamNum] += Incr
                                POppKeyListIdx_3L[StreamNum] += 1
                                if POppKeyListIdx_3L[StreamNum] >= len(POppKeyList_3L[StreamNum]):
                                    POppKeyListC_3L[StreamNum] = 500000000 # 500m as high number to exit at end of list
                                else:
                                    POppKeyListC_3L[StreamNum] = POppKeyList_3L[StreamNum][POppKeyListIdx_3L[StreamNum]] # Get next keylist
                                #endif
                            #endwhile
                            #print(StreamNum+1, POppKeyListIdx_3L[StreamNum], POppKeyListC_3L[StreamNum])
                            #sys.exit()
                        #endif
                        if HwSeqNum == POppKeyListC_3L[StreamNum]:
                            # Get number of such elements
                            Cnt = POppSeqNumList_3L[StreamNum][HwSeqNum]
                            #print(StreamNum+1,HwOType,HwSeqNum,HwStrIdx,Cnt)
                            for LoopIdx in range(Cnt):
                                # Get Platform Strategy Name from P Opportunities line
                                StrName = Get_StrategyName(POppList_3L[StreamNum][LoopIdx+POppSeqNumIdx_3L[StreamNum]])
                                PStrIdx = StrategyIDList_3L.index(StrName)
                                if int(HwStrIdx) == PStrIdx:
                                    # Create HW Message
                                    HwMessage = HwOType+","+HwStrIdx+","+HwStatus+","+HwOIdx+","+HwCond+","+HwGP
                                    # Get HW Strategy Index from trace data line
                                    MatchOpp = POppList_3L[StreamNum][LoopIdx+POppSeqNumIdx_3L[StreamNum]]+","+HwMessage
                                    POppListMatch.append(MatchOpp)
                                    print(MatchOpp, flush=True)
                                #endif
                            #endfor  
                        #endif
                    #endif HwOType=3
                   
                    ################################################
                    # Check Per Stream 2-LEG
                    ################################################
                    if HwOType == "2":
                        # Get StreamId from StreamIdx
                        StreamNum = int(StrategyIDStrList_2L[int(HwStrIdx)]) - 1
                        #print(HwSeqNum, HwStrIdx, HwStatus, StreamNum)
                        if HwSeqNum > POppKeyListC_2L[StreamNum]:
                            #print(StreamNum+1, POppKeyListIdx_2L[StreamNum], POppKeyListC_2L[StreamNum])
                            while HwSeqNum > POppKeyListC_2L[StreamNum]:
                                Incr = POppSeqNumList_2L[StreamNum][POppKeyListC_2L[StreamNum]]
                                POppSeqNumIdx_2L[StreamNum] += Incr
                                POppKeyListIdx_2L[StreamNum] += 1
                                if POppKeyListIdx_2L[StreamNum] >= len(POppKeyList_2L[StreamNum]):
                                    POppKeyListC_2L[StreamNum] = 500000000 # 500m as high number to exit at end of list
                                else:
                                    POppKeyListC_2L[StreamNum] = POppKeyList_2L[StreamNum][POppKeyListIdx_2L[StreamNum]] # Get next keylist
                                #endif
                            #endwhile
                            #print(StreamNum+1, POppKeyListIdx_2L[StreamNum], POppKeyListC_2L[StreamNum])
                            #sys.exit()
                        #endif
                        if HwSeqNum == POppKeyListC_2L[StreamNum]:
                            # Get number of such elements
                            Cnt = POppSeqNumList_2L[StreamNum][HwSeqNum]
                            #print(StreamNum+1,HwOType,HwSeqNum,HwStrIdx,Cnt)
                            for LoopIdx in range(Cnt):
                                # Get Platform Strategy Name from P Opportunities line
                                StrName = Get_StrategyName(POppList_2L[StreamNum][LoopIdx+POppSeqNumIdx_2L[StreamNum]])
                                PStrIdx = StrategyIDList_2L.index(StrName)
                                if int(HwStrIdx) == PStrIdx:
                                    # Create HW Message
                                    HwMessage = HwOType+","+HwStrIdx+","+HwStatus+","+HwOIdx+","+HwCond+","+HwGP
                                    # Get HW Strategy Index from trace data line
                                    MatchOpp = POppList_2L[StreamNum][LoopIdx+POppSeqNumIdx_2L[StreamNum]]+","+HwMessage
                                    POppListMatch.append(MatchOpp)
                                    print(MatchOpp, flush=True)
                                #endif
                            #endfor  
                        #endif
                    #endif HwOType=3
                   
                    ################################################
                    # READ Next Line
                    ################################################
                    line = file.readline()
                #endwhile  
            file.close()
            #endwith
        #endfor

    # Close file
    tar.close()
    #endwith

    #########################################################################
    # 5. Write Out Merged Results
    #########################################################################
    with open('./outputs/POppHwMap.csv', 'w') as file:
        MIdx = 0
        HwMessageFill   = "-1,-1,-1,-1,-1,-1"
        POrdMessageFill = "-1,-1,-1,-1"
        # CSV Header
        file.write(POppHdr+","+POrdHdr+","+PHwHdr+'\n')
        # Dict of matched HW entries
        POppListMatchHw = {}
        # Create key list based on Match list
        for line in POppListMatch:
            Fields = line.split(",")
            Key = Fields[0]
            POppListMatchHw[Key] = line
        #endfor
        # DATA
        for FIdx in range(len(POppListFull)):
            POFields = POppListFull[FIdx].split(",")
            Key = POFields[0]
            MappedStr = POppListFull[FIdx]
            #################################################################
            # Add the HW Status
            #################################################################
            if Key in POppListMatchHw:
                Val = POppListMatchHw[Key]
                #print(Key, Val)
                MappedStr = Val
            else:
                MappedStr = MappedStr+","+HwMessageFill
            #endif
            #################################################################
            # Add the P Order Status
            #################################################################
            Key = POFields[3]+","+POFields[4]+","+POFields[8]
            if Key in POrdList:
                Val = POrdList[Key]
                #print(Key, Val)
                MappedStr = MappedStr+","+Val+'\n'
            else:
                MappedStr = MappedStr+","+POrdMessageFill+'\n'
            #endif
            file.write(MappedStr)
        #endfor
    file.close()
    #endwith
   
    print("------------------------------------------------------------------------")
    print("%%%%%%        HW Strategy Status and Opportunity Mapping Done  %%%%%%%%%")
    print("------------------------------------------------------------------------")
    logger.info("Message: %s, %s", "MAIN_END", 1)


if __name__ == "__main__":
    setup_logging(level=logging.DEBUG, enable_console=False)
    #logger.debug("Message: %s", "debug")
    #logger.warning("warning")
    #logger.error("error")
    #logger.critical("critical")
    sys.exit(main())
