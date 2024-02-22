from gymnasium.envs.registration import register

register(
     id="DREnv_fake/DREnv-v0",
     entry_point="DREnv_fake.envs:DREnv",
     max_episode_steps=65,
)