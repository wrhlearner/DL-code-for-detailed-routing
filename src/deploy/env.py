import gymnasium as gym
import numpy as np
import pandas as pd
from sklearn.preprocessing import MinMaxScaler
import config

class DREnv(gym.Env):
    # draw bar chart and line curve for visuliazation
    # metadata = ("render_modes": [])

    def __init__(self, actions, window_size=2, render_mode=None, drcmax=config.DRCMAX, input_size=9, actionFile=config.initFile):
        self.size = drcmax
        self.maxdrc = None
        self.window_size = window_size
        self.input_size = input_size
        self._index = -1
        if actions is None:
            # index: (size, offset, mazeEndIter, DRCCost, MarkerCost, FixedShapeCost, Decay, ripupMode, followGuide)
            # We have 65 actions, corresponding to default DRC sequences
            # shapeCost = 8
            # MARKERCOST = 32
            # ripupMode: ALL->0, DRC->0.5, NEARDRC->1
            # followGuide: True->1, False->0
            # TODO: set actions by specification. Use IO to read and parse actions in the future
            actions = [[7,  0,  3,      8,       0,       8, 0.950, 0    ,  1],
                        [7, -2,  3,      8,       8,       8, 0.950, 0    ,  1],
                        [7, -5,  3,      8,       8,       8, 0.950, 0    ,  1],
                        [7,  0,  8,      8,      32,   2 * 8, 0.950, 0.5    , 0],
                        [7, -1,  8,      8,      32,   2 * 8, 0.950, 0.5    , 0],
                        [7, -2,  8,      8,      32,   2 * 8, 0.950, 0.5    , 0],
                        [7, -3,  8,      8,      32,   2 * 8, 0.950, 0.5    , 0],
                        [7, -4,  8,      8,      32,   2 * 8, 0.950, 0.5    , 0],
                        [7, -5,  8,      8,      32,   2 * 8, 0.950, 0.5    , 0],
                        [7, -6,  8,      8,      32,   2 * 8, 0.950, 0.5    , 0],
                        [7,  0,  8,  2 * 8,      32,   3 * 8, 0.950, 0.5    , 0],
                        [7, -1,  8,  2 * 8,      32,   3 * 8, 0.950, 0.5    , 0],
                        [7, -2,  8,  2 * 8,      32,   3 * 8, 0.950, 0.5    , 0],
                        [7, -3,  8,  2 * 8,      32,   3 * 8, 0.950, 0.5    , 0],
                        [7, -4,  8,  2 * 8,      32,   3 * 8, 0.950, 0.5    , 0],
                        [7, -5,  8,  2 * 8,      32,   4 * 8, 0.950, 0.5    , 0],
                        [7, -6,  8,  2 * 8,      32,   4 * 8, 0.950, 0.5    , 0],
                        [7, -3,  8,      8,      32,   4 * 8, 0.950, 0    , 0],
                        [7,  0,  8,  4 * 8,      32,   4 * 8, 0.950, 0.5    , 0],
                        [7, -1,  8,  4 * 8,      32,   4 * 8, 0.950, 0.5    , 0],
                        [7, -2,  8,  4 * 8,      32,  10 * 8, 0.950, 0.5    , 0],
                        [7, -3,  8,  4 * 8,      32,  10 * 8, 0.950, 0.5    , 0],
                        [7, -4,  8,  4 * 8,      32,  10 * 8, 0.950, 0.5    , 0],
                        [7, -5,  8,      8,      32,  10 * 8, 0.950, 1, 0],
                        [7, -6,  8,  4 * 8,      32,  10 * 8, 0.950, 0.5    , 0],
                        [5, -2,  8,      8,      32,  10 * 8, 0.950, 0    , 0],
                        [7,  0,  8,  8 * 8,  2 * 32,  10 * 8, 0.950, 0.5    , 0],
                        [7, -1,  8,  8 * 8,  2 * 32,  10 * 8, 0.950, 0.5    , 0],
                        [7, -2,  8,  8 * 8,  2 * 32,  10 * 8, 0.950, 0.5    , 0],
                        [7, -3,  8,  8 * 8,  2 * 32,  10 * 8, 0.950, 0.5    , 0],
                        [7, -4,  8,      8,      32,  50 * 8, 0.950, 1, 0],
                        [7, -5,  8,  8 * 8,  2 * 32,  50 * 8, 0.950, 0.5    , 0],
                        [7, -6,  8,  8 * 8,  2 * 32,  50 * 8, 0.950, 0.5    , 0],
                        [3, -1,  8,      8,      32,  50 * 8, 0.950, 0    , 0],
                        [7,  0,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 0.5    , 0],
                        [7, -1,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 0.5    , 0],
                        [7, -2,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 0.5    , 0],
                        [7, -3,  8,      8,      32,  50 * 8, 0.950, 1, 0],
                        [7, -4,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 0.5    , 0],
                        [7, -5,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 0.5    , 0],
                        [7, -6,  8, 16 * 8,  4 * 32, 100 * 8, 0.990, 0.5    , 0],
                        [3, -2,  8,      8,      32, 100 * 8, 0.990, 0    , 0],
                        [7,  0, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 0.5    , 0],
                        [7, -1, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 0.5    , 0],
                        [7, -2, 16,      8,      32, 100 * 8, 0.990, 1, 0],
                        [7, -3, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 0.5    , 0],
                        [7, -4, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 0.5    , 0],
                        [7, -5, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 0.5    , 0],
                        [7, -6, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 0.5    , 0],
                        [3, -0,  8,      8,      32, 100 * 8, 0.990, 0    , 0],
                        [7,  0, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -1, 32,      8,      32, 100 * 8, 0.999, 1, 0],
                        [7, -2, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -3, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -4, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -5, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -6, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [3, -1,  8,      8,      32, 100 * 8, 0.999, 0    , 0],
                        [7,  0, 64,      8,      32, 100 * 8, 0.999, 1, 0],
                        [7, -1, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -2, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -3, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -4, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -5, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 0.5    , 0],
                        [7, -6, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 0.5    , 0]]
        columns = ['size', 'offset', 'mazeEndIter', 'DRCCost', 'MarkerCost', 'FixedShapeCost', 'Decay', 'ripupMode', 'followGuide']
        self.actions = pd.DataFrame(actions, columns=columns)
        self.n_actions = self.actions.shape[0]
        # self.action_space = spaces.Discrete(self.actions.shape[0])
        # Observations are boxes containing the historical DRC sequence settings and DRC values
        self.observations = None
        # self.observation_space = spaces.Box(low=-1, high=1, shape=(self.window_size, self.input_size), dtype=float)

    def _preprocessing(self):
        if 'size' in self.actions.columns:
            # action space preprocessing
            # drop columns: size
            self.actions = self.actions.drop(columns=['size'])
    
            # normalize coefficients
            for column in ['offset', 'mazeEndIter', 'DRCCost', 'MarkerCost', 'FixedShapeCost', 'Decay']:
                sc = MinMaxScaler(feature_range=(-1, 1))
                self.actions[column] = sc.fit_transform(self.actions[column].to_numpy().reshape(-1, 1))
        
        # observation space preprocessing
        # padding historical data
        self.observations = np.ones((self.window_size, self.input_size), dtype=float) * -1
        # generate drc value before 0th iteration
        self.maxdrc = self.np_random.integers(1, self.size, size=1, dtype=int)[0]
        self.observations[-1, -1] = 1
        # parameters initialization
        self._index = -1
        self._reward = -1   # reward starting value
        self._curr_drc = 1

    def _action_to_setting(self, action):
        """
        The following dictionary maps abstract actions from `self.action_space` to
        the setting we will use for next iteration.
        I.e. 0 corresponds to the 0th default setting
        """
        # return selected value
        return list(self.actions.iloc[action, :])

    def _get_obs(self, action, drc):
        """get DRC value for current iteration
        Use DRC value from external input
        """
        if action:
            # update DRC value
            self._curr_drc = drc/self.size
            # generate observation for current iteration
            setting = self._action_to_setting(action)
            line = np.append(setting, self._curr_drc)
            # update the whole observation
            self.observations = np.r_[self.observations, [line]]
            self.observations = np.delete(self.observations, 0, 0)
            
        return self.observations

    def _get_info(self):
        """"provide index for current iteration"""
        return {
            "iteration index": self._index
        }

    def reset(self, seed=None, options=None):
        # We need the following line to seed self.np_random
        super().reset(seed=seed)

        self._preprocessing()
        observation = self._get_obs(None, config.DRCMAX)
        info = self._get_info()
    
        # if self.render_mode == "human":
        #     self._render_frame()
    
        return observation, info

    def step(self, action, drc):
        # Map the action (element of {0,1,2,3}) to the setting we use
        reward = self._reward
        
        # update index
        self._index += 1

        # update observation
        observation = self._get_obs(action, drc)

        # update truncated
        if self._index == 64 and self._curr_drc != 0:
            # punish truncated case
            truncated = True
            reward = -255    # -255 is a large value without detailed thought
        else:
            truncated = False

        # An episode is done iff current DRC value goes to zero
        if self._curr_drc:
            terminated = False
        else:
            terminated = True
            # udpate reward: encourage finishing detailed routing using less iterations
            reward = 0

        # update info
        info = self._get_info()
    
        # if self.render_mode == "human":
        #     self._render_frame()
    
        return observation, reward, terminated, truncated, info