#include <stdio.h>
#include <math.h>
#include "cartpole.h"
#include "raylib.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define SCALE 100.0f

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CartPole - Tryb Ręczny (C + Raylib)");
    SetTargetFPS(60);

    // Initializing cart-pole
    CartPole* cp = cp_create();
    cp_reset(cp);

    float dt = 1.0f / 60.0f;
    cp->dt = dt;


    while (!WindowShouldClose()) {
        float force = 0.0f;

        if (IsKeyDown(KEY_LEFT))  force = -cp->force_mag;
        if (IsKeyDown(KEY_RIGHT)) force = cp->force_mag;

        cp_step(cp, force, dt);


        // ------ Drawing graphics ------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        float x = cp->state[0];
        float theta = cp->state[2];

        // Converting meters to pixels. x=0 is the center of the screen.
        int cart_px = SCREEN_WIDTH / 2 + (int)(x * SCALE);
        int cart_py = SCREEN_HEIGHT / 2 + 50;

        int cart_width = 50;
        int cart_height = 30;

        // Drawing tracks
        DrawLine(0, cart_py, SCREEN_WIDTH, cart_py, LIGHTGRAY);

        // Drawing the blue cart
        DrawRectangle(cart_px - cart_width/2, cart_py - cart_height/2, cart_width, cart_height, BLUE);

        // Drawing pendulum pole as a red line. Lenght of the pole in pixels is cp->length * 2 (since length is half the pole's length) multiplied by SCALE.
        float pole_length = cp->length * 2.0f;
        int pole_end_x = cart_px + (int)(pole_length * SCALE * sinf(theta));
        int pole_end_y = cart_py - (int)(pole_length * SCALE * cosf(theta));

        DrawLineEx((Vector2){(float)cart_px, (float)cart_py},
                   (Vector2){(float)pole_end_x, (float)pole_end_y},
                   8.0f, RED);

        // Drawing HUD interface
        DrawText("Control: LEFT/RIGHT", 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Position X: %.2f m", x), 10, 40, 20, DARKGRAY);
        DrawText(TextFormat("Angle Theta: %.2f rad", theta), 10, 70, 20, DARKGRAY);
        DrawText(TextFormat("Force: %.1f N", force), 10, 100, 20, (force == 0) ? GRAY : MAROON);

        EndDrawing();
    }

    cp_destroy(cp);
    CloseWindow();

    return 0;
}
