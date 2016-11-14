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



void bubble( array_interval_t* interval ) {
   int temp;
   int i,j;
	 bool swapped = false;
   
   // loop through all numbers 
   for(i = interval->a; i < interval->c; i++) { 
      swapped = false;
		
      // loop through numbers falling ahead 
      for(j = interval->a; j < interval->c-i; j++) {
        // printf("     Items compared: [ %d, %d ] ", interval->array.array[j],interval->array.array[j+1]);

         // check if next number is lesser than current no
         //   swap the numbers. 
         //  (Bubble up the highest number)
			
         if(interval->array.array[j] > interval->array.array[j+1]) {
            temp = interval->array.array[j];
            interval->array.array[j] = interval->array.array[j+1];
            interval->array.array[j+1] = temp;

            swapped = true;
           // printf(" => swapped [%d, %d]\n",interval->array.array[j],interval->array.array[j+1]);
         }else {
           // printf(" => not swapped\n");
         }
			
      }

      // if no number was swapped that means 
      //   array is sorted now, break the loop. 
      if(!swapped) {
         break;
      }
      
    //  printf("Iteration %d#: ",(i+1)); 
   }
}

__task void quick_sort_task( void* void_ptr){
  // type casting the pointer back to a qsort_task_parameter
	qsort_task_parameters_t* task_param = (qsort_task_parameters_t*)void_ptr;
	
	array_interval_t p_array = task_param->interval;
	
 	int pivotindex = partition(&p_array);
 	int priority = task_param->priority;
	
	printf("The pivot is %d \n",pivotindex);
	printf("The array is ");
	
	
	
	//setting left and right
	array_interval_t right;	
	array_interval_t left;
	//setting left
	left.array.array = p_array.array.array;
	left.a = 0;
	left.c = pivotindex -1;
	//setting right
	right.array.array = p_array.array.array+pivotindex+1;
	right.a = pivotindex +1;
	right.c = p_array.c;
	
	//run bubble if list is under a certain size
	if ((left.c - left.a)<10){
		bubble(&left);
	}
	//if list is larger than 10 run create the new task
	else{
		qsort_task_parameters_t left_task;
		left_task.interval = left;
		left_task.priority = priority + 1;
		os_tsk_create_ex( quick_sort_task, left_task.priority, &left_task );
	}
	
	//run bubble if list is under a certain size
	if ((right.c - right.a)<10){
		bubble(&right);
	}
	//if list is larger than 10 run create the new task
	else{
		qsort_task_parameters_t right_task;
		right_task.interval = right;
		right_task.priority = priority + 1;
		os_tsk_create_ex( quick_sort_task, right_task.priority, &right_task );
	}
	
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
