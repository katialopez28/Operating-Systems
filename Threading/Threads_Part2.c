// Katia Lopez

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#define NUM_CHARACTERS 5
#define NONSHARED 1
#define MAX 50000

/*Part 2 (35 pts) Use condition variables to
implement the producer-consumer algorithm.

Assume two threads: one producer and one consumer.

The producer reads characters one by one from a string
stored in a file named "message.txt", then writes
sequentially these characters into a circular queue.

Meanwhile, the consumer reads sequentially from the
queue and prints them in the same order.

Assume a buffer (queue) size of 5 characters.*/


sem_t character_checked_out, characters_in_line;

//adding a semaphore as a mutex to guard the critical section
sem_t mutex;

// Creating a circular queue with size of 5 char
char circularQueue[5];
int front = -1, rear = -1;

//String length of message
int n1;
//String of the file
char *s1;


FILE *fp;
int readf(char* filename)
{
    if((fp=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can't open %s!\n", filename);
        exit(0);
    }

    s1=(char *)malloc(sizeof(char)*MAX);

    if (s1==NULL)
    {
        printf ("ERROR: Out of memory!\n") ;
        exit(0);
    }

    /*read s1 from the file*/
    s1=fgets(s1, MAX, fp);
    n1=strlen(s1); /*length of s1*/

    if(s1==NULL) /*when error exit*/
    {
        exit(0);
    }
}


// Producer doesn't need to output
// just needs to push characters on to the queue
void * Producer()
{
    int i;

    // will use for loop to insert a max of NUM_CHARACTERS into queue
    for (i=0; i< NUM_CHARACTERS; i++)
    {
        // Will produce a new character onto the queue
        sem_wait(&character_checked_out);

        // Down semaphore
        sem_wait(&mutex);

        //Instead of using rand to create a new customer
        //Just add new characters from the string

        //Check to see if queue is full
        if ((front == rear + 1) || (front == 0 && rear == NUM_CHARACTERS - 1))
        {
            printf("Queue is full\n");
        }
        else if (n1<NUM_CHARACTERS && i==n1)
        {
            //if string length of input is less than NUM_CHARACTERS
            //and char we are trying to access is at index n1
            //skip the rest of the loop but increment using sem_post
            sem_post(&mutex);
            sem_post( &characters_in_line );
            break;
        }
        else //Adding a character to the queue
        {
            if (front == -1) front = 0;
            rear = (rear + 1) % NUM_CHARACTERS;
            char element = s1[i]; //getting char from input string

            //inserting element to the queue
            circularQueue[rear] = element;
        }

        // Up semaphore
        sem_post(&mutex);

        //increment number of characters in "line" (queue)
        sem_post( &characters_in_line );
    }

    return NULL;
}



void * Consumer()
{
    int i;
    for (i=0; i<NUM_CHARACTERS; i++)
    {
        // Wait here for a character to appear in line
        sem_wait(&characters_in_line);

        // Down semaphore
        sem_wait(&mutex);

        // Will consume one character from queue
        char element;
        if (front == -1) //empty
        {
            printf("Queue is empty\n");
        }
        else if (n1<NUM_CHARACTERS && i==n1)
        {
            //if string length of input is less than NUM_CHARACTERS
            //and char we are trying to access is at index n1
            //skip the rest of the loop but increment using sem_post
            sem_post(&mutex);
            sem_post( &character_checked_out );
            break;
        }
        else
        {
            element = circularQueue[i];
            // Consumer prints the single char it consumes
            // print one character per line
            printf("%c \n", element);

        }

        // Up semaphore
        sem_post(&mutex);

        sem_post( &character_checked_out );

        // Sleep a little bit so we can read the output on the screen
        sleep(1);
    }

    return NULL;
}


int main(int argc, char *argv[])
{
    if(argc<2 )
    {
      printf("Error: You must pass in the datafile as a command line parameter\n");
    }
    readf (argv[1]);

    pthread_t producer_tid;
    pthread_t consumer_tid;

    sem_init(&mutex, 0, 1);      /* initialize mutex to 1 - binary semaphore */
                                 /* second param = 0 - semaphore is local */

    sem_init(&character_checked_out, NONSHARED, 1);
    sem_init(&characters_in_line, NONSHARED, 0);

    pthread_create(&producer_tid, NULL, Producer, NULL);
    pthread_create(&consumer_tid, NULL, Consumer, NULL);

    pthread_join(producer_tid, NULL);
    pthread_join(consumer_tid, NULL);

    sem_destroy(&mutex); /* destroy semaphore */
}
