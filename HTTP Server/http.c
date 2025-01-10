#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define RMAX 4096
#define HMAX 1024
#define BMAX 1024

static char request[RMAX+1];

static int HSIZE = 0;
static char header[HMAX];

static int BSIZE = 0;
static char body[BMAX];

char saved_data[1024];
size_t saved_data_size = 0;

void flush_socket(int clientfd) {
    char tmp[4096];
    while (recv(clientfd, tmp, sizeof(tmp), MSG_DONTWAIT) > 0) {
        // Consume any unread data
    }
}

static int open_listenfd(int port)
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        return -1;  // Error creating socket
    }


        struct sockaddr_in server ;
memset (& server , 0x00 , sizeof( server ));
        server.sin_family = AF_INET ;
        server.sin_port = htons (port);
        inet_pton ( AF_INET , "127.0.0.1", &( server . sin_addr ));
        int optval = 1;
setsockopt ( listenfd , SOL_SOCKET , SO_REUSEADDR , & optval , sizeof ( optval ));
        bind ( listenfd , ( struct sockaddr *)& server , sizeof ( server ));

      listen(listenfd, SOMAXCONN);
    return listenfd;
}

static void send_data(int clientfd, char buf[], int size)
{
ssize_t amt, total = 0;
do {
amt = send(clientfd, buf + total, size - total, 0);
total += amt;
} while (total < size);
}
static void send_response(int clientfd)
{
send_data(clientfd, header, HSIZE);
send_data(clientfd, body, BSIZE);
}

