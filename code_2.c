#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "esp_timer.h"

#define NUM_TASKS 200
#define NUM_CORES 4
#define DATA_FILE "data.txt"
#define RESULT_FILE "results.txt"

// Define a mutex to protect the file access
SemaphoreHandle_t fileMutex;

void thread_function(void *pvParameters) {
    int task_id = (int)pvParameters;
    int sum = 0;

    // Record the start time
    int64_t start_time = esp_timer_get_time();

    // Open the data file for reading
    FILE *data_file = fopen(DATA_FILE, "r");
    if (data_file == NULL) {
        perror("Failed to open data file");
        vTaskDelete(NULL);
    }

    // Read integers from the data file and calculate the sum
    int num;
    while (fscanf(data_file, "%d", &num) == 1) {
        sum += num;
    }

    fclose(data_file);

    // Record the end time
    int64_t end_time = esp_timer_get_time();

    // Get the core number where the task is running
    int core_num = xPortGetCoreID();

    // Take the mutex to protect file access
    if (xSemaphoreTake(fileMutex, portMAX_DELAY) == pdTRUE) {
        // Open the results file for writing
        FILE *results_file = fopen(RESULT_FILE, "a");
        if (results_file == NULL) {
            perror("Failed to open results file");
            vTaskDelete(NULL);
        }

        // Write the result, task ID, core number, and execution time to the results file
        fprintf(results_file, "Task %d (Core %d): Sum = %d, Start Time = %lld, End Time = %lld\n", task_id, core_num, sum, start_time, end_time);
        fclose(results_file);

        // Release the mutex
        xSemaphoreGive(fileMutex);
    }

    vTaskDelete(NULL);
}

int main() {
    // Initialize the mutex for file access
    fileMutex = xSemaphoreCreateMutex();

    if (fileMutex == NULL) {
        perror("Failed to create mutex");
        return 1;
    }

    // Create FreeRTOS tasks on multiple cores
    TaskHandle_t tasks[NUM_TASKS];
    for (int i = 0; i < NUM_TASKS; i++) {
        int task_id = i;
        // Create tasks on different cores (cyclic assignment)
        int core_num = task_id % NUM_CORES;
        xTaskCreatePinnedToCore(thread_function, "Task", configMINIMAL_STACK_SIZE, (void *)task_id, tskIDLE_PRIORITY + 1, &tasks[i], core_num);
    }

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    return 0;  // Should never reach here
}
