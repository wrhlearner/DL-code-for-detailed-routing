import numpy as np
from utils import *
import modelOps

if __name__ == "__main__":
    # TODO1: initialization
    initFile, dumpFile, modelDir = getSettings()
    agentCount = -1   # agent status flag

    print("Start python code!")
    actionSpace = init(initFile)
    model = loadModel(modelDir)
    action = None
    state = None

    # loop for different designs
    while True:
        # TODO2: get action by wrapping up DQN network. update dump file
        action = getActionFromNetwork(actionSpace, dumpFile, model)
        
        # TODO3: get state and calculate reward
        state = getState(dumpFile)

        # loop for the same design
        while state:
            # TODO4: update network
            updateNetwork(state, model)

            # TODO5: repeat until terminated
            action = getActionFromNetwork(actionSpace, dumpFile, model)
            state = getState(dumpFile)
    
        # TODO6: save updated network
        saveWeightedModel(model, agentCount)