import numpy as np
from utils import *
from env import DREnv
import modelOps

import torch

if __name__ == "__main__":
    # TODO1: initialization
    initFile = "./data/initFile.txt"
    dumpFile = "./data/dumpFile.txt"
    modelDir = "../../../../model/"     # model output directory
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # initialize environment
    actions = None
    window_size = 2
    input_size = 9
    env = DREnv(actions, window_size=window_size, input_size=input_size, actionFile=initFile)

    # training settings
    BATCH_SIZE = 1
    GAMMA = 0.99
    EPS_START = 0.9
    EPS_END = 0.05
    EPS_DECAY = 1000
    TAU = 0.005
    LR = 1e-4

    # Get the number of state observations
    state, info = env.reset()
    n_observations = len(state)

    agentCount = -1   # agent status flag

    print("Start python code!")
    # Get number of actions from gym action space
    n_actions = env.action_space.n
    seq_len = len(observation)
    output_size = n_actions

    policy_net, target_net, optimizer, criterion, num_episodes = loadModel(input_size, seq_len, n_actions, output_size, device)
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

# for i_episode in range(num_episodes):
#     # Initialize the environment and get its state
#     state, info = env.reset()
#     for t in count():
#         action = select_action(state)
#         observation, reward, terminated, truncated, info = env.step(action.item())
#         reward = torch.tensor([reward], device=device)
#         done = terminated or truncated

#         if terminated:
#             next_state = None
#         else:
#             next_state = observation

#         # Store the transition in memory
#         memory.push(state, action, next_state, reward)

#         # Move to the next state
#         state = next_state

#         # Perform one step of the optimization (on the policy network)
#         optimize_model()

#         # Soft update of the target network's weights
#         # θ′ ← τ θ + (1 −τ )θ′
#         target_net_state_dict = target_net.state_dict()
#         policy_net_state_dict = policy_net.state_dict()
#         for key in policy_net_state_dict:
#             target_net_state_dict[key] = policy_net_state_dict[key]*TAU + target_net_state_dict[key]*(1-TAU)
#         target_net.load_state_dict(target_net_state_dict)

#         if done:
#             episode_durations.append(t + 1)
#             plot_durations()
#             break

# print('Complete')
# plot_durations(show_result=True)
# plt.ioff()
# plt.show()

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
        saveWeightedModel(policy_net, modelDir, agentCount)

    # test loop
    count = 0
    # start trial
    env.reset()
    action = select_test_action(observation, policy_net)
    observation, reward, terminated, truncated, info = env.step(action)
    while not(terminated or truncated):
        print(f"iteration {count} action {action} DRC = {int(observation[-1, -1] * 1e6)}\n")
        # start next iteration
        count += 1
        action = select_test_action(observation, policy_net)
        observation, reward, terminated, truncated, info = env.step(action)
        
    print(f"iteration {count} action {action} DRC = {int(observation[-1, -1] * 1e6)}\n")
    env.close()