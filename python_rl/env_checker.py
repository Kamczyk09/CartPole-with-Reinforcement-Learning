import gymnasium as gym
from gymnasium.utils.env_checker import check_env
from cartpole_env import CartPoleCustomEnv

def main():
    print("Initializing the custom CartPole environment...")
    env = CartPoleCustomEnv()

    print("\n--- Running standard Gymnasium checks ---")
    # This function will throw a detailed exception if the API is violated
    check_env(env.unwrapped)
    print("SUCCESS: The environment is fully Gymnasium API compliant!")

    print("\n--- Running a random agent simulation ---")
    obs, info = env.reset()

    steps_survived = 0
    episodes = 1

    # Run a simple loop to see how a completely random agent performs
    for step in range(200):
        # Sample a random continuous action from [-1.0, 1.0]
        action = env.action_space.sample()

        # Take a step in the C physics engine
        obs, reward, terminated, truncated, info = env.step(action)
        steps_survived += 1

        if terminated or truncated:
            print(f"Episode {episodes} ended after {steps_survived} steps. (Terminated: {terminated})")
            obs, info = env.reset()
            steps_survived = 0
            episodes += 1

    print("\nSimulation finished without any crashes!")
    env.close()

if __name__ == "__main__":
    main()