import numpy as np

import gymnasium as gym
from gymnasium import spaces

class DREnv(gym.Env):
    # draw bar chart and line curve for visuliazation
    # metadata = ("render_modes": [])

    def __init__(self, render_mode=None, drcmax=1e6):
        # Observations are boxes containing the DRC value of last and current iteration.
        # for env initialization, use [-1, 1e6] as initial observation
        self.size = drcmax
        
        self.observation_space = spaces.Box(low=-1, high=self.size, shape=(2,), dtype=int)

        # We have 65 actions, corresponding to default DRC sequences
        # shapeCost = 8
        # ripupMode: ALL->0, DRC->1, NEARDRC->2
        # followGuide: True->1, False->0
        # 32 = 32
        self.action_space = spaces.Discrete(65)

        """
        The following dictionary maps abstract actions from `self.action_space` to
        the setting we will use for next iteration.
        I.e. 0 corresponds to the 0th default setting
        """
        self._action_to_setting = {
            0: (7,  0,  3,      8,       0,       8, 0.950, 0    ,  1),
            1: (7, -2,  3,      8,       8,       8, 0.950, 0    ,  1),
            2: (7, -5,  3,      8,       8,       8, 0.950, 0    ,  1),
            3: (7,  0,  8,      8,      32,   2 * 8, 0.950, 1    , 0),
            4: (7, -1,  8,      8,      32,   2 * 8, 0.950, 1    , 0),
            5: (7, -2,  8,      8,      32,   2 * 8, 0.950, 1    , 0),
            6: (7, -3,  8,      8,      32,   2 * 8, 0.950, 1    , 0),
            7: (7, -4,  8,      8,      32,   2 * 8, 0.950, 1    , 0),
            8: (7, -5,  8,      8,      32,   2 * 8, 0.950, 1    , 0),
            9: (7, -6,  8,      8,      32,   2 * 8, 0.950, 1    , 0),
            10: (7,  0,  8,  2 * 8,      32,   3 * 8, 0.950, 1    , 0),
            11: (7, -1,  8,  2 * 8,      32,   3 * 8, 0.950, 1    , 0),
            12: (7, -2,  8,  2 * 8,      32,   3 * 8, 0.950, 1    , 0),
            13: (7, -3,  8,  2 * 8,      32,   3 * 8, 0.950, 1    , 0),
            14: (7, -4,  8,  2 * 8,      32,   3 * 8, 0.950, 1    , 0),
            15: (7, -5,  8,  2 * 8,      32,   4 * 8, 0.950, 1    , 0),
            16: (7, -6,  8,  2 * 8,      32,   4 * 8, 0.950, 1    , 0),
            17: (7, -3,  8,      8,      32,   4 * 8, 0.950, 0    , 0),
            18: (7,  0,  8,  4 * 8,      32,   4 * 8, 0.950, 1    , 0),
            19: (7, -1,  8,  4 * 8,      32,   4 * 8, 0.950, 1    , 0),
            20: (7, -2,  8,  4 * 8,      32,  10 * 8, 0.950, 1    , 0),
            21: (7, -3,  8,  4 * 8,      32,  10 * 8, 0.950, 1    , 0),
            22: (7, -4,  8,  4 * 8,      32,  10 * 8, 0.950, 1    , 0),
            23: (7, -5,  8,      8,      32,  10 * 8, 0.950, 2, 0),
            24: (7, -6,  8,  4 * 8,      32,  10 * 8, 0.950, 1    , 0),
            25: (5, -2,  8,      8,      32,  10 * 8, 0.950, 0    , 0),
            26: (7,  0,  8,  8 * 8,  2 * 32,  10 * 8, 0.950, 1    , 0),
            27: (7, -1,  8,  8 * 8,  2 * 32,  10 * 8, 0.950, 1    , 0),
            28: (7, -2,  8,  8 * 8,  2 * 32,  10 * 8, 0.950, 1    , 0),
            29: (7, -3,  8,  8 * 8,  2 * 32,  10 * 8, 0.950, 1    , 0),
            30: (7, -4,  8,      8,      32,  50 * 8, 0.950, 2, 0),
            31: (7, -5,  8,  8 * 8,  2 * 32,  50 * 8, 0.950, 1    , 0),
            32: (7, -6,  8,  8 * 8,  2 * 32,  50 * 8, 0.950, 1    , 0),
            33: (3, -1,  8,      8,      32,  50 * 8, 0.950, 0    , 0),
            34: (7,  0,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 1    , 0),
            35: (7, -1,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 1    , 0),
            36: (7, -2,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 1    , 0),
            37: (7, -3,  8,      8,      32,  50 * 8, 0.950, 2, 0),
            38: (7, -4,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 1    , 0),
            39: (7, -5,  8, 16 * 8,  4 * 32,  50 * 8, 0.950, 1    , 0),
            40: (7, -6,  8, 16 * 8,  4 * 32, 100 * 8, 0.990, 1    , 0),
            41: (3, -2,  8,      8,      32, 100 * 8, 0.990, 0    , 0),
            42: (7,  0, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 1    , 0),
            43: (7, -1, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 1    , 0),
            44: (7, -2, 16,      8,      32, 100 * 8, 0.990, 2, 0),
            45: (7, -3, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 1    , 0),
            46: (7, -4, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 1    , 0),
            47: (7, -5, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 1    , 0),
            48: (7, -6, 16, 16 * 8,  4 * 32, 100 * 8, 0.990, 1    , 0),
            49: (3, -0,  8,      8,      32, 100 * 8, 0.990, 0    , 0),
            50: (7,  0, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 1    , 0),
            51: (7, -1, 32,      8,      32, 100 * 8, 0.999, 2, 0),
            52: (7, -2, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 1    , 0),
            53: (7, -3, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 1    , 0),
            54: (7, -4, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 1    , 0),
            55: (7, -5, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 1    , 0),
            56: (7, -6, 32, 32 * 8,  8 * 32, 100 * 8, 0.999, 1    , 0),
            57: (3, -1,  8,      8,      32, 100 * 8, 0.999, 0    , 0),
            58: (7,  0, 64,      8,      32, 100 * 8, 0.999, 2, 0),
            59: (7, -1, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 1    , 0),
            60: (7, -2, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 1    , 0),
            61: (7, -3, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 1    , 0),
            62: (7, -4, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 1    , 0),
            63: (7, -5, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 1    , 0),
            64: (7, -6, 64, 64 * 8, 16 * 32, 100 * 8, 0.999, 1    , 0)
        }

        # assert render_mode is None or render_mode in self.metadata["render_modes"]
        # self.render_mode = render_mode

        # """
        # If human-rendering is used, `self.window` will be a reference
        # to the window that we draw to. `self.clock` will be a clock that is used
        # to ensure that the environment is rendered at the correct framerate in
        # human-mode. They will remain `None` until human-mode is used for the
        # first time.
        # """
        # self.window = None
        # self.clock = None

    def _get_obs(self):
        """get DRC value for current iteration"""
        return (self._prev_drc, self._curr_drc)

    def _get_info(self):
        """"provide index for current iteration"""
        return {
            "iteration index": self._index
        }

    def reset(self, seed=None, options=None):
        # We need the following line to seed self.np_random
        super().reset(seed=seed)

        # randomly set a starting DRC value before 0th iteration
        self._prev_drc = -1
        self._curr_drc = self.np_random.integers(1, self.size, size=1, dtype=int)
        self._index = -1    # before running any detailed routing DRC sequence setting
        self._reward = -1   # reward starting value
        observation = self._get_obs()
        info = self._get_info()
    
        # if self.render_mode == "human":
        #     self._render_frame()
    
        return observation, info

    def step(self, action):
        # Map the action (element of {0,1,2,3}) to the setting we use
        setting = self._action_to_setting[action]
        reward = self._reward
        
        # update DRC values
        # for a fake-runtime, generate a random number which is smaller or equal to previous DRC
        self._prev_drc = self._curr_drc
        self._curr_drc = self.np_random.integers(0, self._curr_drc, size=1, dtype=int)
        
        # update index
        self._index += 1
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

        # update observation and info
        observation = self._get_obs()
        info = self._get_info()
    
        # if self.render_mode == "human":
        #     self._render_frame()
    
        return observation, reward, terminated, truncated, info

    # def render(self):
    #     if self.render_mode == "rgb_array":
    #         return self._render_frame()
    
    # def _render_frame(self):
    #     if self.window is None and self.render_mode == "human":
    #         pygame.init()
    #         pygame.display.init()
    #         self.window = pygame.display.set_mode(
    #             (self.window_size, self.window_size)
    #         )
    #     if self.clock is None and self.render_mode == "human":
    #         self.clock = pygame.time.Clock()
    
    #     canvas = pygame.Surface((self.window_size, self.window_size))
    #     canvas.fill((255, 255, 255))
    #     pix_square_size = (
    #         self.window_size / self.size
    #     )  # The size of a single grid square in pixels
    
    #     # First we draw the target
    #     pygame.draw.rect(
    #         canvas,
    #         (255, 0, 0),
    #         pygame.Rect(
    #             pix_square_size * self._target_location,
    #             (pix_square_size, pix_square_size),
    #         ),
    #     )
    #     # Now we draw the agent
    #     pygame.draw.circle(
    #         canvas,
    #         (0, 0, 255),
    #         (self._agent_location + 0.5) * pix_square_size,
    #         pix_square_size / 3,
    #     )
    
    #     # Finally, add some gridlines
    #     for x in range(self.size + 1):
    #         pygame.draw.line(
    #             canvas,
    #             0,
    #             (0, pix_square_size * x),
    #             (self.window_size, pix_square_size * x),
    #             width=3,
    #         )
    #         pygame.draw.line(
    #             canvas,
    #             0,
    #             (pix_square_size * x, 0),
    #             (pix_square_size * x, self.window_size),
    #             width=3,
    #         )
    
    #     if self.render_mode == "human":
    #         # The following line copies our drawings from `canvas` to the visible window
    #         self.window.blit(canvas, canvas.get_rect())
    #         pygame.event.pump()
    #         pygame.display.update()
    
    #         # We need to ensure that human-rendering occurs at the predefined framerate.
    #         # The following line will automatically add a delay to keep the framerate stable.
    #         self.clock.tick(self.metadata["render_fps"])
    #     else:  # rgb_array
    #         return np.transpose(
    #             np.array(pygame.surfarray.pixels3d(canvas)), axes=(1, 0, 2)
    #         )

    # def close(self):
    #     if self.window is not None:
    #         pygame.display.quit()
    #         pygame.quit()