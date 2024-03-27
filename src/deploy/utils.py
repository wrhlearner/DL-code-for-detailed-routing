# basics
import os
import time
from collections import namedtuple, deque
import random
import math
import matplotlib.pyplot as plt
# torch related
import torch
# scientific computing
import numpy as np
# self defined
import config


def init(filename):
    """get action space"""
    actionSpace = []
    action = []

    # wait if init file doesn't exists
    while(not os.path.exists(filename)):
        time.sleep(1)        
    # read and parse action space file if init file does exist
    with open(filename, 'r') as file:
        while True:
            action = file.readline()
            if action:
                # parse actions
                action = [int(action.split(' ')[0]),
                          float(action.split(' ')[1]),
                          float(action.split(' ')[2])]
                actionSpace.append(action)
            else:   # line is empty
                break

    return actionSpace

def getActionFromNetwork(env, filename, model, actions):
    """Tasks: 
    1. get action from network
    2. update dump file: 
        2.1 increase agentCount value by 1
        2.2 write action to dump file
    3. wait and parse dump file. Get DRC value
    4. update env
    5. increase envCount or end python code if envCount in dump file == -1
    """
    action = None
    state = None
    reward = None
    terminated = None
    truncated = None
    info = None
    endPython = False

    # task 1: get action from network
    action = select_action(state, model, env, actions)
    actionValue = action.item()

    # task 2: update dump file
    lines = None
    with open(config.dumpFile, "r") as file:
        # 2.1 increase agentCount value by 1
        lines = file.readlines()
        # update dump file with increased agentCount value and action
        newFirstLine = str(config.agentCount + 1) + " " + str(actionValue)
        config.agentCount = config.agentCount + 1
        if lines:
            lines.insert(1, newFirstLine)
        else:
            lines = [newFirstLine]

    with open(config.dumpFile, 'w') as file:
        # 2.2 write action to dump file
        file.writelines(lines)

    # task 3: wait and parse dump file. Get DRC value
    lastTime = os.path.getmtime(config.dumpFile)
    while True:
        curTime = os.path.getmtime(config.dumpFile)
        if curTime != lastTime:
            # parse dump file for updated DRC value
            with open(config.dumpFile, "r") as file:
                line = file.readline()
            _, _, fileEnvCount, drc = [int(x) for x in line.split(" ")]
            break
        time.sleep(1)
    # task 4: update env
    # get updated state from environment
    state, reward, terminated, truncated, info = env.step(actionValue, drc)
    reward = torch.tensor([reward], device=config.device)
    # task 5: increase envCount or end python code if envCount in dump file == -1
    if fileEnvCount != -1:
        # increase envCount
        config.envCount = config.envCount + 1
    else:
        # end python
        endPython = True

    return action, state, reward, terminated, truncated, info, endPython

def saveWeightedModel(model, modelDir, count, modelname="DQN_FC_"):
    """save updated model with a new name"""
    # converting to Torch Script via annotation
    scriptModule = torch.jit.script(model)
    # serialize model to a file
    modelName = modelName + str(count) + ".pt"
    modelpath = os.path.join(modelDir, modelName)
    scriptModule.save(modelpath)

# a data structure containing several named elements
Transition = namedtuple('Transition', ('state', 'action', 'next_state', 'reward'))

class ReplayMemory(object):
    def __init__(self, capacity):
        self.memory = deque([], maxlen=capacity)

    def push(self, *args):
        """Save a transition"""
        self.memory.append(Transition(*args))

    def sample(self, batch_size):
        return random.sample(self.memory, batch_size)

    def __len__(self):
        return len(self.memory)
    
def select_action(state, policy_net, env, actions):
    sample = random.random()
    eps_threshold = config.EPS_END + (config.EPS_START - config.EPS_END) * math.exp(-1. * config.steps_done / config.EPS_DECAY)
    config.steps_done += 1
    if sample > eps_threshold:
        # select action according to policy
        with torch.no_grad():
            # t.min(0) will return the smallest drc value of the tensor returned by the policy network.
            # we pick action with the smallest expected DRC value.
            # create network input
            # inputs shape torch.Size([67, 9])
            inputs = np.vstack((state, actions))
            inputs = torch.from_numpy(inputs)
            inputs = inputs.to(config.device, dtype=torch.float32)
            # select the action with minimum DRC output as optimum action
            return policy_net(inputs).min(0).indices.view(1, 1)
    else:
        # select action randomly from action space
        return torch.tensor([[random.randint(0, env.n_actions - 1)]], device=config.device, dtype=torch.long)
    
def select_test_action(state, model, actions):
    # select action according to policy
    with torch.no_grad():
        inputs = np.vstack((state, actions))
        inputs = torch.from_numpy(inputs)
        inputs = inputs.to(config.device, dtype=torch.float32)
        # select the action with minimum DRC output as optimum action
        action = model(inputs).min(0).indices.view(1, 1)
        return action.item()

def plot_durations(show_result=False):
    if config.is_ipython:
        from IPython import display
    
    plt.figure(1)
    durations_t = torch.tensor(config.episode_durations, dtype=torch.float)
    if show_result:
        plt.title('Result')
    else:
        plt.clf()
        plt.title('Training...')
    plt.xlabel('Episode')
    plt.ylabel('Duration')
    plt.plot(durations_t.numpy())
    # Take 100 episode averages and plot them too
    if len(durations_t) >= 100:
        means = durations_t.unfold(0, 100, 1).mean(1).view(-1)
        means = torch.cat((torch.zeros(99), means))
        plt.plot(means.numpy())

    plt.pause(0.001)  # pause a bit so that plots are updated
    if config.is_ipython:
        if not show_result:
            display.display(plt.gcf())
            display.clear_output(wait=True)
        else:
            display.display(plt.gcf())

def saveMemory(memory, filename):
    """save memory data into a file for future training/testing"""
    with open(filename, '') as file:
        for item in memory:
            file.write(str(item) + '\n')
