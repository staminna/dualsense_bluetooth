#include <iostream>
#include <SDL.h>
#include <lo/lo.h>

int main(int argc, char *argv[]) {
    // Set up OSC target
    lo_address target = lo_address_new("127.0.0.1", "9000"); // Replace 7400 with your chosen port

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

    // Enable sensors
    SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_ACCEL, SDL_TRUE);
    SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_GYRO, SDL_TRUE);

    // Main loop
    bool running = true;
    while (running) {
        float accel[3] = {0};
        float gyro[3] = {0};

        // Get accelerometer data
        if (SDL_GameControllerGetSensorData(controller, SDL_SENSOR_ACCEL, accel, 3) == 0) {
            lo_send(target, "/sensor/accel", "fff", accel[0], accel[1], accel[2]);
        }

        // Get gyroscope data
        if (SDL_GameControllerGetSensorData(controller, SDL_SENSOR_GYRO, gyro, 3) == 0) {
            lo_send(target, "/sensor/gyro", "fff", gyro[0], gyro[1], gyro[2]);
        }

        SDL_Delay(16); // 60 FPS
    }

    // Clean up
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
