# Limit Order Book (LOB) Dashboard

## Overview
The **Limit Order Book (LOB) Dashboard** is a high-performance, multithreaded simulation of a financial exchange's order book. It features real-time order matching, latency benchmarking, and a modern GUI built with Dear ImGui. This project is designed to showcase expertise in systems programming, multithreading, and financial systems, making it highly relevant for roles in high-frequency trading (HFT) and quantitative finance.

## Key Features

### 1. **Order Matching Engine**
- Implements a price-time priority matching algorithm.
- Supports **limit orders**, with plans to extend to market orders, IOC, and FOK.
- Handles partial fills and maintains a live bid/ask depth.

### 2. **Multithreaded Architecture**
- **Producers:** Simulate multiple traders generating random orders.
- **Consumer:** Matches incoming orders in real-time.
- Thread-safe data structures ensure high concurrency and low latency.

### 3. **Latency Benchmarking**
- Measures and visualizes:
  - **Queue Push Latency** (order submission).
  - **Queue Pop Latency** (order retrieval).
  - **Order Matching Latency**.
  - **GUI Frame Latency**.
- Real-time metrics displayed in the GUI, including average, min, and max latencies.

### 4. **Modern GUI**
- Built with **Dear ImGui**, GLFW, and OpenGL3.
- Displays:
  - Live bid/ask depth.
  - Performance metrics and latency plots.
  - Alerts for matcher thread crashes or completion.

### 5. **Extensibility**
- Modular design allows easy addition of new order types, matching algorithms, and risk checks.
- Codebase is well-documented and follows modern C++ best practices.

## Screenshots
![Order Book Dashboard](data/sample_screenshot.png)

## Performance Highlights
- **Throughput:** Handles 100,000+ orders per second in simulation.
- **Low Latency:** Optimized for minimal locking and efficient data structures.
- **Scalability:** Supports multiple producer threads for high order flow.

## Technical Stack
- **Programming Language:** C++ (Modern C++17/20 features).
- **Libraries:**
  - [Dear ImGui](https://github.com/ocornut/imgui) for GUI.
  - GLFW and OpenGL3 for rendering.
- **Concurrency:** std::thread, std::mutex, and std::atomic.
- **Data Structures:** Custom thread-safe queue and order book.

## How to Run

### Prerequisites
- **Linux** (tested on Ubuntu 20.04+).
- C++17-compatible compiler (e.g., GCC 9+).
- OpenGL3 and GLFW installed.

### Build Instructions
1. Clone the repository:
   ```bash
   git clone https://github.com/your-repo/LimitBookOrder.git
   cd LimitBookOrder
   ```
2. Build the project:
   ```bash
   make
   ```
3. Run the executable:
   ```bash
   ./lob
   ```

### Optional: Run with Sample Data
- Use the provided `data/sample_orders.csv` to simulate historical order flow.

## Future Enhancements
- **Networking:** Add FIX/ITCH protocol support for real-time market data.
- **Order Types:** Extend to market orders, stop orders, and advanced order types.
- **Risk Management:** Implement pre-trade risk checks.
- **Backtesting:** Replay historical data for strategy testing.
- **Visualization:** Add heatmaps and advanced analytics.

## Why This Project?
This project demonstrates:
- **Low-latency systems design:** Critical for HFT and trading systems.
- **Concurrency and multithreading:** Efficient handling of high-frequency order flow.
- **Financial domain knowledge:** Understanding of order books and matching algorithms.
- **Modern C++ expertise:** Clean, modular, and extensible codebase.
---

