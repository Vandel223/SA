### Developed for SA at Instituto Superior Técnico

# Sensors and Actuators Lab

Arduino and MATLAB code for a set of Sensors and Actuators lab exercises.

## Structure

- `SA_lab_ino/` — Arduino sketches, one folder per exercise:
  - `SA_led` — basic LED blink
  - `SA_ldr` — photoresistor analog readings
  - `SA_temp` — DS18B20 temperature logging
  - `SA_ultrasounds` — HC-SR04 distance measurement (interrupt-based)
  - `SA_acc_print` / `SA_acc_crash_detect` — ADXL330 accelerometer reading and crash detection
  - `SA_DC_motor` — DC motor speed measurement and transfer function
  - `SA_stepper` / `SA_stepper_half` — stepper motor position/speed control (full-step and half-step)
- `SA_lab_matlab/` — MATLAB analysis of Peltier module thermal data:
  - `peltier_analysis.m` — loads and plots rise/fall transients and steady-state comparisons
  - `data/` — CSV logs used by the script

## Usage

- Open the relevant `.ino` file in the Arduino IDE, select your board, and upload.
- Run `peltier_analysis.m` in MATLAB from the `SA_lab_matlab/` folder (expects the `data/` subfolder alongside it).

## Requirements

- Arduino IDE (libraries: `Stepper`, `AccelStepper`, `DS18B20` depending on sketch)
- MATLAB
