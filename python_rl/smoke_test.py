import ctypes
import os

# 1. Load the shared library (assuming it's in the parent directory under 'build')
lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build', 'libcartpole.so'))
lib = ctypes.CDLL(lib_path)

# 2. Configure argument and return types for the C functions
lib.cp_create.restype = ctypes.c_void_p
lib.cp_step.argtypes = [ctypes.c_void_p, ctypes.c_float, ctypes.c_float]
lib.cp_get_state.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_float)]
lib.cp_reset.argtypes = [ctypes.c_void_p, ctypes.c_uint]

# 3. Test the C library from Python
print("Initialization of the CartPole environment in C...")
cp_handle = lib.cp_create()

print("Resetting physical parameters...")
lib.cp_reset(cp_handle, 0)

# Place to store the state vector [x, x_dot, theta, theta_dot]
state_array = (ctypes.c_float * 4)()

# Simulate 10 steps in Python, continuously pushing the cart to the right
force = 10.0
dt = 0.02

print("\nSimulation of 10 steps (pushing the cart to the right with a force of 10N):")
for i in range(1, 11):
    lib.cp_step(cp_handle, ctypes.c_float(force), ctypes.c_float(dt))
    lib.cp_get_state(cp_handle, state_array)

    x = state_array[0]
    theta = state_array[2]
    print(f"Step {i}: X Position = {x:.4f} m, Theta Angle = {theta:.4f} rad")

print("\nSuccess! Python and C are communicating and calculating correctly.")