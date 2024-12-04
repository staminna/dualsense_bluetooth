#include <iostream>
#include <SDL.h>
#include <lo/lo.h>
#include <hidapi/hidapi.h>
#include <signal.h>
#include <chrono>
#include <thread>

// Global variables for graceful shutdown
volatile bool running = true;
volatile bool start_readings = false;

// Signal handler for CTRL+C/Break
void signalHandler(int signum) {
    running = false;
    std::cout << "\nShutting down...\n";
}

// Check if sensors are available
bool checkSensors(hid_device* handle) {
    if (!handle) return false;

    unsigned char buf[64];
    std::cout << "  Attempting to read sensor data...\n";

    // Try to read data multiple times
    for (int attempt = 0; attempt < 5; attempt++) {
        int res = hid_read_timeout(handle, buf, sizeof(buf), 2000);
        std::cout << "  Read attempt " << attempt + 1 << ": " << res << " bytes\n";
        
        if (res > 0) {
            std::cout << "  Raw data: ";
            for (int i = 0; i < res; i++) {
                printf("%02x ", buf[i]);
            }
            std::cout << "\n";

            // For DualSense, we expect the first byte to be the report ID (usually 0x01)
            std::cout << "  Report ID: 0x" << std::hex << (int)buf[0] << std::dec << "\n";
            
            // Even if we don't get all the bytes we expect, if we get any data, consider it a success
            return true;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "  Read attempt " << attempt + 1 << " failed. Retrying...\n";
    }

    std::cerr << "  Failed to read any sensor data after 5 attempts\n";
    return false;
}

// Attempt to connect to the DualSense controller over Bluetooth
hid_device* connectController() {
    struct hid_device_info* devs = hid_enumerate(0x054c, 0x0ce6); // DualSense VID/PID
    if (!devs) {
        std::cout << "No DualSense controllers found\n";
        return nullptr;
    }

    struct hid_device_info* cur_dev = devs;
    hid_device* handle = nullptr;
    int device_count = 0;

    while (cur_dev) {
        device_count++;
        std::cout << "\nFound device " << device_count << ":\n";
        std::cout << "  Path: " << (cur_dev->path ? cur_dev->path : "null") << "\n";
        std::cout << "  Manufacturer: " << (cur_dev->manufacturer_string ? 
            std::string(reinterpret_cast<const char*>(cur_dev->manufacturer_string)) : "null") << "\n";
        std::cout << "  Product: " << (cur_dev->product_string ? 
            std::string(reinterpret_cast<const char*>(cur_dev->product_string)) : "null") << "\n";
        std::cout << "  VID/PID: " << std::hex << cur_dev->vendor_id << "/" << cur_dev->product_id << std::dec << "\n";
        std::cout << "  Interface: " << cur_dev->interface_number << "\n";

        if (cur_dev->path) {
            std::cout << "  Attempting to open device...\n";
            // Try to open the device directly
            handle = hid_open(0x054c, 0x0ce6, nullptr);
            if (handle) {
                std::cout << "  Successfully opened DualSense controller\n";
                if (checkSensors(handle)) {
                    std::cout << "  Sensors verified\n";
                    break;
                } else {
                    std::cout << "  Sensor check failed\n";
                    hid_close(handle);
                    handle = nullptr;
                }
            } else {
                std::cout << "  Failed to open device: " << hid_error(nullptr) << "\n";
                // Try alternative method using path
                handle = hid_open_path(cur_dev->path);
                if (handle) {
                    std::cout << "  Successfully opened DualSense controller using path\n";
                    if (checkSensors(handle)) {
                        std::cout << "  Sensors verified\n";
                        break;
                    } else {
                        std::cout << "  Sensor check failed\n";
                        hid_close(handle);
                        handle = nullptr;
                    }
                } else {
                    std::cout << "  Failed to open device using path: " << hid_error(nullptr) << "\n";
                }
            }
        }
        cur_dev = cur_dev->next;
    }

    if (device_count == 0) {
        std::cout << "No DualSense devices found\n";
    }

    hid_free_enumeration(devs);
    return handle;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Set up OSC target
    lo_address target = lo_address_new("127.0.0.1", "7400");

    if (hid_init()) {
        std::cerr << "Failed to initialize HIDAPI\n";
        return 1;
    }

    hid_device* handle = nullptr;
    while (running && !handle) {
        std::cout << "Attempting to connect to DualSense controller...\n";
        handle = connectController();
        if (!handle) {
            std::cout << "Failed to connect. Retrying in 5 seconds...\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    if (!handle) {
        std::cerr << "Failed to connect to controller\n";
        hid_exit();
        return 1;
    }

    std::cout << "Press ENTER to start sensor readings...\n";
    std::cin.get();
    start_readings = true;

    // Main sensor reading loop
    while (running) {
        unsigned char buf[64];
        
        // Read input report
        int res = hid_read(handle, buf, sizeof(buf));
        
        if (res > 0) {
            // Debug raw data
            std::cout << "Raw data (" << res << " bytes): ";
            for (int i = 0; i < res; i++) {
                printf("%02x ", buf[i]);
            }
            std::cout << "\n";

            if (buf[0] == 0x01) {  // Check for correct report ID
                // Based on the raw data we're receiving:
                // buf[1] and buf[2] might be X axis
                // buf[3] and buf[4] might be Y axis
                // buf[5] and buf[6] might be Z axis
                
                int16_t x_raw = buf[1];  // Using single byte for now
                int16_t y_raw = buf[3];
                int16_t z_raw = buf[5];

                // Normalize to floating point (-1.0 to 1.0)
                float x = (x_raw - 128) / 128.0f;  // Assuming center is around 128
                float y = (y_raw - 128) / 128.0f;
                float z = (z_raw - 128) / 128.0f;

                // Send data over OSC
                lo_send(target, "/sensor/accel", "fff", x, y, z);
                lo_send(target, "/sensor/gyro", "fff", x, y, z);  // Using same data for now

                // Debug output
                std::cout << "Normalized values - X: " << x << ", Y: " << y << ", Z: " << z << "\n";
            }
        } else if (res == 0) {
            // No data available, wait a bit
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            std::cerr << "Failed to read data (res = " << res << ")\n";
            if (res < 0) {
                std::cerr << "Error: " << hid_error(handle) << "\n";
            }
            // Try to reconnect
            hid_close(handle);
            handle = connectController();
            if (!handle) {
                std::cout << "Reconnection failed. Retrying...\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60Hz
    }

    // Cleanup
    if (handle) hid_close(handle);
    hid_exit();
    return 0;
}
