#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

struct CAB
{
	pthread_mutex_t lock;
    int cab_status; // 0: wait 1: premier 2: poolone 3: poolfull 
    int cab_type; 
	int R1;
	int R2;
	int cabId;
} *Cabs[500];

struct RIDER
{
	pthread_t tid;
    int id;
    int cabnumber;
    int cab_type; // 0: pool 1: premier
	int arrival_time;
	int max_Wait_time;
	int ride_time;
	int paymentServer;
} *riders[500];

struct PAYMENT
{
    pthread_mutex_t lock;
    pthread_t tid;
	int id;
	int rider;
	int pymt_Status;
	
} *Pservers[50];

int n, total_no_of_riders = 0;
int m;
int k;

pthread_mutex_t total_rider_lock;

struct PAYMENT ;

void *Kiraya_Lo(void *var)
{
	struct PAYMENT *payment = (struct PAYMENT *) var;
	while(1) 
    {
		while(payment->pymt_Status == 0) 
        {
			if(total_no_of_riders == 0) 
            {
				return NULL;
			}
		}
		printf("Rider %d has started payment in Server %d\n", payment->rider, payment->id);
		fflush(stdout);
		
        sleep(2); //WAIT
		
        printf("Payment finished with Rider %d\n", payment->rider);
		fflush(stdout);
        
        payment->pymt_Status = 0;
		
        payment->rider = -1;
		
        pthread_mutex_lock(&(total_rider_lock));
		
        total_no_of_riders = total_no_of_riders - 1;
		
        pthread_mutex_unlock(&(total_rider_lock));
	}
	return NULL;
}


void *Cab_Booking(void *var)
{
	struct RIDER *rider = (struct RIDER *) var;
	
    
    sleep(rider->arrival_time);
	
    if(rider->cab_type == 0)
    	printf("Rider %d is waiting for pool cab\n", rider->id);

    else {
    	printf("Rider %d is waiting for premier cab\n", rider->id);
    }
	
	
    
	for(int t = rider->arrival_time ; t <= rider->arrival_time + rider->max_Wait_time; t++) 
    {	
        int i = 0;
		if(rider->cab_type == 0) 
    	{		
    		while(i < n) 
            {
                int available = 0;
				
                pthread_mutex_lock(&(Cabs[i]->lock)); //Lock lgao!
				
                available = Cabs[i]->cab_status;
				
                pthread_mutex_unlock(&(Cabs[i]->lock)); // Unlocked !
				
                if(available == 2) 
                {
					pthread_mutex_lock(&(Cabs[i]->lock));
					
                    Cabs[i]->cab_status = 3;
					
                    pthread_mutex_unlock(&(Cabs[i]->lock));
					rider->cabnumber = i;
				}
				if(rider->cabnumber != -1) 
                {
					break;
				}
                i = i + 1;			
			}
			if(rider->cabnumber != -1) 
            {
				break;
			}
        }    
        i=0;
			while(i < n) 
            {
				int available = 0;
                pthread_mutex_lock(&(Cabs[i]->lock));
			    
                available = Cabs[i]->cab_status;
				
                pthread_mutex_unlock(&(Cabs[i]->lock));
				
                if(available == 0) 
                {
					pthread_mutex_lock(&(Cabs[i]->lock));
					Cabs[i]->cab_status = 2;
					pthread_mutex_unlock(&(Cabs[i]->lock));
					rider->cabnumber = i;
				}	
				if(rider->cabnumber != -1) 
                {
					break;
				}
                i = i + 1;
			}
			if(rider->cabnumber != -1) 
            {
				break;
			}
			sleep(1);
	}
	
    if(rider->cabnumber < 0) 
    {
		printf("Rider %d timed out\n", rider->id);
		fflush(stdout);
		
        pthread_mutex_lock(&(total_rider_lock));
		
        total_no_of_riders = total_no_of_riders - 1;
		
        pthread_mutex_unlock(&(total_rider_lock));
		return NULL;
	}
	printf("Rider %d is assigned cab number %d\n", rider->id, rider->cabnumber);
	
    sleep(rider->ride_time);
	
    printf("Rider %d finished \n", rider->id);//PAHUCHGYA
	
    pthread_mutex_lock(&(Cabs[rider->cabnumber]->lock));//Lock
	
    Cabs[rider->cabnumber]->cab_status = Cabs[rider->cabnumber]->cab_status - 1;///decrement
	
    if(Cabs[rider->cabnumber]->cab_status == 1) 
    {
		Cabs[rider->cabnumber]->cab_status = 0;
	} 
	
    pthread_mutex_unlock(&(Cabs[rider->cabnumber]->lock));
	
    while(1) 
    {
        int i=0;
		while(i < k) 
        {
            int status = 0;
			pthread_mutex_lock(&(Pservers[i]->lock));
            
            status = Pservers[i]->pymt_Status;
			
            pthread_mutex_unlock(&(Pservers[i]->lock));
			
            if(status == 0) 
            {
				pthread_mutex_lock(&(Pservers[i]->lock));
				Pservers[i]->rider = rider->id;
            	Pservers[i]->pymt_Status = 1;
		     	pthread_mutex_unlock(&(Pservers[i]->lock));
				rider->paymentServer = i;
			}
			if(rider->paymentServer != -1) 
            {
				break;
			}

        i = i + 1;
		}
		if(rider->paymentServer != -1) 
        {
			break;
		}
	}
	
	fflush(stdout);
	return NULL;
}

