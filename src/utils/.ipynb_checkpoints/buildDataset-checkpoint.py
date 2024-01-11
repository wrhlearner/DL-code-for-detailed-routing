import os
import torch
import random
import pandas as pd

def parseRawData(strategy: str, output: str, csvfile: str, default: bool=True, createCSV: bool=False) -> pd.core.frame.DataFrame:
    """Extract strategy and DRC values, save them into a csv file"""
    if createCSV:
        dataset = []
        coeff = []
        # parse strategy
        with open(strategy, 'r') as f:
            data = f.readlines()
        if default:
            # default strategy, only 1
            coeff = data[0].split('@')[:-1]
            coeff = [[float(value) if i < 7 
                      else value for i, value in enumerate(x.split(',')[:-1])] 
                     for x in coeff]
        else:
            # artificially generated strategies, multiple
            pass
        # parse DRC values and form dataset
        with open(output, 'r') as f:
            data = f.readlines()
        for i, value in enumerate(data):
            # for each design, combine them with strategy settings
            drc = [int(x) for x in value.split(',')[:-1]]
            drclen = len(drc)
            # print(f"drc: {drc}\ndrclen: {drclen}")
            if default:
                for j, seq in enumerate(coeff):
                    # for each DRC sequence, combine them with drc values
                    design = [i]
                    for x in seq:
                        design.append(x)
                    if j < drclen:
                        design.append(drc[j])
                    else:
                        design.append(0)
                    dataset.append(design)
            else:
                pass
        # save data to csv
        columns = ['design', 'size', 'offset', 'mazeEndIter', 
                   'DRCCost', 'MarkerCost', 'FixedShapeCost', 
                   'Decay', 'ripupMode', 'followGuide', 'DRC']
        df = pd.DataFrame(dataset, columns=columns)
        df.to_csv(csvfile, index=False)
    else:
        df = pd.read_csv(csvfile)
        
    return df

def getDefaultData(defaultStrategy, defaultOutput) -> list:
    """Wrap up default strategies and outputs to generate an input list"""
    dataset = []
    defaultInputSet = []
    inputset = []
#     coef = {'size' : None, 'offset' : None, 'mazeEndIter' : None, 'MarkerCost' : None, 
#              'FixedShapeCost' : None, 'Decay' : None, 'ripupMode': None, 'followGuide' : None}
    # parse default strategy and output
    # check input file exists
    if not os.path.exists(defaultStrategy):
        print("Default strategy {} doesn't exist!".format(defaultStrategy))
        return []
    # parse default strategy
    with open(defaultStrategy, 'r') as f:
        line = f.readline()
        temp = line.split("@")
        for i in range(len(temp) - 1):
            element = temp[i]
            coef = dict([])
            # parse 1 strategy
            result = element.split(",")[:-1]
            coef['size'] = int(result[0])
            coef['offset'] = int(result[1])
            coef['mazeEndIter'] = int(result[2])
            coef['MarkerCost'] = int(result[3])
            coef['FixedShapeCost'] = int(result[4])
            coef['Decay'] = float(result[5])
            if result[6] == 'ALL':
                coef['ripupMode'] = 0
            elif result[6] == 'DRC':
                coef['ripupMode'] = 1
            else:
                coef['ripupMode'] = 2
            if result[7] == 'True':
                coef['followGuide'] = 1
            elif result[7] == 'False':
                coef['followGuide'] = 0
            # add result to list
            defaultInputSet.append(coef)
    # check input file exists
    if not os.path.exists(defaultOutput):
        print("Default strategy output {} doesn't exist!".format(defaultOutput))
        return []
    # parse default strategy outputs
    with open(defaultOutput, 'r') as f:
        allLines = f.readlines()
        for line in allLines:
            result = line.split(',')[:-1]
            datapoint = []
            for i, value in enumerate(result):
                datapoint.append([defaultInputSet[i], int(value)])
            # add datapoint into dataset
            dataset.append(datapoint)
            
    return dataset
    
def getData(strategy, output, defaultStrategy, defaultOutput) -> list:
    """Wrap up strategies and outputs to generate an input list"""
    dataset = []
    defaultInputSet = []
    inputset = []
