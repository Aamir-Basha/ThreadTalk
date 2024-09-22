#include "../../include/client.h"

int main(void)
{

    
    connect_to_server();

    
    handshake();

   
    start_reader_thread();

    
    send_message();

    
    close_connection();
    return EXIT_FAILURE;
}
