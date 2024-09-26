### Alessio Franchini, Alice Petrillo
## Energy Manager ⚡

This project is an **Internet of Things (IoT)** system designed to manage a **microgrid**. A microgrid is a localized energy system that can operate independently or in conjunction with a broader utility grid. The system generates electricity using **renewable energy sources** and manages energy flow between local consumers, storage systems, and the grid.

## ✨ Features
- 🌐 **Real-time monitoring** of energy production, consumption, and storage.
- 🔋 **User-defined battery threshold** for optimal energy storage management.
- 📡 **Sensors and actuator status monitoring** to ensure system reliability.
- 🤖 **Predictive model** for estimating energy production based on solar panel data (irradiance and temperature).

## 🏗️ System Architecture

### 🛠️ IoT Devices:
The system is based on several IoT devices:
- **Sensors**: 
  - 🌡️ **Thermometer**: Measures the temperature of the solar panel.
  - ☀️ **Pyranometer**: Measures the irradiance on the solar panel.
  - ⚖️ **Capacity Sensor**: Measures the battery charge level.
  - 🔌 **Energy Meter**: Measures the energy consumption of the home appliances.
- **Actuator**: 
  - ⚙️ **Inverter**: Manages the energy flow between the house, battery, and grid based on production and consumption data.

### 📡 CoAP Network:
The system communicates through the **CoAP protocol**, where:
- **Sensors** send data to the CoAP server at regular intervals.
- **Actuator** registers itself to the server and receives data from the sensors to control energy distribution.

### 🗄️ Data Storage:
- Data from sensors and the actuator are stored in a **SQL database**.
- A **remote control application** provides a command-line interface to monitor energy flow, set battery thresholds, and check the connection status of sensors and the actuator.

### 📊 Visualization:
- Data is visualized using **Grafana**, which shows:
  - 📈 **Time series** of energy flow.
  - 🟢 **Real-time sensor values**.

## 🧠 Machine Learning Model
A **regression model** predicts the amount of energy produced by the solar panel based on temperature, irradiance, and timestamp (month, day, hour). The model is implemented using **TensorFlow** and trained to minimize the **mean squared error**. The model is deployed using the **emlearn** library for integration into the system.