#     coef = {'size' : None, 'offset' : None, 'mazeEndIter' : None, 'MarkerCost' : None, 
#              'FixedShapeCost' : None, 'Decay' : None, 'ripupMode': None, 'followGuide' : None}
    # parse default strategy and output
    # check input file exists
    if not os.path.exists(defaultStrategy):
        print("Default strategy {} doesn't exist!".format(defaultStrategy))
        return []
    # parse default strategy
    with open(defaultStrategy, 'r') as f:
        line = f.readline()
        temp = line.split("@")
        for i in range(len(temp) - 1):
            element = temp[i]
            coef = dict([])
            # parse 1 strategy
            result = element.split(",")[:-1]
            coef['size'] = int(result[0])
            coef['offset'] = int(result[1])
            coef['mazeEndIter'] = int(result[2])
            coef['MarkerCost'] = int(result[3])
            coef['FixedShapeCost'] = int(result[4])
            coef['Decay'] = float(result[5])
            if result[6] == 'ALL':
                coef['ripupMode'] = 0
            elif result[6] == 'DRC':
                coef['ripupMode'] = 1
            else:
                coef['ripupMode'] = 2
            if result[7] == 'True':
                coef['followGuide'] = 1
            elif result[7] == 'False':
                coef['followGuide'] = 0
            # add result to list
            defaultInputSet.append(coef)
    # check input file exists
    if not os.path.exists(defaultOutput):
        print("Default strategy output {} doesn't exist!".format(defaultOutput))
        return []
    # parse default strategy outputs
    with open(defaultOutput, 'r') as f:
        allLines = f.readlines()
        for line in allLines:
            result = line.split(',')[:-1]
            datapoint = []
            for i, value in enumerate(result):
                datapoint.append([defaultInputSet[i], int(value)])
            # add datapoint into dataset
            dataset.append(datapoint)
    # parse strategy and output
    # check input file exists
    if not os.path.exists(strategy):
        print("Strategy {} doesn't exist!".format(strategy))
        return []
    # parse strategy
    with open(strategy, 'r') as f:
        allLine = f.readlines()
        for line in allLine:
            temp = line.split("@")
            instrategy = []
            for i in range(len(temp) - 1):
                element = temp[i]
                coef = dict([])
                # parse 1 strategy
                result = element.split(",")[:-1]
                coef['size'] = int(result[0])
                coef['offset'] = int(result[1])
                coef['mazeEndIter'] = int(result[2])
                coef['MarkerCost'] = int(result[3])
                coef['FixedShapeCost'] = int(result[4])
                coef['Decay'] = float(result[5])
                if result[6] == 'ALL':
                    coef['ripupMode'] = 0
                elif result[6] == 'DRC':
                    coef['ripupMode'] = 1
                else:
                    coef['ripupMode'] = 2
                if result[7] == 'True':
                    coef['followGuide'] = 1
                elif result[7] == 'False':
                    coef['followGuide'] = 0
                # add result to list
                instrategy.append(coef)
            inputset.append(instrategy)
    # check input file exists
    if not os.path.exists(output):
        print("Output {} doesn't exist!".format(output))
        return []
    # parse strategy outputs
    with open(output, 'r') as f:
        allLines = f.readlines()
        for index, line in enumerate(allLines):
            result = line.split(',')[:-1]
            datapoint = []
            for i, value in enumerate(result):
                if i > 63:
                    break
#                 print("design {} strategy {}".format(index + 1, i + 1))
                datapoint.append([inputset[index][i], int(value)])
            # add datapoint into dataset
            dataset.append(datapoint)
            
    return dataset

def datasetToTensor(dataset, dim=9):
    """Turning the input to tensor to construct dataset"""
    Dataset = []
    for element in dataset:
        drc = element[0][1]
        datain = []
        for i, it in enumerate(element):
            coef = it[0]
            datapoint = list(coef.values())
            datapoint.append(drc)
            datain.append(datapoint)
            drc = it[1]
        Dataset.append(torch.Tensor(datain).reshape(-1, 1, dim))
    
    return Dataset

def originalDatasetToTensor(dataset, dim=9):
    """Turning the input to tensor to construct dataset"""
    Dataset = []
    for element in dataset:
        datain = []
        for i, it in enumerate(element):
            coef = it[0]
            drc = it[1]
            datapoint = list(coef.values())
            datapoint.append(drc)
            datain.append(datapoint)
        Dataset.append(torch.Tensor(datain).reshape(-1, 1, dim))
    
    return Dataset

def LSTMdatasetToTensor(dataset, dim=9):
    """Turning the input to tensor to construct dataset"""
    Dataset = []
    Label = []
    for element in dataset:
        drc = element[0][1]
        datain = []
        dataout = torch.zeros(len(element), 1, 1)
        for i, it in enumerate(element):
            coef = it[0]
            datapoint = list(coef.values())
            datapoint.append(drc)
            datain.append(datapoint)
            drc = it[1]
            dataout[i][0][0] = drc
        Dataset.append(torch.Tensor(datain).reshape(-1, 1, dim))
        Label.append(dataout)
        
    return Dataset, Label


def splitDataset(X, trainSize = 0.7):
    """split dataset into training set and test set with a set ratio. Randomly generate."""
    # generate a list of random integer number as indices to the datasets
    trainIndex = random.sample(list(range(len(X))), int(trainSize*len(X)))
    testIndex = [x for x in list(range(len(X))) if x not in trainIndex]
    # generate datasets
    trainningSet = []
    testSet = []
    for i in trainIndex:
        trainningSet.append(X[i])
    for i in testIndex:
        testSet.append(X[i])

    return trainningSet, testSet

def LSTMsplitDataset(X, Y, trainSize = 0.7):
    """split dataset into training set and test set with a set ratio. Randomly generate."""
    # generate a list of random integer number as indices to the datasets
    trainIndex = random.sample(list(range(len(X))), int(trainSize*len(X)))
    testIndex = [x for x in list(range(len(X))) if x not in trainIndex]
    # generate datasets
    X_train = []
    Y_train = []
    X_test = []
    Y_test = []
    for i in trainIndex:
        X_train.append(X[i])
        Y_train.append(Y[i])
    for i in testIndex:
        X_test.append(X[i])
        Y_test.append(Y[i])

    return X_train, Y_train, X_test, Y_test