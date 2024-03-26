import numpy as np
from utils import *
from env import DREnv
from modelOps import *
import matplotlib
import torch
from itertools import count

# set up matplotlib
is_ipython = 'inline' in matplotlib.get_backend()
# if is_ipython:
#     from IPython import display

plt.ion()

if __name__ == "__main__":
    ########## initialization ##########
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
    envCount = -1       # env status flag in python code

    # TODO1: initialization
    initFile = "./data/initFile.txt"
    dumpFile = "./data/dumpFile.txt"
    modelDir = "../../../../model/"             # model output directory
    trainDataDir = "./data/trainDataFile.txt"   # save memory info

    # other settings
    DRCMAX = 1e6

    # initialize environment
    actions = None
    window_size = 2
    input_size = 9
    env = DREnv(actions, window_size=window_size, input_size=input_size, actionFile=initFile)

    # Get the number of state observations
    observation, info = env.reset()
    n_observations = len(observation)

    print("Start python code!")
    # Get number of actions from gym action space
    n_actions = env.action_space.n
    seq_len = n_observations
    output_size = n_actions

    # instantiate model and memory
    policy_net, target_net, optimizer, criterion, num_episodes = loadModel(input_size, seq_len, n_actions, output_size)
    memory = ReplayMemory(1000)

    # settings for selecting actions
    steps_done = 0
    # create actions for network input
    df_actions = env.actions.copy()
    df_actions.loc[:, 'drc'] = np.ones(len(df_actions)) * -1
    actions = df_actions.to_numpy()
    # settings for plotting training process
    episode_durations = []

    action = None
    state = None
    ########## initialization ##########

    ########## create dump file ##########
    # create dump file
    if not os.path.exists(dumpFile):
        with open(dumpFile, 'w') as file:
            pass
    ########## create dump file ##########
        
    ########## training loop ##########
    for i_episode in range(num_episodes):
        # Initialize the environment and get its state
        state, info = env.reset()
        for t in count():
            action, state, reward, terminated, truncated, info, endPython = getActionFromNetwork(env, dumpFile, policy_net)
            done = terminated or truncated or endPython

            if terminated:
                next_state = None
            else:
                next_state = observation

            # Store the transition in memory
            memory.push(state, action, next_state, reward)

            # Move to the next state
            state = next_state

            # Perform one step of the optimization (on the policy network)
            policy_net, target_net = optimize_model(memory, actions, policy_net, target_net, optimizer, criterion)

            # Soft update of the target network's weights
            # θ′ ← τ θ + (1 −τ )θ′
            target_net_state_dict = target_net.state_dict()
            policy_net_state_dict = policy_net.state_dict()
            for key in policy_net_state_dict:
                target_net_state_dict[key] = policy_net_state_dict[key]*TAU + target_net_state_dict[key]*(1-TAU)
            target_net.load_state_dict(target_net_state_dict)

            if done:
                saveWeightedModel(policy_net, modelDir, agentCount)
                saveMemory(memory, trainDataDir)
                episode_durations.append(t + 1)
                plot_durations()
                break

    print('Complete')
    plot_durations(show_result=True)
    plt.ioff()
    plt.show()

    ########## training loop ##########

    # # test loop
    # count = 0
    # # start trial
    # env.reset()
    # action = select_test_action(observation, policy_net)
    # observation, reward, terminated, truncated, info = env.step(action)
    # while not(terminated or truncated):
    #     print(f"iteration {count} action {action} DRC = {int(observation[-1, -1] * 1e6)}\n")
    #     # start next iteration
    #     count += 1
    #     action = select_test_action(observation, policy_net)
    #     observation, reward, terminated, truncated, info = env.step(action)
        
    # print(f"iteration {count} action {action} DRC = {int(observation[-1, -1] * 1e6)}\n")
    # env.close()