struct CAB *cab_init(int i) 
{
	struct CAB *temp = (struct CAB *) malloc(sizeof(struct CAB));
	temp->cab_type = rand()%2 + 1;
	temp->cab_status = 0;
	temp->cabId = i + 1;
	temp->R1 = -1;
	temp->R2 = -1;
	pthread_mutex_init(&(temp->lock), NULL);
	return temp;
}

struct RIDER *rider_init(int i)
{
	struct RIDER *temp = (struct RIDER *) malloc(sizeof(struct RIDER));
	temp->id = i;
	temp->cab_type = rand() % 2;
    temp->cabnumber = -1;
	temp->arrival_time = rand() % 50 + 1;
    temp->ride_time = rand() % 8 + 1;
	temp->max_Wait_time = rand() % 8 + 1;
	temp->paymentServer = -1;
	return temp;
}

struct PAYMENT *payment_init(int i)
{
	struct PAYMENT *temp = (struct PAYMENT *) malloc(sizeof(struct PAYMENT));
	temp->pymt_Status = 0;
	temp->rider = -1;
	temp->id = i;
	pthread_mutex_init(&(temp->lock), NULL);
	return temp;
}

int main()
{
	scanf("%d %d %d", &n, &m, &k);

	total_no_of_riders = m;
	pthread_mutex_init(&(total_rider_lock), NULL);
    
    
    int i=0;
	
	srand(time(NULL));

	while( i < n) 
    {
		Cabs[i] = cab_init(i);
		i++;
	}
  
    int j=0;
	while(j < m) 
    {
		riders[j] = rider_init(j);
		pthread_create(&(riders[j]->tid), NULL, Cab_Booking, (void *) riders[j]);
		j++;	    	
    }
    
    int p=0;
	while(p < k) 
    {
		Pservers[p] = payment_init(p);
		pthread_create(&(Pservers[p]->tid), NULL, Kiraya_Lo, (void *) Pservers[p]);
        p = p + 1;
    }
    
    int x = 0;
	while(x < m) 
    {
		pthread_join(riders[x]->tid, NULL);
        x = x + 1;
	}
	
    int y=0;
    while(y < k) 
    {
		pthread_join(Pservers[y]->tid, NULL);
        y = y + 1;
    }

	printf("RIDING COMPLETED !\n");
	return 0;
}