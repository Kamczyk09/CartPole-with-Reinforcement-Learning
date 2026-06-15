import os
import pygame
from stable_baselines3 import PPO
from cartpole_env import CartPoleCustomEnv

def main():
    # Load environment and model
    env = CartPoleCustomEnv(render_mode="human")
    model_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'models', 'ppo_cartpole'))

    print("Loading the trained model...")
    model = PPO.load(model_path)

    obs, info = env.reset()
    total_reward = 0

    print("Testing the policy...")
    running = True
    for step in range(2000):
        if not running: break

        env.render()  # Render the environment to visualize the agent's behavior

        # deterministic=True ensures the agent takes the best known action, not a random exploratory one
        action, _states = model.predict(obs, deterministic=True)

        obs, reward, terminated, truncated, info = env.step(action)
        total_reward += reward

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        if terminated or truncated:
            print(f"Episode ended after {step} steps. (Terminated: {terminated})")
            break

    print(f"Total reward achieved: {total_reward} / 500.0")
    env.close()

if __name__ == "__main__":
    main()