import os
import modelOps

def init(filename):
    """get action space"""
    actionSpace = None

    return actionSpace

def loadModel(dir):
    """Tasks:
    1. load model
    2. create training settings
    """
    model = None

    return model

def getActionFromNetwork(actionSpace, filename, model):
    """Tasks: 
    1. get action from network
    2. update dump file
    """
    action = None

    return action

def getState(filename):
    """get state from dump file"""
    state = None

    return state

def updateNetwork(state, model):
    """update policy network. wrap up training code here"""
    pass

def saveWeightedModel(model, count):
    """save updated model with a new name"""
    pass

def getSettings():
    """get training/testing settings"""
    
    initFile = input("Input init file dir. Press Enter for default settings: ")
    dumpFile = input("Input dump file dir. Press Enter for default settings: ")
    modelname = input("Input model name. Press Enter for default settings: ")

    end = False

    if initFile is None:
        initFile = "../data/initFile.txt"
    elif os.path.isdir(initFile):
        initFile = os.path.join(initFile, "initFile.txt")
    else:
        print(f"Invalid init file dir {initFile}!")
        end = True
    
    if dumpFile is None:
        dumpFile = "../data/dumpFile.txt"
    elif os.path.isdir(dumpFile):
        dumpFile = os.path.join(dumpFile, "dumpFile.txt")
    else:
        print(f"Invalid dump file dir {dumpFile}!")
        end = True

    if not os.path.isfile(modelname):
        print(f"Invalid model name {modelname}!")
        end = True

    if end:
        exit()
                
    return initFile, dumpFile, modelname
