#include "../../../include/server.h"
#include "../../../include/ring_buffer.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

uint16_t get_port_number() {
    u_int16_t port = 0;
    const char* username = getenv("USER");
    if (username == NULL)
    {
        return 31026;
    }
    char c;
    while ((c = *username) != '\0') {
        port = (port << 1) + port +(u_int16_t)c;
        username++;
    }
    return 31026 + (port % 4096);
}


unsigned int    client_number_count = 0;


//! HINT: maybe global synchronization variables are needed
//! HINT: but try to keep the critical region as small as possible

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int initialize_server()
{

   
    //! implement this function
    struct sockaddr_in server_addr;

    //Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket");
        return -1; 
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket to address");
        close(server_socket); //Close the socket to release resources
        return -1; 
    }

    if (listen(server_socket, 25) == -1) {
        perror("Listening");
        close(server_socket); //Close the socket to release resources
        return -1;
    }


    return server_socket;
}

//-----------------------------------------------------------------------------

int handshake(int file_descriptor)
{

    //! Read message from client
    char    message_buffer[MAX_MESSAGE_LENGTH];
    ssize_t message_lenght = 0;
    if((message_lenght = read(file_descriptor, message_buffer, MAX_MESSAGE_LENGTH - 1)) < 1)
    {
        return -1;
    }
    message_buffer[MAX_MESSAGE_LENGTH - 1] = '\0'; //! Enforce NULL Terminated string
    printf("Handshake: %s\n", message_buffer);

    //! Reply message to client
    const char server_message[] = SERVER_HANDSHAKE_MSG;
    if(write(file_descriptor, server_message, sizeof(server_message)) < 0)
    {
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------

void* handle_connection(void* socket)
{
    //! HINT: Synchronization is needed in this function
    int file_descriptor = *(int*)socket;
    free(socket); // can also be freed when thread exits

    unsigned int client_number;

    // Lock the mutex before accessing the shared variable
    if (pthread_mutex_lock(&lock) != 0) {
        perror("lock Error");
        close(file_descriptor);
        return NULL;
    }

    client_number = client_number_count;
    client_number_count++;

    // Unlock the mutex after accessing the shared variable
    if (pthread_mutex_unlock(&lock) != 0) {
        perror("unlock Error");
        close(file_descriptor);
        return NULL;
    }

    printf("adding reader no %d\n", client_number);

    //! get reader number for thread
    int thread_reader = ringbuffer_add_reader(client_number);

    //! handshake
    if(handshake(file_descriptor) < 0)
    {
        printf("handshake failed \n");
        ringbuffer_remove_reader(&thread_reader, file_descriptor);
        close(file_descriptor);
        return NULL;
    }

    void* buffer = malloc(sizeof(char) * (MAX_MESSAGE_LENGTH));
    if(buffer == NULL)
    {
        perror("malloc");
        ringbuffer_remove_reader(&thread_reader, file_descriptor);
        close(file_descriptor);
        return NULL;
    }
    //! main loop for each thread to continuesly read from file descriptor and handle input
    while(1)
    {

        //! clean the buffer
        memset(buffer, 0, MAX_MESSAGE_LENGTH);

        int   number_of_read_bytes = read(file_descriptor, buffer, MAX_MESSAGE_LENGTH - 1);
        char* message              = (char*)buffer;
        message[MAX_MESSAGE_LENGTH - 1]   = '\0';

        //! error handling for read
        if(number_of_read_bytes < 0)
        {
            perror("read");
            break;
        }
        else if(number_of_read_bytes == 0)
        {
            printf("closing connection\n");
            break;
        }
        //! handle clients input
        if(handle_input(client_number, message, file_descriptor, &thread_reader) != 0)
        {
            break;
        }
    }
    ringbuffer_remove_reader(&thread_reader, client_number);
    if(close(file_descriptor) < 0)
    {
        perror("close");
    }
    free(buffer);
    return NULL;
}

//-----------------------------------------------------------------------------

int handle_input(int client_number, char* input, int socket, int* current_reader_pointer)
{

    const char* error_message_1       = "r:invalid input: short message";
    const char* error_message_2       = "r:invalid input: unknown message type";
    const char* write_error_message   = "r:nack";
    const char* write_success_message = "r:ack";

    //! check message length
    if(strlen(input) < 2)
    {

        if(write(socket, error_message_1, strlen(error_message_1) + 1) < 0)
        {
            perror("write");
            return -1;
        }
        return 0;
    }

    //! check first two chars of message
    char control_character = input[0];
    char delimiter         = input[1];
    if(delimiter != ':' || !(control_character == 'g' || control_character == 's'))
    {
        printf("invalid input\n");
        if(write(socket, error_message_2, strlen(error_message_2) + 1) < 0)
        {
            perror("write");
            return -1;
        }
        return 0;
    }

    char* message = ++input;
    message++;

    //! handle GET request
    if(control_character == 'g')
    {
        char buffer[MAX_MESSAGE_LENGTH - 2];

        ringbuffer_read(current_reader_pointer, buffer, client_number);

        printf("client %d read: %s length %zu\n", client_number, buffer, strlen(buffer));

        char message[MAX_MESSAGE_LENGTH] = "r:";

        strcat(message, buffer);

        if(write(socket, message, strlen(message) + 1) < 0)
        {
            perror("write");
            return (-1);
        }
        //! handle set request
    }
    //! handle SET request
    else if(control_character == 's')
    {

        //! write in reingbuffer
        int write_ack = ringbuffer_write(message);

        //! write failed
        if(write_ack != 0)
        {
            if(write(socket, write_error_message, strlen(write_error_message) + 1) < 0)
            {
                perror("write");
                return -1;
            }
            printf("client %d write failed\n", client_number);
        }
        //! write success
        else
        {
            if(write(socket, write_success_message, strlen(write_success_message) + 1) < 0)
            {
                perror("write");
                return -1;
            }
            printf("client %d write: %s length %zu\n", client_number, message, strlen(message));
        }
    }
    else
    {
        printf("an unknown error occured\n");
        return (-1);
    }

    return 0;
}

//-----------------------------------------------------------------------------

void accept_connections(int socket_number)
{
    int new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    while (1) {
        if ((new_socket = accept(socket_number, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed!");
            break;
            continue;
        }

        pthread_t thread_id;
        int *new_sock = malloc(sizeof(int));

        if (new_sock == NULL) {
            perror("Malloc failed!");
            close(new_socket); 
            continue;
        }
        *new_sock = new_socket;

        if (pthread_create(&thread_id, NULL, handle_connection, (void*)new_sock) != 0) {
            perror("Could not create thread");
            free(new_sock);
            close(new_socket);
        }
        pthread_detach(thread_id);
    }
}