void handle_client(int clientfd) {
    char request[4096];
    memset(request, 0, sizeof(request));
    ssize_t received = recv(clientfd, request, sizeof(request) - 1, 0);

    if (received <= 0) {
        close(clientfd);
        return; // Handle connection close or error
    }

    // Null-terminate the received request

     BSIZE = 0;

    // Check for the presence of double CRLF indicating a valid HTTP request
    if (strstr(request, "\r\n\r\n") == NULL) {
        // Send 400 Bad Request response
        snprintf(header, sizeof(header),
                 "HTTP/1.1 400 Bad Request\r\n\r\n");
	HSIZE = strlen(header);;
        send_response(clientfd);
	flush_socket(clientfd);
	memset(header, 0, sizeof(header));
	memset(body, 0, sizeof(body));
	HSIZE = 0;
	BSIZE = 0;
	close(clientfd);
	request[0] = '\0';
        return;
    }

    // Handle POST /write
    if (strncmp(request, "POST /write", 11) == 0) {
        char *content_length_str = strstr(request, "Content-Length: ");
        if (!content_length_str) {
            snprintf(header, sizeof(header), "HTTP/1.1 400 Bad Request\r\n\r\n");
            HSIZE = strlen(header);
            BSIZE = 0;
            send_response(clientfd);

            // Clear buffers
            memset(header, 0, sizeof(header));
            memset(body, 0, sizeof(body));
            HSIZE = 0;
            BSIZE = 0;

            close(clientfd);
            return;
        }

        int content_length = atoi(content_length_str + 16);
        if (content_length < 0 || content_length > (int)sizeof(saved_data)) {
            snprintf(header, sizeof(header), "HTTP/1.1 413 Request Entity Too Large\r\n\r\n");
            HSIZE = strlen(header);
            BSIZE = 0;
            send_response(clientfd);

            // Clear buffers
            memset(header, 0, sizeof(header));
            memset(body, 0, sizeof(body));
            HSIZE = 0;
            BSIZE = 0;

            close(clientfd);
            return;
        }

        char *body_start = strstr(request, "\r\n\r\n") + 4;
        saved_data_size = (received - (body_start - request) < content_length)
                              ? received - (body_start - request)
                              : content_length;
        memcpy(saved_data, body_start, saved_data_size);

        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %zu\r\n\r\n",
                 saved_data_size);
        HSIZE = strlen(header);
        memcpy(body, saved_data, saved_data_size);
        BSIZE = saved_data_size;

        send_response(clientfd);
	memset(header, 0, sizeof(header));
	memset(body, 0, sizeof(body));
	HSIZE = 0;
	BSIZE = 0;

        close(clientfd);
        return;
    }else if (strncmp(request, "GET /read", 9) == 0) {
        // Prepare the response body
        strcpy(body, saved_data);
        BSIZE = saved_data_size;


        if (BSIZE == 0){


        strcpy(body,"<empty>");
        BSIZE = strlen(body);

        }

        // Prepare the response header
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %d\r\n\r\n",
                 BSIZE);
        HSIZE = strlen(header);



	

    
    	send_response(clientfd);
        memset(header, 0, sizeof(header));
        memset(body, 0, sizeof(body));
        HSIZE = 0;
        BSIZE = 0;
	return;
    }else if (strncmp(request, "GET /ping", 9) == 0) {
        // Prepare the response body
        strcpy(body, "pong");
        BSIZE = strlen(body);

        if (BSIZE == 0){
        strcpy(body,"<empty>");
	BSIZE = strlen(body);
        }
	
        // Prepare the response header
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %d\r\n\r\n",
                 BSIZE);
        HSIZE = strlen(header);





        send_response(clientfd);
        memset(header, 0, sizeof(header));
        memset(body, 0, sizeof(body));
        HSIZE = 0;
        BSIZE = 0;
        return;
    } 
    // Check if the request is for "/echo"
    else if (strncmp(request, "GET /echo", 9) == 0) {
        // Find the start and end of headers
        char *header_start = strstr(request, "\r\n");
        if (header_start != NULL) {
            header_start += 2; // Skip the initial "\r\n"
        } else {
            header_start = ""; // No headers, set to empty
        }

        char *header_end = strstr(header_start, "\r\n\r\n");
        if (header_end != NULL) {
            *header_end = '\0'; // Null-terminate at the end of headers
        }


        // Copy headers into the body for the response
        snprintf(body, sizeof(body), "%s", header_start);
        BSIZE = strlen(body);

        if (BSIZE > 1022) {
        BSIZE = 0;
            snprintf(header, sizeof(header),
                     "HTTP/1.1 413 Request Entity Too Large\r\n\r\n");
            HSIZE = strlen(header);

            send_response(clientfd);
            memset(header, 0, sizeof(header));
            memset(body, 0, sizeof(body));
            HSIZE = 0;
            BSIZE = 0;
            flush_socket(clientfd);
            close(clientfd);
            return; // Fully exit
        }



        // Prepare HTTP response header
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %d\r\n\r\n",
                 BSIZE);
        HSIZE = strlen(header);

        send_response(clientfd);
        memset(header, 0, sizeof(header));
        memset(body, 0, sizeof(body));
        HSIZE = 0;
        BSIZE = 0;
	close(clientfd);
	return;
    }else if (strncmp(request, "GET /", 5) == 0 && !strstr(request, "GET /read")) {
    // Extract the file path from the request
    char file_path[256] = {0};

    // Check if the request matches the expected format
    if (sscanf(request, "GET /%s HTTP/1.1", file_path) != 1) {
        // Handle invalid request format (400 Bad Request)
        strcpy(body, "400 Bad Request");
        BSIZE = 0;
        snprintf(header, sizeof(header),
                 "HTTP/1.1 400 Bad Request\r\n\r\n");
        HSIZE = strlen(header);
        send_response(clientfd);
        return;
    }

    FILE *file = fopen(file_path, "rb");
    if (file != NULL) {
        struct stat file_stat;
        if (fstat(fileno(file), &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
            // Valid file, build header
            snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Length: %ld\r\n\r\n",
                     file_stat.st_size);
            HSIZE = strlen(header);

            // Send header first
            send(clientfd, header, HSIZE, 0);

            // Read and send the file in chunks
            char chunk[1024];  // Chunk buffer
            size_t bytes_read;
            while ((bytes_read = fread(chunk, 1, sizeof(chunk), file)) > 0) {
                send(clientfd, chunk, bytes_read, 0);
            }

            fclose(file);
            return;
        } else {
            // File is not a regular file or some other error
            fclose(file);
        }
    }

    // Handle file not found or invalid file
    strcpy(body, "404 Not Found");
    BSIZE = 0;
    snprintf(header, sizeof(header),
             "HTTP/1.1 404 Not Found\r\n\r\n");
    HSIZE = strlen(header);
    send_response(clientfd);
}


}

int main(int argc, char * argv[])
{
    assert(argc == 2);
    int port = atoi(argv[1]);

        int listenfd = open_listenfd(port);

struct sockaddr_in client ;
socklen_t csize ;
memset (& client , 0x00 , sizeof( client ));
memset (& csize , 0x00 , sizeof( csize ));
while (1) {
int connfd = accept ( listenfd , ( struct sockaddr *)& client , & csize );
handle_client(connfd);
close (connfd);
}

    return 0;
}
