#include <iostream>
#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Set the hint for PS5 rumble support
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");

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
    } else {
        printf("Accelerometer is NOT supported.\n");
    }

    if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO)) {
        if (SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_GYRO, SDL_TRUE) < 0) {
            printf("Failed to enable gyroscope: %s\n", SDL_GetError());
        } else {
            printf("Gyroscope enabled.\n");
        }
    } else {
        printf("Gyroscope is NOT supported.\n");
    }

    // Main loop
    SDL_Event event;
    bool running = true;

    while (running) {
        // Poll events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_CONTROLLERBUTTONDOWN:
                    printf("Button %d pressed.\n", event.cbutton.button);
                    break;

                case SDL_CONTROLLERBUTTONUP:
                    printf("Button %d released.\n", event.cbutton.button);
                    break;

                case SDL_CONTROLLERAXISMOTION:
                    printf("Controller Axis %d: %d\n", event.caxis.axis, event.caxis.value);
                    break;

                case SDL_CONTROLLERDEVICEREMOVED:
                    printf("Controller removed.\n");
                    running = false;
                    break;

                case SDL_SENSORUPDATE:
                    if (event.csensor.sensor == SDL_SENSOR_ACCEL) {
                        printf("Accelerometer - X: %.2f, Y: %.2f, Z: %.2f\n",
                               event.csensor.data[0],
                               event.csensor.data[1],
                               event.csensor.data[2]);
                    } else if (event.csensor.sensor == SDL_SENSOR_GYRO) {
                        printf("Gyroscope - X: %.2f, Y: %.2f, Z: %.2f\n",
                               event.csensor.data[0],
                               event.csensor.data[1],
                               event.csensor.data[2]);
                    }
                    break;

                default:
                    break;
            }
        }

        SDL_Delay(16);  // Delay for 60 FPS
    }

    // Clean up
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
