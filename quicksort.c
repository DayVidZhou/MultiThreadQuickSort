#include <LPC17xx.h>
#include <RTL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "quicksort.h"
#include "array_tools.h"

// You decide what the threshold will be
#define USE_INSERTION_SORT 30

// Local prototypes



//structs
typedef struct {
	array_t array;
	size_t a;
	size_t c;
} array_interval_t;

typedef struct{
	array_interval_t interval;
	unsigned char priority;
} qsort_task_parameters_t;


// swap function used in the partition function
__inline void swap(array_type* arr, int a, int b){
    array_type tmp = arr[a];
    arr[a] = arr[b];
    arr[b] = tmp;
}
// This function implements the partitioning and swapping as described in the quicksort algorithm
// It has one input argument which is "Interval"
// interval is defined as <array_interval_t* interval;> and passed from the tasks when this function is called
// When you define inteval in your code using the following
//array_interval_t* interval;
//.....
//cur_params = (qsort_task_parameters_t*) void_ptr;
//interval = &(cur_params->interval);
//.......
/// rest of your code
//The definition of interval is the same for both tasks
// When you want to call the partition functio you should use
// pivot_index = partition(interval)

int partition(array_interval_t* interval){
    int pivot_index, tmp_index, i;
    array_type pivot_value;
    
    tmp_index = interval->a;
    pivot_index = interval->a+ (interval->c - interval->a)/2; // You can change this line to choose your best method
    pivot_value = interval->array.array[pivot_index];
    
    swap(interval->array.array, pivot_index, interval->c);
    
    for (i = interval->a; i<interval->c; i++){
        if (interval->array.array[i] < pivot_value){
            swap(interval->array.array, i, tmp_index);
            ++tmp_index;
        }
    }
    
    swap(interval->array.array, tmp_index, interval->c);
    return tmp_index;
}


void insertion( array_interval_t* interval ) {
	size_t i, j, c = interval->c; // Save it to a varible to avoid repeated checking
	array_type temp;
	array_type* arr = interval->array.array;
	
	for (i = interval->a; i<=c; i++){
		temp = arr[i];
		j = i;
		while(j>0 && arr[j-1] > temp ){
			arr[j] = arr[j-1];
			j--;
		}
		arr[j] = temp;
	}
}

__task void quick_sort_task( void* void_ptr){
  // type casting the pointer back to a qsort_task_parameter
	qsort_task_parameters_t* task_param = (qsort_task_parameters_t*)void_ptr;
	
	array_interval_t p_array = task_param->interval;
	
 	int pivotindex = partition(&p_array);
 	int priority = task_param->priority;
	int i;
	//initialize left and right
	array_interval_t right;	
	array_interval_t left;
	
// 	printf("The pivot index is %d \n",pivotindex);
// 	printf("Array is ");
// 	for (i = p_array.a; i <= p_array.c; i++){
// 		printf("%d , ",p_array.array.array[i]);
// 	}
// 	printf("\n");
	
	//CHECKING BASE CASE
	if (p_array.a < p_array.c){
		
	//setting left
	left.array.array = p_array.array.array;
	left.a = 0;
	left.c = pivotindex -1;
	left.array.length = left.c - left.a + 1;
	//setting right
	right.array.array = p_array.array.array;
	right.a = pivotindex +1;
	right.c = p_array.c;
	right.array.length = right.c - right.a + 1;
	
	//run insertion if list is under a certain size
	if (pivotindex != 0 && (left.c - left.a)<10){
	//	printf (" LEFT insertion RAN ");
	//	printf (" LEFT SIDE a is %d and c is %d ", left.a,left.c);
		insertion(&left);
	}
	//if list is larger than 10 run create the new task
	else if (pivotindex != 0){
		qsort_task_parameters_t left_task;
		left_task.interval = left;
		left_task.priority = priority + 1;
		if (!os_tsk_create_ex( quick_sort_task, left_task.priority, &left_task )){
			insertion(&left_task.interval);
		}	
	}
	
	//run insertion if list is under a certain size
	if (pivotindex != p_array.c &&(right.c - right.a)<10){
//		printf(" RIGHT insertion RAN ");
//		printf (" RIGHT SIDE a is %d and c is %d ", right.a,right.c);
		insertion(&right);
	}
	//if list is larger than 10 run create the new task
	else if (pivotindex != p_array.c){
		qsort_task_parameters_t right_task;
		right_task.interval = right;
		right_task.priority = priority + 1;
		if (!os_tsk_create_ex( quick_sort_task, right_task.priority, &right_task )){
			insertion(&right_task.interval);
		}	
	}
}
	free(task_param);
	os_tsk_delete_self();
}

void quicksort( array_t array ) {
	array_interval_t interval;
	qsort_task_parameters_t task_param;
	
	// Based on MTE 241 course notes--you can change this if you want
	//  - in the course notes, this sorts from a to c - 1
	interval.array =  array;
	interval.a     =  0;
	interval.c     =  array.length-1;
	
	task_param.interval = interval;

	// If you are using priorities, you can change this
	task_param.priority = 10;
	
	//start the quick_sort threading
	
	os_tsk_create_ex( quick_sort_task, task_param.priority, &task_param ); 
}
