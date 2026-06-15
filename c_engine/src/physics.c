#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "cartpole.h"

CartPole* cp_create(void) {
  CartPole* cp = (CartPole*)malloc(sizeof(CartPole));
  if (cp == NULL) {
    fprintf(stderr, "Failed to allocate memory for CartPole\n");
    return NULL;
  }
  return cp;
}

void cp_destroy(CartPole* cp) {
  if (cp != NULL) {
    free(cp);
  }
}

void cp_reset(CartPole* cp) {
  cp->state[0] = 0.0f; // cart position
  cp->state[1] = 0.0f; // cart velocity
  cp->state[2] = 3.14159f; // pole angle
  cp->state[3] = 0.0f; // pole angular velocity

  cp->mass_cart = 4.0f;
  cp->mass_pole = 0.08f;
  cp->length = 1.0f; // half the pole's length
  cp->gravity = 9.8f;
  cp->dt = 0.02f; // time step
  cp->force_mag = 20.0f; // max force of the cart (Newtons)
}

void cp_step(CartPole* cp, float force, float dt) {
  float x = cp->state[0];
  float x_dot = cp->state[1];
  float theta = cp->state[2];
  float theta_dot = cp->state[3];

  float costheta = cosf(theta);
  float sintheta = sinf(theta);

  // For the dynamics equations, see:
  // https://coneural.org/florian/papers/05_cart_pole.pdf
  float temp = (force + cp->mass_pole * cp->length * theta_dot * theta_dot * sintheta) / (cp->mass_cart + cp->mass_pole);
  float theta_acc = (cp->gravity * sintheta - costheta * temp) / (cp->length * (4.0f/3.0f - cp->mass_pole * costheta * costheta / (cp->mass_cart + cp->mass_pole)));
  float x_acc = temp - cp->mass_pole * cp->length * theta_acc * costheta / (cp->mass_cart + cp->mass_pole);

  // Update state using Euler's method
  x_dot += x_acc * dt;
  x += x_dot * dt;
  theta_dot += theta_acc * dt;
  theta += theta_dot * dt;

  theta_dot *= 0.997f; // Damping for the pole's angular velocity
  x_dot *= 0.98f; // Damping for the cart's velocity

  cp->state[0] = x;
  cp->state[1] = x_dot;
  cp->state[2] = theta;
  cp->state[3] = theta_dot;
}

void cp_get_state(CartPole* cp, float* out4) {
  for (int i = 0; i < 4; i++) {
    out4[i] = cp->state[i];
  }
}

int cp_is_done(CartPole* cp) {
    float x = cp->state[0];
    float theta = cp->state[2];

    float x_threshold = 2.4f;
    float theta_threshold = 0.21f; // about 12 degrees

    if (x < -x_threshold || x > x_threshold) return 1;
    if (theta < -theta_threshold || theta > theta_threshold) return 1;

      return 0; // Still working
  }
