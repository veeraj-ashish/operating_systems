#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#define NUM_THREADS 40
#define DATA_FILE "data.txt"
#define RESULT_FILE "results.txt"

void *thread_function(void *arg) {
    int thread_id = *(int *)arg;
    int sum = 0;

    // Open the data file for reading
    FILE *data_file = fopen(DATA_FILE, "r");
    if (data_file == NULL) {
        perror("Failed to open data file");
        pthread_exit(NULL);
    }

    // Read integers from the data file and calculate the sum
    int num;
    while (fscanf(data_file, "%d", &num) == 1) {
        sum += num;
    }

    fclose(data_file);

    // Get the core number where the thread is running
    int core_num = sched_getcpu();

    // Open the results file for writing
    FILE *results_file = fopen(RESULT_FILE, "a");
    if (results_file == NULL) {
        perror("Failed to open results file");
        pthread_exit(NULL);
    }

    // Write the result, thread ID, and core number to the results file
    fprintf(results_file, "Thread %d (Core %d): Sum = %d\n", thread_id, core_num, sum);
    fclose(results_file);

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    clock_t start, end;
    double cpu_time_used;

    start = clock();

    // Open the results file and clear its contents
    FILE *results_file = fopen(RESULT_FILE, "w");
    if (results_file == NULL) {
        perror("Failed to open results file");
        return 1;
    }
    fclose(results_file);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        int result = pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
        if (result) {
            perror("Thread creation failed");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("All threads have finished. Total execution time: %f seconds.\n", cpu_time_used);

    return 0;
}
