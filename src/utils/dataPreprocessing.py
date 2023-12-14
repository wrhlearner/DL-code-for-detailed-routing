import os
import re

def parseDefaultOneLine(line, shapeCost=8, markerCost=32) -> list:
    """Parse 1 line of iteration setting"""
    # remove delimiters
    tmp = re.split(r'[\{\}\,]+', line)
    iteData = []
    # get size, offset, and mazeEndIter
    iteData.append(int(re.sub(r'\s+', '', tmp[1])))
    iteData.append(int(re.sub(r'\s+', '', tmp[2])))
    iteData.append(int(re.sub(r'\s+', '', tmp[3])))
    # get 1st cost
    compo = re.split(r'[\s\*]+', tmp[4])
    if len(compo) == 2: # no coefficient before shapeCost
        iteData.append(shapeCost)
    else:               # shapeCost with coefficient
        iteData.append(int(compo[1]) * shapeCost)
    # get 2nd cost
    compo = re.split(r'[\s\*]+', tmp[5])
    if len(compo) == 2: # no coefficient before shapeCost or markerCost
        if compo[1] == 'shapeCost':
            iteData.append(shapeCost)
        elif compo[1] == '0':
            iteData.append(0)
        else:
            iteData.append(markerCost)
    else:               # cost with coefficient
        iteData.append(int(compo[1]) * markerCost)
    # get 3rd cost
    compo = re.split(r'[\s\*]+', tmp[6])
    if len(compo) == 2: # no coefficient before shapeCost
        iteData.append(shapeCost)
    else:               # shapeCost with coefficient
        iteData.append(int(compo[1]) * shapeCost)
    # get decay
    iteData.append(float(re.sub(r'\s+', '', tmp[7])))
    # get mode
    mode = re.sub(r'[\s\:]+', '', tmp[8])
    if mode[-7:] == 'ModeALL':
        iteData.append('ALL')
    elif mode[-7:] == 'ModeDRC':
        iteData.append('DRC')
    else:
        iteData.append('NEARDRC')
    # get followGuide
    followGuide = re.sub(r'\s+', '', tmp[9])
    if followGuide == 'false':
        iteData.append(False)
    else:
        iteData.append(True)
        
    return iteData

# parse default strategy
def parseDefaultStrategy(inPath, outPath, shapeCost=8, markerCost=32) -> list:
    """Parse default strategy, save result to file, and return parsing status.
    
    Inputs:
    inPath: input FlexDR.cpp file path
    outPath: output result file path
    shapeCost: default shapeCost
    markerCost: default markerCost
    
    Return:
    Return list of strategy data.
    """
    # check file exists
    inputPath = os.path.join(inPath, "FlexDR.cpp")
    if not os.path.exists(inputPath):
        print("FlexDR.cpp for default setting doesn't exist in {}!".format(inputPath))
        return None
    # open, read, and extract coefficients from input file
    data = [] # temp coeff data
    count = 1
    with open(inputPath, 'r') as f:
        line = f.readline()
        while count < 1030:
            count = count + 1
            # check strategy() function
            if line == "static std::vector<FlexDR::SearchRepairArgs> strategy()\n":
                print("Start parsing iteration data!")
                # skip prefix lines
                line = f.readline()
                line = f.readline()
                line = f.readline()
                line = f.readline()
                count = count + 3
                # parse iteration settings
                while line != "  };\n":
                    # parse data
                    tempData = parseDefaultOneLine(line, shapeCost=shapeCost, markerCost=markerCost)
                    data.append(tempData)
                    # read next line
                    line = f.readline()
                    count = count + 1
                print("End parsing iteration data!")
            else:
                line = f.readline()
    # check output path exists
    if not os.path.exists(outPath):
        print("Output path {} for default setting data doesn't exist!".format(outPath))
        return None
    # create and write to output file
    with open(os.path.join(outPath, "defaultStrategy.txt"), 'w') as f:
        for line in data:
            for element in line:
                f.write("{},".format(element))
            f.write("@")
    return data

def parseJSONFile(filename) -> list:
    """parse JSON file and return an output vector
    
    Inputs: 
    filename: path to 5_3_route.json file
    
    Return:
    A output vector containing the number of violations for each iteration
    """
    # check file exists
    if not os.path.exists(filename):
        print(filename)
        print("JSON file {} doesn't exist!".format(filename))
        return None
    # read and parse lines
    data = []
    with open(filename, 'r') as f:
        while True:
            line = f.readline()
            tmp = re.split(r'[\"\,\s\:]+', line)
            if len(tmp) > 2 and tmp[1] == 'detailedroute__route__drc_errors__iter':
                data.append(int(tmp[3]))
            if line == "}":
                break
    # return result
    return data

