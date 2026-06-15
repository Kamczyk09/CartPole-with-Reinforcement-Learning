import os
from stable_baselines3 import PPO
from cartpole_env import CartPoleCustomEnv

def main():
    # 1. Setup paths for saving the model
    models_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'models'))
    os.makedirs(models_dir, exist_ok=True)
    model_path = os.path.join(models_dir, "ppo_cartpole")

    # 2. Initialize the environment
    print("Initializing environment...")
    env = CartPoleCustomEnv()

    # 3. Create the PPO agent
    # MlpPolicy: A standard neural network (Multi-Layer Perceptron)
    # verbose=1: Prints training metrics (like average reward) to the console
    print("Creating PPO model...")
    model = PPO("MlpPolicy", env, verbose=1)

    # 4. Train the agent
    print("Starting training (100,000 timesteps)...")
    model.learn(total_timesteps=100_000)

    # 5. Save the weights
    print(f"Saving the model to {model_path}...")
    model.save(model_path)

    print("Training finished successfully!")
    env.close()

if __name__ == "__main__":
    main()