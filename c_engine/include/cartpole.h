#ifndef CARTPOLE_H
#define CARTPOLE_H

// Struct for info about cart and pole
typedef struct {
  float state[4];
  float mass_cart;
  float mass_pole;
  float length;
  float gravity;
  float dt; // Delta Time - timestep
  float force_mag; // max force of the cart (Newtons)
} CartPole;


// ------ API of the physiscs engine ------

// Initializes and allocates memory for cart-pole
CartPole* cp_create(void);

// Frees memory of a cart-pole
void cp_destroy(CartPole*);

// Resets cart-pole to initial state
void cp_reset(CartPole*);

// Takes one step of physics
void cp_step(CartPole*, float force, float dt);

// Copies current state to provided out4 table
void cp_get_state(CartPole*, float* out4);

// Checks if the episode should be terminated
int cp_is_done(CartPole *);


#endif