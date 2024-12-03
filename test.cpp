#include <hidapi/hidapi.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cstring>

// DualSense VID/PID
#define DS_VENDOR_ID 0x054c
#define DS_PRODUCT_ID 0x0ce6

// DualSense input report offsets based on dualsensectl
#define DS_INPUT_REPORT_USB 0x01
#define DS_INPUT_REPORT_USB_SIZE 64
#define DS_INPUT_REPORT_GYRO_X_OFFSET 16
#define DS_INPUT_REPORT_GYRO_Y_OFFSET 18
#define DS_INPUT_REPORT_GYRO_Z_OFFSET 20
#define DS_INPUT_REPORT_ACCEL_X_OFFSET 22
#define DS_INPUT_REPORT_ACCEL_Y_OFFSET 24
#define DS_INPUT_REPORT_ACCEL_Z_OFFSET 26
#define DS_INPUT_REPORT_SENSOR_TIMESTAMP_OFFSET 28

int main(int argc, char* argv[]) {
    if (hid_init()) {
        std::cerr << "Failed to initialize HIDAPI" << std::endl;
        return 1;
    }

    // Open the DualSense controller
    hid_device *device = hid_open(DS_VENDOR_ID, DS_PRODUCT_ID, NULL);
    if (!device) {
        std::cerr << "Unable to open DualSense controller" << std::endl;
        hid_exit();
        return 1;
    }

    // Get device info
    wchar_t wstr[255];
    hid_get_manufacturer_string(device, wstr, 255);
    std::wcout << L"Manufacturer: " << wstr << std::endl;
    hid_get_product_string(device, wstr, 255);
    std::wcout << L"Product: " << wstr << std::endl;
    hid_get_serial_number_string(device, wstr, 255);
    std::wcout << L"Serial Number: " << wstr << std::endl;

    std::cout << "\nReading sensor data for 10 seconds..." << std::endl;
    std::cout << "Move the controller around to see if values change" << std::endl;
    std::cout << "Press Ctrl+C to exit\n" << std::endl;

    // Main loop
    auto start_time = std::chrono::steady_clock::now();
    bool running = true;
    uint8_t data[DS_INPUT_REPORT_USB_SIZE];

    while (running) {
        // Read input report
        int res = hid_read(device, data, sizeof(data));
        if (res > 0) {
            // Extract sensor data (assuming little-endian)
            int16_t gyro_x = *(int16_t*)&data[DS_INPUT_REPORT_GYRO_X_OFFSET];
            int16_t gyro_y = *(int16_t*)&data[DS_INPUT_REPORT_GYRO_Y_OFFSET];
            int16_t gyro_z = *(int16_t*)&data[DS_INPUT_REPORT_GYRO_Z_OFFSET];
            
            int16_t accel_x = *(int16_t*)&data[DS_INPUT_REPORT_ACCEL_X_OFFSET];
            int16_t accel_y = *(int16_t*)&data[DS_INPUT_REPORT_ACCEL_Y_OFFSET];
            int16_t accel_z = *(int16_t*)&data[DS_INPUT_REPORT_ACCEL_Z_OFFSET];

            uint32_t timestamp = *(uint32_t*)&data[DS_INPUT_REPORT_SENSOR_TIMESTAMP_OFFSET];

            // Print sensor data
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Gyro:  X=" << gyro_x << " Y=" << gyro_y << " Z=" << gyro_z << std::endl;
            std::cout << "Accel: X=" << accel_x << " Y=" << accel_y << " Z=" << accel_z << std::endl;
            std::cout << "Timestamp: " << timestamp << std::endl;
            std::cout << "----------------------------------------" << std::endl;
        }

        // Check if 10 seconds have passed
        auto current_time = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count() >= 10) {
            running = false;
        }

        // Small delay to prevent too frequent polling
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60Hz
    }

    hid_close(device);
    hid_exit();
    return 0;
}
