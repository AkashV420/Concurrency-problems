#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
void quickSort(int arr[], int low, int high);
void normal_quickSort(int arr[], int low, int high);
void swap(int *a, int *b)
{
  int temp;
  temp = *a;
 *a = *b;
 *b = temp;
}
int * shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int*)shmat(shm_id, NULL, 0);
}
int partition(int arr[], int low, int high)
{
  int pivot,j,i;
    int varl=(high+low)/2;
   pivot = arr[varl];
   swap(&arr[varl],&arr[high]);
 
  i = low - 1;//index of smaller element
 
 for (j = low; j <= high- 1; j++)
    {
        // If current element is smaller than the pivot
        if (arr[j] < pivot)
        {
            i++;    // increment index of smaller element
            swap(&arr[i],&arr[j]);
        }
    }

swap(&arr[i+1],&arr[high]);
return (i+1);
}

void normal_quickSort(int arr[], int low, int high)
{
  if(low < high)
  {
     int pi;
     pi = partition(arr, low, high);
     normal_quickSort(arr, low, pi - 1);
     normal_quickSort(arr, pi + 1, high);
   }
}



            

void quickSort(int arr[], int low, int high)
{
    int n = (high-low+1);
    if(n<=5)
    {
    int key,i,j;
            int l=0,r=n-1;
        for(i=l;i<r;i++)
        {
            key=arr[i];
             j=i-1; 
             while (j >= 0 && arr[j] > key) 
            {  
                arr[j + 1] = arr[j];  
                j = j - 1;  
            }  
            arr[j + 1] = key;     
        }
        return ;
    }
    int pi = partition(arr, low, high);
    if(low < high)
    {
        int pid1, pid2;
         pid1 = fork();
         pid2;
        if(pid1==0)
        {
            //sort left half array
            quickSort(arr, low, pi - 1);
            _exit(1);
        }
        else{
            pid2 = fork();
            if(pid2==0)
            {
                //sort right half array
                quickSort(arr,pi + 1,high);
                _exit(1);
            }
            else{
                //wait for the right and the left half to get sorted
                int status;
                waitpid(pid1, &status, 0);
                waitpid(pid2, &status, 0);
            }
        }
    } 
    return;
}

struct arg
{
    int l;
    int r;
    int* arr;    
};

void *threaded_quicksort(void* a){
    //note that we are passing a struct to the threads for simplicity.
    struct arg *args = (struct arg*) a;
    int l,r;
     l = args->l;
     r = args->r;
    int *arr = args->arr;
    if(l > r) return NULL;    
    
    
    if(r-l+1 <= 5)
    {
       // int a[5];
        //int mi=INT_MAX;
        //int mid=-1;
        int key,i,j;
        for(i=l;i<r;i++)
        {
            key=arr[i];
             j=i-1; 
             while (j >= 0 && arr[j] > key) 
            {  
                arr[j + 1] = arr[j];  
                j = j - 1;  
            }  
            arr[j + 1] = key;     
        }
        return NULL;
    }

    //sort left half array
    struct arg a1;
    a1.l = l;
    a1.r = (l + r)/2;
    a1.arr = arr;
    pthread_t tid1;
    pthread_create(&tid1, NULL, threaded_quicksort, &a1);
    
    //sort right half array
    struct arg a2;
    a2.l = (l+r)/2+1;
    a2.r = r;
    a2.arr = arr;
    pthread_t tid2;
    pthread_create(&tid2, NULL, threaded_quicksort, &a2);
    
    //wait for the two halves to get sorted
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    partition(arr,l,r);
}

void runSorts(long long int n)
{

    struct timespec ts;
    
    //getting shared memory
    int *arr = shareMem(sizeof(int)*(n+1));
    int i=0;
    while(i<n)
    { 
        scanf("%d", arr+i);
        i = i + 1;
    }

    printf("Running concurrent_quicksort for n = %lld\n", n);
     clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec/(1e9)+ts.tv_sec;

  	if(n<=5)
    {
            int key,i,j;
            int l=0,r=n-1;
        for(i=l;i<r;i++)
        {
            key=arr[i];
             j=i-1; 
             while (j >= 0 && arr[j] > key) 
            {  
                arr[j + 1] = arr[j];  
                j = j - 1;  
            }  
            arr[j + 1] = key;     
        }
for(int i=0;i<n;i++)
    {
        printf("%d ",arr[i]);
    }
    printf("\n");
    // shmdt(arr);
    return ;
    }       	 	
    partition(arr, 0, n-1);
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en-st;

    int brr[n+1];
    for(int i=0;i<n;i++) brr[i] = arr[i];


    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = n-1;
    a.arr = brr;
    printf("Running threaded_concurrent_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    //multithreaded quicksort
    pthread_create(&tid, NULL, threaded_quicksort, &a);
    pthread_join(tid, NULL);    
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en-st;

    printf("Running normal_quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    // normal quicksort
    normal_quickSort(brr, 0, n-1);
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;

    printf("normal_quicksort ran:\n\t[ %Lf ] times faster than concurrent_quicksort\n\t[ %Lf ] times faster than threaded_concurrent_quicksort\n\n\n", t1/t3, t2/t3);

    for(int i=0;i<n;i++)
    {
        printf("%d ",brr[i]);
    }
    printf("\n");
    shmdt(arr);
    
    return;
}

int main()
{

    long long int n;
    scanf("%lld", &n);
    runSorts(n);

    
    return 0;
}