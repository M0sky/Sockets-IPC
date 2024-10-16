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

