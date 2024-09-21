#include "../../include/reads_list.h"
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

//! HINT: when designing the reads_list_element remember to
//! HINT: keep the critical region as small as necessary.
typedef struct reads_list_element
{
    //! HINT: something is missing here
    unsigned int    client_number;
    struct reads_list_element* next;
    struct reads_list_element* previous;

    //We need a counter to keep track of how many people are in wrting and reading
    int reads_list_counter;

} reads_list_element_t;


reads_list_element_t* head = NULL;

//! HINT: maybe global synchronization variables are needed
//Using a write-read mutex prbably better than a normal Mutex...
//pthread_mutex_t rw_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rw_list_lock = PTHREAD_RWLOCK_INITIALIZER;

//-----------------------------------------------------------------------------

int reads_list_insert_element(unsigned int client_number)
{
    //! HINT: synchronization is needed in this function   
     

    //! create new element
    reads_list_element_t* new_element = malloc(sizeof(reads_list_element_t));
    if(new_element == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    new_element->client_number = client_number;
    new_element->next     = NULL;
    new_element->previous = NULL;
    
    //Lock the list before modification
    if (pthread_rwlock_wrlock(&rw_list_lock) != 0){
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }

    //! insert element into list
    if(head == NULL)
    {
        head = new_element;
        pthread_rwlock_unlock(&rw_list_lock);


        return 0;
    }

    reads_list_element_t* temporary = head;
    while(temporary->next != NULL)
    {
        temporary = temporary->next;
    }
    new_element->previous = temporary;
    temporary->next       = new_element;

     //Unlock the list after insertion
    pthread_rwlock_unlock(&rw_list_lock);
    return 0;
}

//-----------------------------------------------------------------------------

sem_t* reads_list_get_reader_semaphore(unsigned int client_number)
{
    (void) client_number; //! Please remove this when you implement this function
    //! please implement this function
    return NULL; //! Please select a proper return value
    //Fehlt... 
}

//-----------------------------------------------------------------------------

void reads_list_increment_all() {

    //Lock the list before modification
    if (pthread_rwlock_wrlock(&rw_list_lock) != 0){
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }

    reads_list_element_t* temporary = head;
    while(temporary != NULL)
    {
        temporary->reads_list_counter++;
        temporary = temporary->next;

       
    } 
    //Unlock the list after incrementing
    pthread_rwlock_unlock(&rw_list_lock);
}
//-----------------------------------------------------------------------------

void reads_list_decrement(unsigned int client_number)
{
    //Lock the list before modification
    if (pthread_rwlock_wrlock(&rw_list_lock) != 0){
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }

    reads_list_element_t* temp = head;
    while (temp != NULL && temp->client_number != client_number) {
        temp = temp->next;
    }

    if (temp != NULL) {
        temp->reads_list_counter--;
    }

    //Unlock the list after decrementing
   pthread_rwlock_unlock(&rw_list_lock);
}

//-----------------------------------------------------------------------------

int reads_list_remove_reader(unsigned int client_number)
{
    
   

    //! find element to remove
    reads_list_element_t* temp = head;
    while(temp != NULL && temp->client_number != client_number)
    {
        temp = temp->next;
    }

    if(temp == NULL)
    {
        return -1;
    }
    //Lock the list before modification
    if (pthread_rwlock_wrlock(&rw_list_lock) != 0){
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }
    //! bend pointers around element
    if(temp->previous != NULL)
    {
        temp->previous->next = temp->next;
    }
    if(temp->next != NULL)
    {
        temp->next->previous = temp->previous;
    }
    if(temp == head)
    {
        head = temp->next;
    }

    //Unlock the list before deleting element
    pthread_rwlock_unlock(&rw_list_lock);

    //! finally delete element
    free(temp);
    return 0;
}

//-----------------------------------------------------------------------------

int reads_list_get_reads(unsigned int client_number)
{
    int buffer = 0;
    //Lock the list before accessing
    if (pthread_rwlock_wrlock(&rw_list_lock) != 0){
        perror("thread_unlock");
        exit(EXIT_FAILURE);
    }

    reads_list_element_t* temp = head;
    //Searching for the client.
    while(temp != NULL) {
        if (temp->client_number == client_number) {
            buffer = temp->reads_list_counter;
            pthread_rwlock_unlock(&rw_list_lock);
            //Return the counter if client is found
            return buffer;
        }
        temp = temp->next;
    }

    // Unlocking the list
    pthread_rwlock_unlock(&rw_list_lock);
    //Client not found
    return 0;
}
