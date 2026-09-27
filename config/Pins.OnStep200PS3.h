// OnStep200P ESP32-S3 pin map. Current wiring: docs/HARDWARE.md.
#pragma once

#if defined(ESP32)

// USB CDC/JTAG is the command/debug port. It does not consume GPIO1/3 on S3.
#if SERIAL_A_BAUD_DEFAULT != OFF
  #define SERIAL_A Serial
#endif

// Shared I2C bus reserved for the existing OLED.
#define I2C_SCL_PIN 17
#define I2C_SDA_PIN 18

// Axis 1: assumed azimuth until the physical driver-to-axis mapping is checked.
#define AXIS1_ENABLE_PIN 1
#define AXIS1_M0_PIN     OFF
#define AXIS1_M1_PIN     OFF
#define AXIS1_M2_PIN     OFF
#define AXIS1_M3_PIN     OFF
#define AXIS1_STEP_PIN   2
#define AXIS1_DIR_PIN    42

// Axis 2: assumed altitude until the physical driver-to-axis mapping is checked.
#define AXIS2_ENABLE_PIN 10
#define AXIS2_M0_PIN     OFF
#define AXIS2_M1_PIN     OFF
#define AXIS2_M2_PIN     OFF
#define AXIS2_M3_PIN     OFF
// GPIO35/36/37 are connected to OPI PSRAM on the S3R8. Requires rewiring
// the existing ALT driver: EN 37->10, STEP 36->9, DIR 35->8.
#define AXIS2_STEP_PIN   9
#define AXIS2_DIR_PIN    8

// Joystick pins are declared here for the project plugin, not as ST4 inputs.
#define ONSTEP200P_JOYSTICK_SW_PIN  6
#define ONSTEP200P_JOYSTICK_Y_PIN   4
#define ONSTEP200P_JOYSTICK_X_PIN   5

#else
#error "OnStep200PS3 pin map requires an ESP32-family target."
#endif
