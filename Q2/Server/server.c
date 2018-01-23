#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>

#define min(x, y) ((x) < (y) ? (x) : (y))


#define false 0
#define PORT 8080

#define NUM_FILES 3


//  VVVVVVVV Pure stuff, written in Zen.
uint32_t get_file_size(const char *name) {
    FILE *fp = fopen(name, "rb");
    if (fp == NULL) {
        fprintf(stderr, "unable to open file: %s. ", name);
        assert (false && "unable to find file, quitting");
    }

    fseek(fp, 0L, SEEK_END);
    const unsigned long long size = ftell(fp);
    fclose(fp);
    return size;
}

void send_file(const int sockfd, const char *filename, const unsigned long filesize) {
// TODO: add progress bar for extra points.
#define BUFFER_SIZE 32
    char buffer[BUFFER_SIZE];
    FILE *fp = fopen(filename, "rb");
    unsigned long long consumed = 0;

    while (consumed < filesize) {
        const int left = filesize - consumed;
        assert (left > 0);
        const unsigned  cur_consumed = fread(buffer, 1, min(left, BUFFER_SIZE), fp);
        consumed += cur_consumed;
        send(sockfd , buffer , cur_consumed, 0 );
        printf("sent: %d\n", cur_consumed);
    }
    assert (consumed == filesize && "somehow consumed more data than the file size");
#undef BUFFER_SIZE
}

void send_file_size(const int sockfd, const uint32_t file_size) {
    const uint32_t converted_size = htonl(file_size);
    send(sockfd , &converted_size ,sizeof(converted_size), 0 );
}


//  VVVVVVVV Impure stuff that mutates global state VVVVVVVVVVVVV
const char *file_names[3] = {"Data/file1.txt", "Data/file2.txt", "Data/file3.txt" };
uint32_t file_sizes[NUM_FILES];

void read_file_sizes() {
    for(int i = 0; i < NUM_FILES; i++) {
        file_sizes[i] = get_file_size(file_names[i]);
    }
}


int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  // sockaddr_in - references elements of the socket address. "in" for internet
    int opt = 1;
    int addrlen = sizeof(address);

    
    read_file_sizes();
    for(int i = 0; i < 3; i++) {

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // This is to lose the pesky "Address already in use" error message
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                    &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
        address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
        address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian


        // Forcefully attaching socket to the port 8080
        if (bind(server_fd, (struct sockaddr *)&address,
                    sizeof(address))<0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        // Port bind is done. You want to wait for incoming connections and handle them in some way.
        // The process is two step: first you listen(), then you accept()
        if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                        (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf ("sending file size: %s - value: %d\n", file_names[i], file_sizes[i]);
        send_file_size(new_socket, file_sizes[i]);
        printf("sending file: %s...\n", file_names[i]);
        send_file(new_socket, file_names[i], file_sizes[i]);

        close(server_fd);
    }

    return 0;
}