# parse default log files
def parseDefaultLogFiles(inPath, outPath) -> int:
    """Parse log files in the whole directory
    
    Inputs:
    inPath: log file directory
    outPath: output file path
    
    Return:
    Return list of output data.
    """
    # check path exists
    if not os.path.exists(inPath):
        print("Default log file path {} doesn't exist!".format(inPath))
        return None
    # check 5_3_route.json file exists and get outputs
    data = []
    for root, dirs, files in os.walk(inPath):
        for file in files:
            if file == "5_3_route.json":
                # parse json file for outputs
                data.append(parseJSONFile(os.path.join(root, file)))
                print("Data of {}: {}".format(os.path.join(root, file), parseJSONFile(os.path.join(root, file))))
    # check output path exists
    if not os.path.exists(outPath):
        print("Output path {} for default log data doesn't exist!".format(outPath))
        return None
    # create and write to output file
    with open(os.path.join(outPath, "defaultOutputs.txt"), 'w') as f:
        for line in data:
            for element in line:
                f.write("{},".format(element))
            f.write("\n")
            
    return data

def parseOneLine(line) -> list:
    """Parse 1 line of iteration setting"""
    # remove delimiters
    tmp = re.split(r'[\{\}\,\t]+', line)
    iteData = []
    # get size, offset, mazeEndIter, 3 types of costs, and decay
    iteData.append(int(re.sub(r'\s+', '', tmp[1])))
    iteData.append(int(re.sub(r'\s+', '', tmp[2])))
    iteData.append(int(re.sub(r'\s+', '', tmp[3])))
    iteData.append(int(re.sub(r'\s+', '', tmp[4])))
    iteData.append(int(re.sub(r'\s+', '', tmp[5])))
    iteData.append(float(re.sub(r'\s+', '', tmp[6])))
    # get mode
    mode = re.sub(r'[\s\:]+', '', tmp[7])
    if mode[-3:] == 'ALL':
        iteData.append('ALL')
    elif mode[-7:] == 'NEARDRC':
        iteData.append('NEARDRC')
    else:
        iteData.append('DRC')
    # get followGuide
    followGuide = re.sub(r'\s+', '', tmp[8])
    if followGuide == 'false':
        iteData.append(False)
    else:
        iteData.append(True)
        
    return iteData

# parse default strategy
def parseStrategy(inPath) -> list:
    """Parse default strategy, save result to file, and return parsing status.
    
    Inputs:
    inPath: input FlexDR.cpp file path
    shapeCost: default shapeCost
    markerCost: default markerCost
    
    Return:
    Return list of strategy data.
    """
    # check file exists
    inputPath = os.path.join(inPath, "FlexDR.cpp")
    if not os.path.exists(inputPath):
        print("FlexDR.cpp for default setting doesn't exist in {}!".format(inputPath))
        return None
    # open, read, and extract coefficients from input file
    data = [] # temp coeff data
    count = 1
    with open(inputPath, 'r') as f:
        line = f.readline()
        while count < 1030:
            count = count + 1
            # check strategy() function
            if line == "static std::vector<FlexDR::SearchRepairArgs> strategy()\n":
                print("Start parsing iteration data!")
                # skip prefix lines
                line = f.readline()
                line = f.readline()
                line = f.readline()
                line = f.readline()
                count = count + 3
                # parse iteration settings
                while line != "  };\n":
                    # parse data
                    tempData = parseOneLine(line)
                    data.append(tempData)
                    # read next line
                    line = f.readline()
                    count = count + 1
                print("End parsing iteration data!")
            else:
                line = f.readline()

    return data

def parseStrategyAndLog(inPath, outPath) -> int:
    """Parse artificially generated strategy, save result to file, and return parsing status.
    
    Inputs:
    inPath: input FlexDR.cpp file path
    outPath: output result file path
    
    Return:
    Return 0 for correct result. 1 for Error
    """
    # check input file exists
    if not os.path.exists(inPath):
        print("Log file path {} doesn't exist!".format(inPath))
        return 1
    # traverse design folders
    coeffData = []
    outData = []
    for Folder in os.listdir(inPath):
        for folder in os.listdir(os.path.join(inPath, Folder)):
            # check FlexDR.cpp and 5_3_route.json file exists
            currentDir = os.path.join(inPath, Folder, folder)
            print("\n\n")
            print(currentDir)
            cppFilename = os.path.join(currentDir, "FlexDR.cpp")
            jsonFilename = os.path.join(currentDir, "base/5_3_route.json")
            if os.path.exists(cppFilename) and os.path.exists(jsonFilename):
                print("\tGet data from {}".format(currentDir))
                # get iteration settings
                strategy = parseStrategy(currentDir)
                coeffData.append(strategy)
                print(coeffData)
                # get outputs
                out = parseJSONFile(jsonFilename)
                outData.append(out)
                print(out)
    # check output path exists
    if not os.path.exists(outPath):
        print("Output path {} doesn't exist!".format(outPath))
        return 1
    # create and write to output file
    with open(os.path.join(outPath, "strategy.txt"), 'w') as f:
        for strategy in coeffData:
            for line in strategy:
                for element in line:
                    f.write("{},".format(element))
                f.write("@")
            f.write("\n")
    with open(os.path.join(outPath, "outputs.txt"), 'w') as f:
        for line in outData:
            for element in line:
                f.write("{},".format(element))
            f.write("\n")
    return 0

