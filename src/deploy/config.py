import matplotlib
import torch

# set up matplotlib
is_ipython = 'inline' in matplotlib.get_backend()

# training settings
BATCH_SIZE = 1
GAMMA = 0.99
EPS_START = 0.9
EPS_END = 0.05
EPS_DECAY = 1000
TAU = 0.005
LR = 1e-4
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
if torch.cuda.is_available():
    num_episodes = 150
else:
    num_episodes = 50

# status variable
agentCount = -1     # agent status flag in dump file

# TODO1: initialization
initFile = "/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/data/initFile.txt"
dumpFile = "/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/data/dumpFile.txt"
modelDir = "/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/model/"                                 # model output directory
trainDataDir = "/home/jborg/Data/Research/DLPnR/DL-code-for-detailed-routing/src/deploy/data/trainDataFile.txt"      # save memory info

# memory settings
MEMSIZE = 1000

# environment settings
DRCMAX = 1e6

# initialize environment
window_size = 2
input_size = 9

# settings for selecting actions
steps_done = 0
# settings for plotting training process
episode_durations = []