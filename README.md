# SERVIDOR

**SERVIDOR** is a C program that implements a server using both TCP and UDP sockets for inter-process communication (IPC). It is designed to handle multiple clients simultaneously, demonstrating the use of sockets, threads, and linked lists to manage client connections. The server can process requests via TCP and UDP protocols and support messaging, including channel-based communication.

## Functionality

### 1. **Connection Types:**
   - **TCP**: Handles individual connections from clients using the `accept()` system call and assigns each client a new thread to manage their requests.
   - **UDP**: Listens for client messages and processes them using connectionless communication. Each UDP client is also assigned a thread.

### 2. **Threaded Client Handling:**
   - For each new client connection (both TCP and UDP), the server spawns a **new thread** to handle communication with that client. This ensures that multiple clients can interact with the server simultaneously.
   - Threads handle sending and receiving messages, responding to commands such as `NICK`, `USER`, `JOIN`, `PART`, and `PRIVMSG`.

### 3. **Client Management:**
   - **TCP Clients**: Managed through a linked list of `Cliente` structs, which store client-specific information such as socket number, nickname, username, and the channels they have joined.
   - **UDP Clients**: Similarly managed through a linked list of `ClienteUDP` structs, storing client socket and address details.

### 4. **Commands:**
   - **NICK**: Assigns a nickname to a client.
   - **USER**: Assigns a username to a client.
   - **JOIN**: Allows a client to join a channel (denoted by `#channel`).
   - **PART**: Lets a client leave a channel.
   - **PRIVMSG**: Sends a private message to either a channel or another client.
   - **QUIT**: Disconnects a client from the server.

### 5. **Concurrency and Synchronization:**
   - The server uses **mutexes** (`pthread_mutex_t`) to synchronize access to shared resources such as the linked lists and the log file to prevent race conditions between threads.
   
### 6. **Logging:**
   - The server logs connection events (client connections, disconnections, and command executions) to a log file (`ircd.log`). Mutexes are used to ensure thread-safe logging.

## Key Components

- **Linked Lists**:
   - TCP and UDP clients are managed through dynamically allocated linked lists, allowing the server to handle an arbitrary number of connections.
   - Functions such as `crearVacia()`, `insertarNodoFinal()`, and `eliminarNodo()` manage the client lists.

- **Client Threads**:
   - Each client is handled by a dedicated thread (`client_handler` for TCP and `clientUDP_handler` for UDP), enabling concurrent processing of client requests.
   
- **Socket Management**:
   - The server sets up both **TCP** and **UDP** sockets to listen on the specified port (**6821**) and handles client requests accordingly.

- **Error Handling**:
   - The server gracefully handles errors, ensuring that clients are properly disconnected in case of failure. Specific functions like `errout()` are used for this purpose.

## How to Run

Compile and execute the program as follows:
```bash
./servidor
```
The server will bind to port 6821 and wait for incoming TCP and UDP connections. It supports multiple clients simultaneously, using threads to manage each connection.

## Example

After starting the server, it can accept both TCP and UDP clients for messaging. Clients can send commands like `NICK`, `USER`, `JOIN`, `PART`, and `PRIVMSG` to interact with the server and other clients.

##Server Features

- **Multi-client handling**: Supports simultaneous connections via threads.
- **Channel-based communication**: Allows clients to join channels and send messages to all channel members.
- **Private messaging**: Supports direct messages between clients.
- **Logging**: Keeps track of all client activities, including connections, disconnections, and message exchanges.

# CLIENTCP

**CLIENTCP** is a C program that implements a client using TCP/IP sockets. It is designed to work in conjunction with a server program, demonstrating the use of stream sockets as an Inter-Process Communication (IPC) mechanism. This program sends and receives messages over a network connection.

## Functionality

1. **Connection to the Server:**
   - The client connects to a remote server via a TCP socket on port **6821**.
   - The server’s IP is resolved using the `getaddrinfo` function, which is provided as a command-line argument.

2. **Sending and Receiving Messages:**
   - The client can **send messages** or commands to the server through the `send_msg()` function, which reads lines from a file specified as an argument.
   - It can also **receive messages** from the server using the `recv_msg()` function, displaying them in the console.
   - If an error message or the word **"SALIDA"** (exit) is received, the client disconnects.

3. **Threading:**
   - Two independent threads are created:
     - One thread for **sending messages** (`send_msg`).
     - Another thread for **receiving messages** (`recv_msg`).

4. **Connection Termination:**
   - If the message "QUIT" is sent or received, the client safely terminates the connection using `shutdown` and `close`.

## Key Components

- **`str_trim_lf`:** A helper function that removes newline characters from messages.
- **`recv_msg`:** Handles receiving messages from the server. If it receives "ERROR" or "SALIDA", the client stops.
- **`send_msg`:** Sends messages from a file to the server. It checks for the "QUIT" command to close the connection.
- **Error handling:** The program prints error messages if socket creation or connection fails.

## How to Run

Compile and execute the program as follows:
```bash
./clientcp <server_name> <file_name>
```
Where:
- `<server_name>` is the IP address or hostname of the server.
- `file_name` is the file containing the commands to be sent to the server.

## Example

```bash
./clientcp 192.168.1.100 commands.txt
```

# CLIENTUDP

**CLIENTUDP** is a C program that implements a client using UDP sockets. It is designed to work in conjunction with a server program, demonstrating the use of sockets as an Inter-Process Communication (IPC) mechanism. This program sends and receives messages over a network connection.

## Functionality

1. **Connection to the Server:**
   - The client sends messages to a remote server via a UDP socket on port **6821**.
   - The server’s IP is resolved using the `getaddrinfo` function, which is passed as a command-line argument.
   - The server responds with the corresponding internet address or an error if the host is not recognized.

2. **Sending and Receiving Messages:**
   - The client can **send messages** from a file specified as an argument using the `send_msg()` function.
   - It can also **receive messages** from the server via the `recv_msg()` function and print them to the console.
   - If the client receives the messages **"ERROR"** or **"SALIDA"**, it terminates the connection.

3. **Threading:**
   - Two independent threads are created:
     - One thread for **sending messages** (`send_msg`).
     - Another thread for **receiving messages** (`recv_msg`).

4. **Connection Setup:**
   - Upon startup, the client sends an acknowledgment (`ACK`) to the server to initiate the communication.
   - The server replies with an acknowledgment, and if successful, the client begins message transmission.

5. **Connection Termination:**
   - If the message "QUIT" is encountered in the file, the client stops sending and the connection closes.

## Key Components

- **`str_trim_lf`:** A helper function that removes newline characters from the messages.
- **`recv_msg`:** Handles receiving messages from the server. Terminates if "ERROR" or "SALIDA" is received.
- **`send_msg`:** Sends messages from a file to the server. Checks for the "QUIT" command to end communication.
- **Error handling:** Error messages are printed if socket creation, binding, or communication fails.

## How to Run

Compile and run the program as follows:
```bash
./clientudp <server_name> <file_name>
```
Where:
- `<server_name>` is the IP address or hostname of the server.
- `file_name` is the file containing the commands to be sent to the server.

## Example

```bash
./clientcp 192.168.1.100 commands.txt
```


