# Frontend Software for PDC

A real-time data visualization application designed for **Phasor Data Concentrators (PDCs)** and **Phasor Measurement Units (PMUs)**.  
Built using **C++ (Qt 6 + QtCharts)**, it provides smooth and interactive visualization of streaming power system data with customizable controls.  

---

## Features
- Variable Selection Dropdown: Choose from 15+ data variables (phasor magnitudes, angles, frequency, ROCOF, analog values).  
- Adjustable Window Size: Control the time window for flexible visualization.  
- Real-Time Data Smoothing: Smooth curves using `QSplineSeries` for better readability.  
- Scrollable Time Window: Navigate historical data with auto-resume scrolling for live updates.  
- Split View: Compare two variables side by side using a dynamic split window.  

---

## Technology Stack
- Programming Language: C++  
- Framework: Qt 6 (GUI + Widgets)  
- Charting Library: QtCharts (`QChartView`, `QSplineSeries`)  
- Installer: Qt Installer Framework (cross-platform executables)  

---

## Getting Started

### Installation
1. Run the installer:  
   ```bash
   gui_installer.exe
   ```
   - If Windows shows a security warning, click "More Info" → "Run Anyway".  
2. Choose an installation directory (default: `C:\Program Files\`).  
3. Complete the installation wizard and launch the app.  

### Running the Application
- Open the installed folder and run:  
  ```
  Find Frontend Software for PDC.exe
  ```
- Use dropdowns to select variables and adjust the time window.  
- Use scroll bar to navigate history, and split view to compare two variables.  

---

## Sample Outputs
- Phasor magnitude vs. time plots  
- Angle plots (radians/degrees)  
- Frequency and ROCOF trends  
- Analog channel values  

---

## Why Qt?
- Seamless C++ integration for efficient data handling.  
- Rich widget support and native performance for industrial GUIs.  
- Built-in charting & event handling for interactive visualizations.  
- Cross-platform deployment with minimal overhead.  

---

## Contributors
- A.D.V.M.S. Nikhil (2022EEB1165)  
- K. Sethu Madhav (2022EEB1186)  

Under the guidance of Prof. Bibhu Prasad Padhy, IIT Ropar.  

---
 
