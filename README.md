## Energy Manager - nRF52840 Dongle Branch ⚡

This branch includes modifications to ensure compatibility with the **nRF52840 Dongle** for running the Energy Manager project. The nRF52840 Dongle is a low-power, compact, and cost-efficient solution for IoT applications, making it ideal for this project.

## 🔧 Modifications
- Optimized **CoAP protocol** communication to work efficiently on the dongle's constrained hardware.
- Integrated the ML model using the **emlearn library**, ensuring it runs smoothly within the dongle's limited memory and processing capabilities.
- Adjusted **power management** and energy-saving modes to align with the nRF52840's low-power features.
- Removed timestamp from messages for the inability of the dongle to create one.
- Adapted float format (%f) with int format variations for the use of string functions, due to inability of handling such format by the dongle.
