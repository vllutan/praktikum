/*Напишите и отладьте программу, которая считывает 
последовательность целых чисел (длина не более 1024) 
и печатает их по возрастанию.*/

#include <stdio.h>

void swap(int *a, int *b){
	int t=*a;
	*a=*b;
	*b=t;
}

int main() {
	int arr[1024], size, min_ind;
	
	printf("Enter the number of elements\n");
	scanf("%d", &size);
	printf("Enter numbers separated by space\n");
	
	for (int i = 0; i < size; ++i) scanf("%d", &arr[i]);
	
	for (int i = 0; i < size; ++i){
		min_ind = i;
		for (int j = i+1; j < size; ++j) 
			if (arr[j] < arr[min_ind]) min_ind = j;
			
		swap(&arr[i], &arr[min_ind]);
	}
	
	for (int i = 0; i < size; ++i) printf("%d ", arr[i]);
	
	return 0;
}
