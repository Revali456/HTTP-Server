# HTTP Server

This project is a simple multi-functional HTTP server implemented in C. It supports basic HTTP requests like `GET`, `POST`, and custom routes, making it an excellent learning tool for network programming and socket handling in C.

## Features

### Supported HTTP Routes

1. **GET /ping**
   - Responds with `pong`.

2. **GET /read**
   - Responds with the saved data from the server. If no data is saved, it returns `<empty>`.

3. **POST /write**
   - Saves the body content of the request into a server-side buffer.
   - Returns the saved content in the response.

4. **GET /echo**
   - Echoes back the headers of the received HTTP request.

5. **GET /[filename]**
   - Serves files located in the server's directory if the file exists.
   - Returns a `404 Not Found` response if the file is missing.

### Error Handling
- Handles malformed requests with a `400 Bad Request` response.
- Limits POST data to a fixed size and responds with `413 Request Entity Too Large` for oversized requests.
- Implements a graceful error handling mechanism to ensure invalid requests or errors do not crash the server.

## How It Works

1. The server uses **POSIX sockets** to listen for incoming connections on a specified port.
2. It parses incoming HTTP requests and routes them based on the request type and path.
3. Depending on the route, it prepares a response, sends it back to the client, and closes the connection.

### Key Functions

- `open_listenfd(port)`: Creates and binds a listening socket.
- `handle_client(clientfd)`: Processes incoming client requests.
- `send_response(clientfd)`: Sends HTTP headers and body data to the client.
- `flush_socket(clientfd)`: Ensures no unread data remains in the socket buffer.

## Prerequisites

- A Linux-based system with GCC installed.
- Basic understanding of HTTP and network programming.

## Compilation and Usage

1. Compile the server:
   ```bash
   gcc -o http_server http.c -Wall -Wextra -pedantic
   ```

2. Run the server on a specified port:
   ```bash
   ./http_server <port>
   ```
   Replace `<port>` with the desired port number (e.g., 8080).

3. Send HTTP requests using tools like `curl` or a web browser:
   ```bash
   curl -X POST -d "Hello, World!" http://127.0.0.1:<port>/write
   curl http://127.0.0.1:<port>/read
   curl http://127.0.0.1:<port>/ping
   ```

## Limitations

- File-serving functionality is limited to files in the current working directory.
- Data saved via `POST /write` is lost when the server restarts.
- Not suitable for production use; designed as a learning project.

## License

This project is licensed under the MIT License. Feel free to use, modify, and distribute it as per the terms of the license.

## Contributing

Contributions are welcome! Feel free to fork the repository and submit a pull request with your changes.

## Acknowledgments

- Inspired by classic examples of HTTP servers in C.
- Thanks to the open-source community for resources and documentation.

