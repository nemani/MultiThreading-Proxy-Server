// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>

#define min(x, y) ((x) < (y) ? (x) : (y))
#define PORT 8080
#define NUM_FILES 3
#define false 0

void read_file() {

}


typedef void (*RecieveDataCallback)(void *buf, uint32_t size, void *data);

// Recieve data from sockfd, of total size total_size.
// Optional: recieveDataCallback - call the callback every time data of BUFFER_SIZE is streamed in, if it is not NULL
// Optiona: out - data will be written to "out" pointer if it is not NULL
// Optional: data - any data for callback as required.
void recieve_data_handler(const int sockfd, RecieveDataCallback cb, const uint32_t total_size,  char *out, void *data) {
    #define BUFFER_SIZE 32
    char buffer[BUFFER_SIZE];
    int left = total_size;
    do {
        const int read_size = read(sockfd, buffer, min(BUFFER_SIZE, left));
        if (read_size <= 0) {
            assert(false && "read nothing");
        }

        if (cb) {
            assert(cb && "corrupted callback pointer");
            cb(buffer, read_size, data);
        }
        left -= read_size;

        if (out) {
            assert(out && "corrupted out pointer");
            memcpy(out, buffer, read_size);
            out += read_size;
        }
    } while (left > 0);

}

uint32_t receive_int(int *num, const int sockfd)
{
    int32_t raw_int = -42;
    recieve_data_handler(sockfd, NULL, sizeof(int32_t),  &raw_int, NULL);
    assert (raw_int != -42);
    *num = ntohl(raw_int);
    return 0;
}

typedef struct FileData {
    FILE *fp;
} FileData;

void recieveFileDataCallback(void *buf, uint32_t size, void *vd) {
    const FileData *data = (FileData *)vd;
    fwrite(buf, size, 1, data->fp);
    printf("wrote (%d bytes) into file.\n", size);
}

const char *file_names[] = {"file1.txt", "file2.txt", "file3.txt"};
int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;


    for(int i = 0; i < 3; i++ ){

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }

        memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
        // which is meant to be, and rest is defined below

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Converts an IP address in numbers-and-dots notation into either a 
        // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
        {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
        {
            printf("\nConnection Failed \n");
            return -1;
        }

        int file_size = -42;
        receive_int(&file_size, sock);
        assert (file_size != -42 && "fuckery has happened, this should have become the file size");
        assert (file_size >= 0 && "file size cannot be negative");
        printf("file size received: %d\n", file_size);

        FILE *fp = fopen(file_names[i], "wb");
        assert(fp != NULL);
        FileData fdata = { fp };
        printf("# beginning transfer of: %s\n", file_names[i]);
        recieve_data_handler(sock, recieveFileDataCallback, file_size, /*out = */ NULL, &fdata);
        fclose(fp);
        close(sock);
    }

    
    return 0;
}
