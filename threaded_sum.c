//Author: Sofia Gratny
//Date: Oct 3 2023

//include
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
//define
#define max 1000000
#define buffersize 4096 //processing data in smaller chunks

//structure to hold thread specific data
typedef struct {
    int start_index;            //starting index for this thread
    int end_index;              //ending index for this thread
    int* data;                  //pointer to the data array
    long long int* partial_sum; //pointer to thread partial sum
    pthread_mutex_t* mutex;     //mutex for synchro 
} ThreadData;

//func prototypes 
int read_file(const char file_name[], int** values);
void* calculate_partial_sum(void* arg);
//main func
int main(int argc, char* argv[]) {
    if (argc != 3) {
    fprintf(stderr, "Usage: %s <filename> <num_threads>\n", argv[0]);
    return -1;
    }
const char* file_name = argv[1];
int num_threads = atoi(argv[2]);

int* data = NULL;
int num_values = read_file(file_name, &data);

    if (num_threads > num_values) {
        fprintf(stderr, "Too many threads requested\n");
        free(data);
        return -1;
    }
    if (num_values == -1) {
        fprintf(stderr, "Error reading file: %s\n", file_name);
        free(data);
        return -1;
    }
    long long int total_sum = 0;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    ThreadData* thread_data_array = malloc(num_threads * sizeof(ThreadData));
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));

    if (thread_data_array == NULL || threads == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        free(data);
        return -1;
    }
     int slice = num_values / num_threads;
    //thread creation
    for (int i = 0; i < num_threads; i++) {
        thread_data_array[i].data = data;
        thread_data_array[i].start_index = i * slice;
        thread_data_array[i].end_index = (i == num_threads - 1) ? num_values - 1 : (i + 1) * slice - 1;
        thread_data_array[i].partial_sum = &total_sum;
        thread_data_array[i].mutex = &mutex;

    if (pthread_create(&threads[i], NULL, calculate_partial_sum, &thread_data_array[i]) != 0) {
        fprintf(stderr, "Error creating thread %d\n", i);
        free(data);
        free(thread_data_array);
        free(threads);
        return -1;
     }
    }
    //wait for all threads to complete
for (int i = 0; i < num_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
            free(data);
            free(thread_data_array);
            free(threads);
            return -1;
     }
    }
    gettimeofday(&end, NULL);
    double execution_time = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    //print total sum & execution time
    printf("Total sum: %lld\n", total_sum);
    printf("Total execution time: %.2f ms\n", execution_time);

    //clean & free memory
    pthread_mutex_destroy(&mutex);
    free(threads);
    free(thread_data_array);
    free(data);
    return 0;
}
int read_file(const char file_name[], int** values) {
    FILE* file;
    int rnum = 0;
    int buffer[buffersize];
    file = fopen(file_name, "r");
if (*values == NULL) {
        fclose(file);
        fprintf(stderr, "Memory allocation error\n");
        return -1;
    }
if (file == NULL) {
        fprintf(stderr, "File not found: %s\n", file_name);
        return -1;
    }
    int value;
    *values = malloc(max * sizeof(int));

    while (1) {
        int bytes_read = fread(buffer, sizeof(int), buffersize, file);
        if (bytes_read <= 0) {
            break; //end of file or error
        }
        for (int i = 0; i < bytes_read / sizeof(int); i++) {
            (*values)[rnum++] = buffer[i];
    }
    }
    fclose(file);
    return rnum;
}
//calc the sum of int wit a slice of array
void* calculate_partial_sum(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long long int thread_sum = 0;
    for (int i = data->start_index; i <= data->end_index; i++) {
    thread_sum += data->data[i];
    }
    pthread_mutex_lock(data->mutex);
    *(data->partial_sum) += thread_sum;
    pthread_mutex_unlock(data->mutex);
    pthread_exit(NULL);
}


