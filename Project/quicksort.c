#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

static double get_wall_seconds() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double seconds = tv.tv_sec + (double)tv.tv_usec / 1000000;
  return seconds;
}

typedef struct qsort {
  int min;
  int max;
  int *array;
} qsort_parameters;


/* ----------- Swap function: swaps two elements ---------- */
void swap(int array[], int i, int j) {
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

/* ------------------ Pivoting function ------------------
Splits array into two subarrays:
                                * Left array with elements <= pivotValue
                                * Right array with elements > pivotValue
*/
int pivot(int *arr, int min, int max, int pivot_idx) {
    int pivotValue = arr[pivot_idx];
    swap(arr, pivot_idx, max);

    int s = min; // Store smallest index

	for (int i = min; i < max; i++) {
		if (arr[i] <= pivotValue) {
            swap(arr, i, s);
            s++; // Increment smaller index
		}
	}
    swap(arr, s, max);
	return s;
}
/* ---------- Regular (non-threaded) QuickSort ---------- */
void quickSort(int *arr, int min, int max) {
	if (min < max) {
        int pivot_index = min + (max-min)/2;
        pivot_index = pivot(arr, min, max, pivot_index);

    	quickSort(arr, min, pivot_index - 1);
    	quickSort(arr, pivot_index + 1, max);
	}
}

/* --------------- Thread Helpers ---------------- */
void thread_quickSort(int *arr, int min, int max);

void* helper_quickSort(void * initial) {
    qsort_parameters* params = initial;
    thread_quickSort(params->array, params->min, params->max);
    return 0;
}

/* --------------- Threaded QuickSort --------------- */
void thread_quickSort(int *arr, int min, int max){
    if (min < max) {

        int pivot_index = min + (max - min)/2; // Pivot by 3-median
        //int pivot_index = max;               // Pivot by last element

        pivot_index = pivot(arr, min, max, pivot_index);
        pthread_t thread; // initalize

        /* Only spawn new thread when necessary, limit: 10 000*/
        if ((max-min) > 10000) {

            qsort_parameters left_thread_params;
            left_thread_params.array = arr;
            left_thread_params.min = min;
            left_thread_params.max = pivot_index-1;

            // Only create thread for left part
            pthread_create(&thread, NULL, helper_quickSort, &left_thread_params);

            //Recursively compute the right part in the same thread
            thread_quickSort(arr, pivot_index+1, max);

            pthread_join(thread, NULL); // Wait for l-thread

        }
        else { // if no thread is necessary, regular quicksort is faster
            quickSort(arr, min, pivot_index-1);
            quickSort(arr, pivot_index+1, max);
         }
    }
}

/********************** Main **********************/
int main(int argc, char const *argv[]) {

    srand (time(NULL)); // Random seed


    if (argc!=2)
    {
        printf("Please specify the length of the array to sort (./quicksort N)\n");
        return -1;
    }
    const int N = atoi(argv[1]);

    printf("Length of array is:  N = %d\n", N);
    int *array = malloc(N*sizeof(int));
    for(int i = 0; i < N; i++) {
        array[i] = rand() % 999;
    }

    clock_t start, end, start1, end1;
    double cpu_time_used, cpu_time_used1;

    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);

    double time1 = get_wall_seconds();
    start = clock();
    thread_quickSort(array, 0, N-1);  // **** Threaded Quicksort
    end = clock();

    printf("\nMulti-threaded quicksorting took %7.3f wall seconds.\n", get_wall_seconds()-time1);
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    double CPU_usage = cpu_time_used / number_of_processors / (get_wall_seconds()-time1);
    printf("CPU time: %7.3f seconds \n", cpu_time_used);
    printf("CPU usage: %7.1f percent \n \n", CPU_usage*100);


    double time2 = get_wall_seconds();
    start1 = clock();
    quickSort(array, 0, N-1);   // ******** Quicksort
    end1 = clock();

    printf("Single-threaded quicksorting took %7.3f wall seconds.\n", get_wall_seconds()-time2);
    cpu_time_used1 = ((double) (end1 - start1)) / CLOCKS_PER_SEC;
    double CPU_usage1 = cpu_time_used1 / number_of_processors / (get_wall_seconds()-time2);
    printf("CPU time: %7.3f seconds.\n", cpu_time_used1);
    printf("CPU usage: %7.1f percent \n", CPU_usage1*100);


    /* Check if actually sorted */
    for(int i = 1; i < N; i++) {
      if(array[i] < array[i-1]) {
        printf("Error! List not sorted!, at array[%d] num: %d \n", i, array[i]);
        return -1;
      }
    }
    printf("\nList is sorted!\n");
    return 0;
    free(array); // free array
}
