//
//  main.cpp
//  ps5-kontroller-sdl2-senson
//
//  Created by Jorge Domingues Nunes on 02/12/2024.
//

#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_SENSOR) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Open the first available controller
    SDL_GameController *controller = NULL;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            controller = SDL_GameControllerOpen(i);
            if (controller) {
                printf("Controller opened: %s\n", SDL_GameControllerName(controller));
                break;
            } else {
                printf("Could not open controller: %s\n", SDL_GetError());
            }
        }
    }

    if (controller == NULL) {
        printf("No controller detected!\n");
        SDL_Quit();
        return 1;
    }

    // Enable sensors (Accelerometer and Gyroscope)
    if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_ACCEL)) {
        if (SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_ACCEL, SDL_TRUE) < 0) {
            printf("Failed to enable accelerometer: %s\n", SDL_GetError());
        } else {
            printf("Accelerometer enabled.\n");
        }
    }

    if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO)) {
        if (SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_GYRO, SDL_TRUE) < 0) {
            printf("Failed to enable gyroscope: %s\n", SDL_GetError());
        } else {
            printf("Gyroscope enabled.\n");
        }
    }

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }

            // Handle button press events
            if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                printf("Button %d pressed\n", event.cbutton.button);
            } else if (event.type == SDL_CONTROLLERBUTTONUP) {
                printf("Button %d released\n", event.cbutton.button);
            }

            // Handle joystick axis motion
            if (event.type == SDL_CONTROLLERAXISMOTION) {
                printf("Axis %d value: %d\n", event.caxis.axis, event.caxis.value);
            }

            // Handle device removal
            if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
                printf("Controller removed!\n");
                running = 0;
            }
        }

        // Check for accelerometer data
        float accel[3];
        if (SDL_GameControllerGetSensorData(controller, SDL_SENSOR_ACCEL, accel, 3) == 0) {
            printf("Accelerometer: X=%f, Y=%f, Z=%f\n", accel[0], accel[1], accel[2]);
        }

        // Check for gyroscope data
        float gyro[3];
        if (SDL_GameControllerGetSensorData(controller, SDL_SENSOR_GYRO, gyro, 3) == 0) {
            printf("Gyroscope: X=%f, Y=%f, Z=%f\n", gyro[0], gyro[1], gyro[2]);
        }

        SDL_Delay(100); // Avoid spamming output
    }

    // Clean up
    SDL_GameControllerClose(controller);
    SDL_Quit();

    return 0;
}
