import os
import time
from collections import namedtuple, deque
import random
import matplotlib.pyplot as plt
import torch
import modelOps


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

def saveWeightedModel(model, modelDir, count):
    """save updated model with a new name"""
    # converting to Torch Script via annotation
    scriptModule = torch.jit.script(model)
    # serialize model to a file
    modelName = "DQN_FC_" + str(count) + ".pt"
    modelpath = os.path.join(modelDir, modelName)
    scriptModule.save(modelpath)

# a data structure containing several named elements
Transition = namedtuple('Transition',
                        ('state', 'action', 'next_state', 'reward'))

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
    
def select_action(state):
    global steps_done
    global actions
    sample = random.random()
    eps_threshold = EPS_END + (EPS_START - EPS_END) * \
        math.exp(-1. * steps_done / EPS_DECAY)
    steps_done += 1
    if sample > eps_threshold:
        # select action according to policy
        with torch.no_grad():
            # t.min(0) will return the smallest drc value of the tensor returned by the policy network.
            # we pick action with the smallest expected DRC value.
            # create network input
            # inputs shape torch.Size([67, 9])
            inputs = np.vstack((state, actions))
            inputs = torch.from_numpy(inputs)
            inputs = inputs.to(device, dtype=torch.float32)
            # select the action with minimum DRC output as optimum action
            return policy_net(inputs).min(0).indices.view(1, 1)
    else:
        # select action randomly from action space
        return torch.tensor([[env.action_space.sample()]], device=device, dtype=torch.long)
    
def select_test_action(state, model):
    global actions
    # select action according to policy
    with torch.no_grad():
        inputs = np.vstack((state, actions))
        inputs = torch.from_numpy(inputs)
        inputs = inputs.to(device, dtype=torch.float32)
        # select the action with minimum DRC output as optimum action
        action = model(inputs).min(0).indices.view(1, 1)
        return action.item()

def plot_durations(show_result=False):
    plt.figure(1)
    durations_t = torch.tensor(episode_durations, dtype=torch.float)
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
    if is_ipython:
        if not show_result:
            display.display(plt.gcf())
            display.clear_output(wait=True)
        else:
            display.display(plt.gcf())
