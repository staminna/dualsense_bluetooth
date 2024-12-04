#include <iostream>
#include <SDL.h>
#include <lo/lo.h>
#include <hidapi/hidapi.h>

int main(int argc, char *argv[]) {
    // Set up OSC target
    lo_address target = lo_address_new("127.0.0.1", "8000"); // Replace 7400 with your chosen port

    // Initialize HIDAPI
    if (hid_init()) {
        printf("Failed to initialize HIDAPI\n");
        return 1;
    }

    // Open the DualSense controller
    struct hid_device_info *devs, *cur_dev;
    devs = hid_enumerate(0x0, 0x0); // Enumerate all devices
    cur_dev = devs;
    hid_device *handle = NULL;
    while (cur_dev) {
        if (cur_dev->vendor_id == 0x054C && cur_dev->product_id == 0x0CE6) { // DualSense VID/PID
            handle = hid_open_path(cur_dev->path);
            if (handle) {
                printf("DualSense controller opened.\n");
                break;
            }
        }
        cur_dev = cur_dev->next;
    }
    hid_free_enumeration(devs);

    if (!handle) {
        printf("Failed to open DualSense controller\n");
        hid_exit();
        return 1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        hid_close(handle);
        hid_exit();
        return 1;
    }

    // Main loop
    bool running = true;
    const unsigned char expected_report_id = 0x01; // Define expected report ID
    while (running) {
        // Read sensor data (example)
        unsigned char buf[64]; // Increase buffer size
        int res = hid_read(handle, buf, sizeof(buf));
        if (res > 0) {
            printf("Read %d bytes from controller\n", res);

            // Check report ID
            if (buf[0] != expected_report_id) {
                printf("Unexpected report ID: %02x\n", buf[0]);
                continue;
            }

            // Debug: Print raw data for analysis
            printf("Raw data (hex): ");
            for (int i = 0; i < res; ++i) {
                printf("%02x ", buf[i]);
            }
            printf("\n");

            // Example offsets based on typical DualSense data format
            int16_t accel_x_raw = (buf[1] << 8) | buf[0];
            int16_t accel_y_raw = (buf[3] << 8) | buf[2];
            int16_t accel_z_raw = (buf[5] << 8) | buf[4];

            int16_t gyro_x_raw = (buf[7] << 8) | buf[6];
            int16_t gyro_y_raw = (buf[9] << 8) | buf[8];
            int16_t gyro_z_raw = (buf[11] << 8) | buf[10];

            float accel_x = accel_x_raw / 8192.0f; // Adjust scale
            float accel_y = accel_y_raw / 8192.0f;
            float accel_z = accel_z_raw / 8192.0f;

            float gyro_x = gyro_x_raw / 16.4f; // Adjust scale
            float gyro_y = gyro_y_raw / 16.4f;
            float gyro_z = gyro_z_raw / 16.4f;

            printf("Accel (raw): %d, %d, %d\n", accel_x_raw, accel_y_raw, accel_z_raw);
            printf("Gyro (raw): %d, %d, %d\n", gyro_x_raw, gyro_y_raw, gyro_z_raw);
            printf("Accel: %f, %f, %f\n", accel_x, accel_y, accel_z);
            printf("Gyro: %f, %f, %f\n", gyro_x, gyro_y, gyro_z);

            // Send sensor data over OSC
            lo_send(target, "/sensor/accel", "fff", accel_x, accel_y, accel_z);
            lo_send(target, "/sensor/gyro", "fff", gyro_x, gyro_y, gyro_z);
        } else {
            printf("Failed to read from controller\n");
        }

        SDL_Delay(16); // 60 FPS
    }

    // Clean up
    SDL_Quit();
    hid_close(handle);
    hid_exit();
    return 0;
}
