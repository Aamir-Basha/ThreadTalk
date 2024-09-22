#include "../../include/client.h"

pthread_t       reading_thread;
int             client_socket;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void connect_to_server()
{
    struct sockaddr_in server_addr;

    
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (!inet_aton("127.0.0.1", &server_addr.sin_addr)) {
        perror("Invalid IP address");
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server successfully.\n");
}



//-------------------------------------------------------------

void handshake()
{

    const char *message = "Handshake: Hello, I'm the client";
    char buffer[MAX_MESSAGE_LENGTH];
    ssize_t bytes_sent, bytes_received;

    //Send msg to the Server
    bytes_sent = send(client_socket, message, strlen(message), 0);
    if (bytes_sent < 0) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }

    //Recv the reply from Server and print it out
    bytes_received = recv(client_socket, buffer, MAX_MESSAGE_LENGTH - 1, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }
   printf("%s\n", buffer);
}

//-------------------------------------------------------------

void close_connection()
{
    while(1){
        if (signal(SIGINT, signal_handler)){
            printf("Goodbye! ;) \n");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }
}

//-------------------------------------------------------------

void send_message()
{
    char buff[MAX_USER_INPUT];
    char *user_input;
    while(1) {
        user_input = prompt_user_input(buff, MAX_USER_INPUT);

        // Lock the mutex to ensure thread safety
        int locking_sending = pthread_mutex_lock(&mutex);
        if(locking_sending != 0){

            perror("pthread_mutex_lock");
            exit(EXIT_FAILURE);
        }
        if(strcmp(user_input, "\n") == 0) {
            get_request();
        }
        else {
            set_request(user_input);
        }
        int unlocking_sending = pthread_mutex_unlock(&mutex);
                if (unlocking_sending != 0) {
                    perror("unlock thread");
                    exit_client(EXIT_FAILURE);
                }

    }


}

//-------------------------------------------------------------

void set_request(char* message)
{
    char ans[MAX_MESSAGE_LENGTH];
    char msg[MAX_MESSAGE_LENGTH];
    snprintf(msg, MAX_MESSAGE_LENGTH, "s:%s", message);

   

    int result = send(client_socket, msg, strlen(msg), 0);
    if (result == -1) {
        perror("send error");
        exit(EXIT_FAILURE);
    }

    ssize_t answer = recv(client_socket, ans, MAX_MESSAGE_LENGTH, 0);
    if(answer < 0){
         exit(EXIT_FAILURE);
    }
    

    if ((!strstr(ans, "r:ack"))) {
        fprintf(stderr, "%s", ans);
        exit(EXIT_FAILURE);
    }
    
    
}

//-------------------------------------------------------------

void get_request()
{
    const char *message = "g:";
    char buffer[MAX_MESSAGE_LENGTH];
    ssize_t bytes_sent, bytes_received;

    bytes_sent = send(client_socket, message, strlen(message), 0);
    if (bytes_sent < 0) {
        perror("Send failed");
        return;
    }

    bytes_received = recv(client_socket, buffer, MAX_MESSAGE_LENGTH - 1, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }
    
    
    if (strcmp(buffer, "r:nack") != 0) { 
        print_reply(buffer);
    }  
}

//-------------------------------------------------------------

void* read_continously(void* unused)
{
    

    while (1) {
        // Lock the mutex to ensure thread safety
        int locking_reading = pthread_mutex_lock(&mutex);
        if(locking_reading != 0){

            perror("pthread_mutex_lock");
            exit(EXIT_FAILURE);
        }
        //While locking keep reading then unlock after getting the req
            else {
                get_request();
                int unlock_reading = pthread_mutex_unlock(&mutex);
                if (unlock_reading != 0) {
                    perror("unlock thread");
                    exit_client(EXIT_FAILURE);
                }
            }
            sleep(READING_INTERVAL);
    }

    //this method should not return so dont care about return value
    return NULL; 
}

//-------------------------------------------------------------

void start_reader_thread()
{
   int error_no = pthread_create(&reading_thread, NULL, read_continously, NULL);
    if (error_no == -1)
    {
        perror("thread error");
    }
   
}
