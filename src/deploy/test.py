from utils import init, loadModel, getActionFromNetwork, getState, updateNetwork, saveWeightedModel, getSettings

def test(initFile, dumpFile, modelDir):
    """train model online"""

    envCount = None     # environment status flag
    agentCount = None   # agent status flag

    print("Start python code!")
    actionSpace = init(initFile)
    model = loadModel(modelDir)
    action = None
    state = None

    # loop for different designs
    while True:
        # TODO1: get action by wrapping up DQN network. update dump file
        action = getActionFromNetwork(actionSpace, dumpFile, model)
        
        # TODO2: get state and calculate reward
        state = getState(dumpFile)

        # loop for the same design
        while state:
            # TODO3: repeat until terminated
            action = getActionFromNetwork(actionSpace, dumpFile, model)
            state = getState(dumpFile)


if __name__ == "__main__":
    
    # get training settings
    initFile, dumpFile, modelDir = getSettings()

    test(initFile=initFile, dumpFile=dumpFile, modelDir=modelDir)