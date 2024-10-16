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


