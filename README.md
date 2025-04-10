# RetroChat - Terminal Based Chat Application

RetroChat is a terminal-based real-time chat application built using C++. It allows multiple clients to communicate through a central server without the need for port forwarding or complex setups.

> Think of it like old-school LAN chat but over the internet!

---

## Features
- Real-time chatting between multiple clients
- Server-Client architecture
- No port forwarding needed on client side
- Terminal-friendly interface
- Simple command-based usage

---

## Tech Stack
- C++ (Networking & Multithreading)
- Sockets (TCP)
- Windows based terminal app
- Server mediates communication between clients

---

## Project Structure
```
├── RetroChat/
│   ├── server.c
│   ├── client.c
│   ├── README.md
```
## Setup & Run

### 1. Clone the repository
```bash
git clone https://github.com/AshutoshThings/RetroChat.git
cd RetroChat
```
## Setup & Run (Windows)

### 2. Compile

Use any of the following methods:

- Visual Studio (Recommended)

- g++ with MinGW

- Command line build (Example):
```bash
g++ server.cpp -o server -lws2_32
g++ client.cpp -o client -lws2_32
```
-lws2_32 links the Windows Sockets (Winsock) library required for networking.

### 3. Run the Server
```bash
./server
```
### 4. Run the Client
```bash
./client
```

Made with ❤️ by AshutoshThings
