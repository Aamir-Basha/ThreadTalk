#include "../../include/ring_buffer.h"
#include "../../include/reads_list.h"
#include "../../include/server.h"

#include <stdbool.h> //Adding this too use Bool
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



typedef struct ring_buffer_element
{

    char text[MAX_MESSAGE_LENGTH];
    int            reader_counter;

} ring_buffer_element_t;


static ring_buffer_element_t ring_buffer[RINGBUFFER_SIZE];

unsigned int current_writer = 0;

unsigned int number_of_readers = 0;

//! HINT: maybe global synchronization variables are needed
pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;

void ringbuffer_initialize(void)
{
    for (int i = 0; i < RINGBUFFER_SIZE; i++)
    {
        ring_buffer[i].reader_counter = 0;
    }
    current_writer = 0;
    number_of_readers = 0;
}

int ringbuffer_write(char* text)
{
    //Lock before trying to handle the write buffer
    if (pthread_rwlock_wrlock(&rw_lock) != 0) {
        perror("wrlock");
        exit(EXIT_FAILURE);
    }

    if (ring_buffer[current_writer % RINGBUFFER_SIZE].reader_counter != 0) {
        pthread_rwlock_unlock(&rw_lock);
        return -1;
    }

    
    strcpy(ring_buffer[current_writer % RINGBUFFER_SIZE].text, text);
    
    ring_buffer[current_writer % RINGBUFFER_SIZE].reader_counter = number_of_readers;
    
    reads_list_increment_all();
    current_writer++;


    //Unblock after writing (if there was something to write)
    if (pthread_rwlock_unlock(&rw_lock) != 0) {
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void ringbuffer_read(int* current_reader, char* buffer, unsigned int client_number)
{
    //Lock before trying to handle the read buffer
    if (pthread_rwlock_wrlock(&rw_lock) != 0) {
        perror("rdlock");
        exit(EXIT_FAILURE);
    }
    
    bool hasCaughtUp = *current_reader >= (int)current_writer;
    bool noMessagesLeft = reads_list_get_reads(client_number) == 0;

    if (hasCaughtUp || noMessagesLeft) {
        strcpy(buffer, "nack");
        pthread_rwlock_unlock(&rw_lock);
        return;
    }



    int reader = *current_reader % RINGBUFFER_SIZE;
    strcpy(buffer, ring_buffer[reader].text);
    ring_buffer[reader].reader_counter--;
    reads_list_decrement(client_number);
    (*current_reader)++;

    //Unblock after adjusting the buffer
    if(pthread_rwlock_unlock(&rw_lock) != 0){
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }
}

int ringbuffer_add_reader(unsigned int client_number)
{
    
   

    if (reads_list_insert_element(client_number) != 0)
    {
        exit(EXIT_FAILURE);
    }
    //lock the list before adding the reader
    if (pthread_rwlock_wrlock(&rw_lock) != 0) {
        perror("wrlock");
        exit(EXIT_FAILURE);
    }
    int new_reader = current_writer;
    number_of_readers++;

    //Unlock the list after adding the reader
    if (pthread_rwlock_unlock(&rw_lock) != 0) {
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }

    return new_reader;
}

void ringbuffer_remove_reader(int* current_reader, unsigned int client_number)
{
    int read = reads_list_get_reads(client_number);

    while (read != 0)
    {
        char buffer[MAX_MESSAGE_LENGTH];
        ringbuffer_read(current_reader, buffer, client_number);
        read = reads_list_get_reads(client_number);
    }

    reads_list_remove_reader(client_number);

    if (pthread_rwlock_wrlock(&rw_lock) != 0) {
        perror("wrlock");
        exit(EXIT_FAILURE);
    }

    number_of_readers--;

    //Unlock the list after removing the reader
    if (pthread_rwlock_unlock(&rw_lock) != 0) {
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }
    
}
