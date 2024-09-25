
# Multi-threaded Server-Client Communication Project

### **Overview**

This project showcases a robust multi-threaded server-client communication system built in C. Designed for high concurrency, it allows multiple clients to simultaneously connect to a server, exchange messages using `GET` and `SET` requests, and perform efficient message handling using synchronization techniques.

### **Key Features**

- **Multi-threading**: Each client connection is handled by its own thread on the server, supporting simultaneous client interactions.
- **Socket-based Communication**: Reliable TCP sockets ensure a stable connection for seamless data exchange.
- **Thread-safe Synchronization**: Critical sections of code are protected using mutex locks for data integrity.
- **Custom Handshake Protocol**: Establishes a secure communication link between the server and client.
- **GET/SET Requests**: 
  - **SET**: Clients store messages on the server.
  - **GET**: Clients retrieve the latest messages from a ring buffer.
- **Error Handling**: Designed to gracefully manage errors like connection loss or invalid inputs.
- **Ring Buffer Implementation**: Efficiently stores and manages client messages.

---

### Project Structure**

```
├── include/
│   ├── client.h
│   ├── server.h
│   ├── ring_buffer.h
├── src/
│   ├── client.c
│   ├── server.c
│   ├── ring_buffer.c
├── Makefile
└── README.md
```

---

### Getting Started**

#### **Prerequisites**

- A Unix-based system (Linux/macOS) with `gcc` installed.
- Basic knowledge of C, socket programming, and multi-threading concepts.

#### **Compilation**

To compile the project, run the following command in your terminal:

```bash
make all
```

This will generate the `client` and `server` executables.

#### **Running the Server**

```bash
./server
```

This starts the server, which listens for client connections and handles their requests.

#### **Running the Client**

```bash
./client
```

The client connects to the server and begins communication, supporting both GET and SET operations.

---

### Workflow Overview**

#### **Server Flow**:
1. **Startup**: Initializes a socket and listens for incoming client connections.
2. **Client Handling**: Spawns a new thread for each connected client to manage their requests.
3. **Request Processing**: 
   - **GET**: Retrieves the latest message from the buffer.
   - **SET**: Stores a new message in the buffer.
4. **Shutdown**: Each client thread handles disconnection, ensuring resource cleanup.

#### **Client Flow**:
1. **Connection**: Establishes a TCP connection with the server.
2. **Requests**: Supports sending GET and SET requests to interact with the server.
3. **Threaded Communication**: A separate thread continuously reads messages from the server.
4. **Termination**: Ensures a graceful disconnection from the server.

---

### Key Concepts**

- **POSIX Threads (`pthread`)**: Enables multi-threading, allowing efficient, concurrent client handling.
- **Mutex Locks**: Used for thread-safe access to shared data, such as the ring buffer.
- **TCP Sockets**: Powers the server-client communication, ensuring reliable message delivery.
- **Ring Buffer**: A circular data structure for efficient message management across clients.

---

### Skills Demonstrated**

- Advanced **C programming** with a focus on low-level systems.
- Experience with **multi-threading** and **thread synchronization**.
- Proficient use of **socket programming** for building networked applications.
- Implementing and managing a **ring buffer** for efficient data handling.
- Graceful error handling and clean resource management in multi-threaded environments.

---

### Future Enhancements**

- **Secure Communication**: Implement SSL/TLS for encrypted data exchange.
- **Scalability**: Enhance the server to handle higher client loads with dynamic resource allocation.
- **Expanded API**: Add support for more request types (e.g., UPDATE, DELETE) to improve the client-server interaction model.

---
