# model definition and function definitions
import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F
import numpy as np


class DQN(nn.Module):

    def __init__(self, input_size, output_size):
        super(DQN, self).__init__()
        
        self.layer1 = nn.Linear(int(input_size), 128)
        self.layer2 = nn.Linear(128, 128)
        self.layer3 = nn.Linear(128, int(output_size))

    # Called with either one element to determine next action, or a batch
    # during optimization. Returns tensor([[left0exp,right0exp]...]).
    def forward(self, x):
        x = torch.flatten(x)
        x = F.relu(self.layer1(x))
        x = F.relu(self.layer2(x))
        
        return self.layer3(x)

def loadModel(input_size, seq_len, n_actions, output_size):
    """Tasks:
    1. load model
    2. create training settings
    """
    global device
    global LR

    # load model
    policy_net = DQN(input_size=input_size * (seq_len + n_actions), output_size=output_size).to(device)
    target_net = DQN(input_size=input_size * (seq_len + n_actions), output_size=output_size).to(device)
    target_net.load_state_dict(policy_net.state_dict())

    # create training settings
    optimizer = optim.AdamW(policy_net.parameters(), lr=LR, amsgrad=True)
    criterion = nn.SmoothL1Loss()

    if torch.cuda.is_available():
        num_episodes = 150
    else:
        num_episodes = 50
    return policy_net, target_net, optimizer, criterion, num_episodes

def optimize_model(memory, actions, policy_net, target_net, optimizer, criterion):
    global BATCH_SIZE
    global GAMMA

    if len(memory) < BATCH_SIZE:
        return
    transitions = memory.sample(BATCH_SIZE)
    # Transpose the batch (see https://stackoverflow.com/a/19343/3343043 for
    # detailed explanation). This converts batch-array of Transitions
    # to Transition of batch-arrays.
    batch = transitions[0]

    if batch.next_state is not None:
        # Compute a mask of non-final states and concatenate the batch elements
        # (a final state would've been the one after which simulation ended)
        action_batch = torch.Tensor(batch.action)
        reward_batch = torch.Tensor(batch.reward)
    
        # create network input
        # inputs shape torch.Size([67, 9])
        state_inputs = np.vstack((batch.state, actions))
        state_inputs = torch.from_numpy(state_inputs)
        state_inputs = state_inputs.to(device, dtype=torch.float32)

        next_state_inputs = np.vstack((batch.next_state, actions))
        next_state_inputs = torch.from_numpy(next_state_inputs)
        next_state_inputs = next_state_inputs.to(device, dtype=torch.float32)
        
        # Compute Q(s_t, a) - the model computes Q(s_t), then we select the
        # columns of actions taken. These are the actions which would've been taken
        # for each batch state according to policy_net
        # policy_net is the assumed-to-be perfect policy network
        state_action_values = policy_net(state_inputs)[action_batch]
    
        # Compute V(s_{t+1}) for all next states.
        # Expected values of actions for non_final_next_states are computed based
        # on the "older" target_net; selecting their best reward with max(1).values
        # This is merged based on the mask, such that we'll have either the expected
        # state value or 0 in case the state was final.
        # target_net is the approximation of the perfect policy network
        next_state_values = torch.zeros(BATCH_SIZE, device=device)
        with torch.no_grad():
            # TODO: change max to min to select DRC setting that yields minimum DRC values
            next_state_values[0] = target_net(next_state_inputs).min(0).values
        # Compute the expected Q values
        expected_state_action_values = (next_state_values * GAMMA) + reward_batch
    
        # Compute Huber loss
        loss = criterion(state_action_values, expected_state_action_values.unsqueeze(1))
    
        # Optimize the model
        optimizer.zero_grad()
        loss.backward()
        # In-place gradient clipping
        torch.nn.utils.clip_grad_value_(policy_net.parameters(), 100)
        optimizer.step()

    return policy_net, target_net