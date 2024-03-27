# basics
import os
import matplotlib
import matplotlib.pyplot as plt
from itertools import count
# scientific computing related
import numpy as np
# self defined files
from utils import ReplayMemory, getActionFromNetwork, saveWeightedModel, saveMemory, plot_durations
from env import DREnv
from modelOps import loadModel, optimize_model
import config


# set up matplotlib
if config.is_ipython:
    from IPython import display

plt.ion()

if __name__ == "__main__":
    ########## initialization ##########
    # initialize environment
    actions = None
    env = DREnv(actions, window_size=config.window_size, input_size=config.input_size, actionFile=config.initFile)

    # Get the number of state observations
    observation, info = env.reset()
    n_observations = len(observation)

    print(f"Start python code at {os.getcwd()}")
    # Get number of actions from gym action space
    n_actions = env.n_actions
    seq_len = n_observations
    output_size = n_actions

    # instantiate model and memory
    policy_net, target_net, optimizer, criterion, config.num_episodes = loadModel(config.input_size, seq_len, n_actions, output_size)
    memory = ReplayMemory(config.MEMSIZE)

    # create actions for network input
    df_actions = env.actions.copy()
    df_actions.loc[:, 'drc'] = np.ones(len(df_actions)) * -1
    actions = df_actions.to_numpy()

    action = None
    state = None
    ########## initialization ##########

    ########## create dump file ##########
    # create dump file
    print(f"Check dump file at {config.dumpFile}\n")
    if not os.path.exists(config.dumpFile):
        print(f"Dump file {config.dumpFile} doesn't exist!")
        with open(config.dumpFile, 'w') as file:
            print(f"Create dump file at {config.dumpFile}.")
    else:
        print(f"Dump file {config.dumpFile} exists!")
    ########## create dump file ##########
        
    ########## training loop ##########clear
    for i_episode in range(config.num_episodes):
        # Initialize the environment and get its state
        print(f"Start design {i_episode}")
        state, info = env.reset()
        for t in count():
            print(f"Start design {i_episode} iteration {t}")
            action, state, reward, terminated, truncated, info, endPython = getActionFromNetwork(env, config.dumpFile, policy_net, actions)
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
                target_net_state_dict[key] = policy_net_state_dict[key]*config.TAU + target_net_state_dict[key]*(1-config.TAU)
            target_net.load_state_dict(target_net_state_dict)

            if done:
                saveWeightedModel(policy_net, config.modelDir, config.agentCount)
                saveMemory(memory, config.trainDataDir)
                config.episode_durations.append(t + 1)
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