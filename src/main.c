/*
    MIT License

    Copyright (c) 2024 SpyterDev

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#include <stdint.h>
#include <math.h>
#include <time.h>
#include "../include/raylib.h"
#include "input.h"

char power_text[12] = "           "; // Text buffer
float power = 0; // Power variable
float power_cap = 52.0833333334; // 20m cap ingame
uint8_t controller_connected = 4; // 0-3 are controllers, 4 means no controller connected

void connect_Controller(void) {
    static clock_t cooldown = 0;
    if (clock() < cooldown + 1000) return; // Checks every secend to see what controller to use
    for (int i = 0; i < 3; i++) {
        if (IsGamepadAvailable(i)) controller_connected = i;
    }
    cooldown = clock();
}

uint8_t input_Triggered(void) {
    // Checks if a controller is connected and whether east (on the D-Pad) is down
    uint8_t controller = 0;
    if (controller_connected < 4) controller = IsGamepadButtonDown(controller_connected, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    return Get_Keypress(VK_MENU) || controller;
}

void uint32_to_string_dest(uint32_t number, char * dest) {
    int temp = number;
    int length = 1;
    while (temp > 9) temp /= 10, length++; // Checks number of digits and length of string
    temp = 0;
    while (temp < length) dest[length-temp-1] = '0' + number - (number / 10) * 10, number/=10, temp++;
    dest[temp] = 'm';
    dest[temp+1] = '\0';
}

// Magical colour shift code (may be used in the future if I update the UI)
Color color_Transition(Color x, Color y, float percentage) {
    return (Color){ x.r * (1 - percentage) + y.r * percentage, 
                    x.g * (1 - percentage) + y.g * percentage,
                    x.b * (1 - percentage) + y.b * percentage,
                    x.a * (1 - percentage) + y.a * percentage};
}

// Shows a progress bar to show how high you're about to jump
void render_Progressbar(int x, int width) {
    if (!input_Triggered()) return; // If you're not trying to jump then it won't show

    // Renders gradient block aka power meter

    DrawRectangleGradientH( x, GetScreenHeight()/2 - GetScreenHeight()/20 * 9, // X and Y cords
                            width, GetScreenHeight()/10*9, // Size of gradient block
                    GREEN, RED);
    
    // Hides part of the power meter to show current jump height

    DrawRectangle(  x + width - width * (power_cap - power) / power_cap, 
                    GetScreenHeight()/2 - GetScreenHeight()/20*9,
                    width * (power_cap - power) / power_cap+1,
                    GetScreenHeight()/10*9,
                    BLACK); // Fancy magic that covers the power meter past the speed
}

// Shows curent jump height in fortnite meters (in game distance)
void render_Info(int x) {
    register uint8_t length = 0;
    while (power_text[length]) length++;
    uint8_t font_size = GetScreenWidth()/4/length;
    uint32_to_string_dest((uint32_t)roundf(power*0.384f), power_text);

    // A scraped circular power meter that goes around the text that shows you your jump height
    /*  
        DrawCircleSector((Vector2){x + (float)GetScreenWidth()/15, GetScreenHeight()>>1},
                    (float)GetScreenWidth()/10, 0, 
                    360 * power / power_cap, 
                    100, color_Transition((Color) {0,210,0,255}, (Color) {210,0,0,255}, power / power_cap));
   
        DrawCircle(x + (float)GetScreenWidth()/15, GetScreenHeight()>>1, (float)GetScreenWidth()/11, BLACK);
    */
   
   // Shows you how high your current jump height is

   DrawText(power_text, x, GetScreenHeight()/2 - font_size/2,  font_size, WHITE);
}

// Tells Windows to push F1 4 times at a certain speed
void trigger_Input(void) {
    register uint16_t interval = 1 / power * 1000; // Wizardry that gets the rate of clicks
    if (interval > 1000) interval = 1000;
    for (int i = 0; i < 4; i++) {
        Simulate_Keypress(VK_F1); // This part tells Windows to that you "pressed" F1
        Sleep(interval); // Pauses to avoid going to fast
    }
    power = 0; // Resets power
}

void update_velocity(void) {
    if (input_Triggered() && power < power_cap) power+=0.04*(GetFrameTime()*700); // Increases power when you are trying to
    if (power > power_cap) power = power_cap; // Stops you from going over power cap
    if (!input_Triggered() && power != 0) trigger_Input();
}

// If you having super jumped in 3 seconds, the window is hidden
void update_Visibility(void) {
    static clock_t cooldown = 0;
    if (input_Triggered()) cooldown = clock(); // Resets visibility timer

    // Hides window when inactive
    if (clock() - cooldown >=  3 * CLOCKS_PER_SEC && GetWindowPosition().y >= 0) SetWindowPosition(0, -GetScreenHeight() - 1);

    // Shows window when active
    else if (clock() - cooldown <  3 * CLOCKS_PER_SEC && GetWindowPosition().y < 0)  {
        SetWindowPosition(GetMonitorWidth(0) * 0.0260416666667,
                          GetMonitorHeight(0) * 0.0185185185185);
    }
}

int main() {
    InitWindow(0, 0, "FN Super Jumper"); // Creates window
    SetWindowSize(GetMonitorWidth(0) / 4, GetMonitorHeight(0) / 10);
    SetWindowPosition(0, -GetScreenHeight() - 1); // Hides window
    SetWindowState(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
    while (!WindowShouldClose() && !Get_Keypress(VK_F6)) {

        // Updates current controller to see if there is one connect or if disconnected

        connect_Controller();

        // Hides window if inactive

        update_Visibility();


        BeginDrawing();
        ClearBackground(BLACK);

        // Renders power bar and text on screen

        render_Info(GetScreenWidth()/20);
        render_Progressbar(GetScreenWidth()/4, 3 * GetScreenWidth() / 4 - GetScreenWidth()/30);

        update_velocity();
        
        EndDrawing();
    }
    CloseWindow();
    return 1;
}