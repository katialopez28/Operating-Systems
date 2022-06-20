// Katia Lopez

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX 5000000

//Test with 1, 2 and 4 threads
#define NUMTHREADS 1

int total = 0;
pthread_mutex_t lock;

int n1,n2;
char *s1,*s2;

FILE *fp;
int readf(char* filename)
{
    if((fp=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can't open %s!\n", filename);
        return 0;
    }

    s1=(char *)malloc(sizeof(char)*MAX);

    if (s1==NULL)
    {
        printf ("ERROR: Out of memory!\n") ;
        return -1;
    }

    s2=(char *)malloc(sizeof(char)*MAX);

    if (s1==NULL)
    {
        printf ("ERROR: Out of memory\n") ;
        return -1;
    }

    /*read s1 s2 from the file*/

    s1=fgets(s1, MAX, fp);
    s2=fgets(s2, MAX, fp);
    n1=strlen(s1); /*length of s1*/
    n2=strlen(s2)-1; /*length of s2*/

    if( s1==NULL || s2==NULL || n1 < n2 ) /*when error exit*/
    {
        return -1;
    }
}


void* num_substring(void *var)
{
    int i,j,k;
    int count;

    //keep a local variable of the total for each thread
    int localmatches = 0;

    int threadid = *(int*)var;

    //create intervals to separate the work in between threads
    int start = (threadid) * n1/NUMTHREADS;
    int end = (threadid+1) * n1/NUMTHREADS;

    for (i=start; i<end; i++)
    {
        count =0;
        for(j = i ,k = 0; k < n2; j++,k++)
        { /*search for the next string of size of n2*/
            if (*(s1+j)!=*(s2+k))
            {
                break ;
            }
            else
            {
                count++;
            }
            if (count==n2)
            {
                /*find a substring in this step*/
                localmatches++;
            }
         }
    }

    //use mutex around global variable so a race condition is prevented
    pthread_mutex_lock(&lock);
    total += localmatches;
    pthread_mutex_unlock(&lock);

    return NULL;
}


int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
      printf("Error: You must pass in the datafile as a commandline parameter\n");
    }

    readf ( argv[1] ) ;

    struct timeval start, end;
    float mtime;
    int secs, usecs, i;

    gettimeofday(&start, NULL);

    //Add threads to main and each of the threads will call num_substring
    pthread_t tid[NUMTHREADS];

    int array[NUMTHREADS];
    for (i=0; i<NUMTHREADS; i++)
    {
        array[i] = i;
        pthread_create(&tid[i], NULL, num_substring, (void*)&array[i]);
    }

    //Joining threads
    for (i=0; i<NUMTHREADS; i++)
    {
        pthread_join(tid[i], NULL);
    }

    //measure time taken
    gettimeofday(&end, NULL);
    secs  = end.tv_sec  - start.tv_sec;
    usecs = end.tv_usec - start.tv_usec;
    mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;

    pthread_mutex_destroy(&lock);

    printf ("The number of substrings is : %d\n" , total);
    printf ("Elapsed time is : %f milliseconds\n", mtime );

    if(s1)
    {
      free(s1);
    }

    if(s2)
    {
      free(s2);
    }


    return 0 ;
}
