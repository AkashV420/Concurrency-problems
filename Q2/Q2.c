#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

pthread_mutex_t student_lock;

int bhooke_bacche;
int n;
int m;
int k;

struct ROBOT
{
	int num;
	int RV;
	int PS;
	int total;
	pthread_t tnum;
} *robots[1000];

struct tabl
{
	int num;
	int status;
	int maxslots;
	int slot_size;
	int ready;
	int slots_left;
	pthread_t tnum;
} *tables[1000];

struct studen
{
	int num;
	int waiting;
	int table;
	int slot;
	pthread_t tnum;
} *students[1000];

pthread_mutex_t tablelocks[1000];


//----------------------/* ROBOTcode starts */-------------------//
void biryani_ready(struct ROBOT*chef_robot)
{
	int total_vessels = 0;
	int assigned = 0;
	total_vessels = chef_robot->RV;
	assigned = 0;
	while(total_vessels >= 0) 
	{
		int i=0;
		while(i < m) 
		{
			if(bhooke_bacche == 0) break;
			pthread_mutex_lock(&(tablelocks[i])); //Apply lock
			
			if(tables[i]->status == 0) 
			{
				//------------------/* insnume mutex lock on robot-> table */------------------//
				tables[i]->status = 1;
				tables[i]->maxslots = chef_robot->PS;
				total_vessels = total_vessels - 1;
				printf("Robotchef %d's vessel number %d has been loaded in %d table\n", chef_robot->num, chef_robot->RV - total_vessels, tables[i]->num);
				
				//-----------------/*get out of mutex lock on robot-> table*/----------------//
			}
			
			pthread_mutex_unlock(&(tablelocks[i]));  //Unlock it
			
			if(total_vessels == 0) 
			{
				return;
			}
			i = i + 1;
		}
		if(bhooke_bacche == 0) 
		{
			return;
		}
	}
	return;
}

void *robot_chef(void *rob)
{   
	int w = 0;
	struct ROBOT*chef_robot = (struct ROBOT*) rob;
	while(1)
	{
		if(bhooke_bacche == 0) 
		{
			return NULL;
		}
		w = rand()%4 + 2;
		
		printf("robotchef %d is preparing biryani\n", chef_robot->num);
		
		
		sleep(w);
		chef_robot->RV = rand()%10 + 1;
		chef_robot->PS = rand()%26 + 25;
		
		printf("robotchef %d has prepared %d vessels with each vessel serving %d students\n", chef_robot->num, chef_robot->RV, chef_robot->PS);
		
		
		biryani_ready(chef_robot);
	}
	return NULL;
}
//------------------/* robotcode ends */-------------//

void ready_to_serve_table(struct tabl *table, int slot_size)
{
	while(table->slot_size > 0) 
	{
		pthread_mutex_lock(&(tablelocks[table->num - 1]));
		if(table->maxslots == 0) 
		{
			table->status = 0;
		}
		
		if (table->maxslots <= slot_size) {
			table->slot_size = table->maxslots;	
		}
		else 
		{
			table->slot_size = slot_size;
		}
		if(table->slots_left > table->slot_size) 
		{
			table->slots_left = table->slot_size;
		}
		
		pthread_mutex_unlock(&(tablelocks[table->num - 1]));
		if(bhooke_bacche == 0) 
		{
			return;
		}
	}
	return;
}

void *serving_table(void *tab)
{
	struct tabl *table = (struct tabl *) tab;
	while(1)
	{
		if(bhooke_bacche == 0) return NULL;
		
		while(1)
		{
			if(table->status == 1) {
				break;
			}
			if(bhooke_bacche == 0) {
				return NULL;
			}					//----------------/* wait for the container to get filled */
		}

		table->slot_size = rand()%10 + 1;
		table->slots_left = table->slot_size;
		
		printf("table %d has generated %d slots\n", table->num, table->slot_size);
		if(bhooke_bacche == 0) return NULL;
		
		ready_to_serve_table(table, table->slot_size);
	}
	return NULL;
}



void wait_for_slot(struct studen *student)
{
	student->waiting = 1;
	while(student->table == 0) 
	{
		int i=0;
		while(i < m) 
		{
			//------------/* mutex lock */----------------//
			pthread_mutex_lock(&(tablelocks[i]));
			if(tables[i]->slots_left > 0) 
			{
				student->table = tables[i]->num;
				tables[i]->slots_left = tables[i]->slots_left - 1;
				tables[i]->maxslots = tables[i]->maxslots - 1;
			}
			pthread_mutex_unlock(&(tablelocks[i]));
			if(student->table != 0) 
			{
				break;
			}
			//--------------/* mutex unlocks */-----------//
			i = i + 1;
		}
	}
	printf("student %d is assigned a slot at table %d and is waiting for biryani to be served\n", student->num, student->table);
	
}

void student_in_slot(struct studen *student)
{
	printf("student %d has been served biryani\n", student->num);
	

	sleep(3); 

	printf("student %d has finished eating biryani\n", student->num);
	
    //--------------------------------------------------------------/* mutex lock */
	
	pthread_mutex_lock(&(tablelocks[student->table-1]));
	tables[student->table-1]->slots_left = tables[student->table-1]->slots_left + 1;
	pthread_mutex_unlock(&(tablelocks[student->table-1]));
	
	//------------------------------------------------------------/* mutex unlock */
	pthread_mutex_lock(&student_lock);
	bhooke_bacche = bhooke_bacche - 1;
	pthread_mutex_unlock(&student_lock);
}

void *incoming_student(void *stud)
{
	struct studen *student = (struct studen *) stud;
	wait_for_slot(student);
	student_in_slot(student);
	return NULL;
}

struct ROBOT*robot_init(int i)
{
	struct ROBOT*temp = (struct ROBOT*)malloc(sizeof(struct ROBOT));
	temp->num = i + 1;
	temp->RV = 0;
	temp->PS = 0;
	return temp;
}
struct tabl *table_init(int j)
{
	struct tabl *temp = (struct tabl *)malloc(sizeof(struct tabl));
	temp->num = j + 1;
	temp->ready = 0;
	temp->status = 0;
	temp->maxslots = 0;
	temp->slot_size = 0;
	temp->slots_left = 0;
	return temp;
}
struct studen *student_init(int p)
{
	struct studen *temp = (struct studen *)malloc(sizeof(struct studen));
	temp->num = p + 1;
	temp->table = 0;
	temp->waiting = 0;
	temp->slot = 0;
	return temp;
}
int main()
{
	srand(time(NULL));
	
	printf("No. of Chef's : ");
    scanf("%d",&n);
    printf("No. of Tables : ");
    scanf("%d",&m);  
    printf("No. of Students : ");
    scanf("%d",&k);
	
	bhooke_bacche = k;

	//--------------/* starting robotthreads */---------------//
    int  i = 0;
	while(i < n) 
	{
		robots[i] = robot_init(i);
		pthread_create(&(robots[i]->tnum), NULL, robot_chef, (void *) robots[i]);	
		i = i + 1;
	}
	//------------/*starting serving table threads */-------//
	int j = 0;
	while(j < m) 
	{
		tables[j] = table_init(j);
		pthread_mutex_init(&(tablelocks[i]), NULL);
		pthread_create(&(tables[j]->tnum), NULL, serving_table, (void *) tables[j]);	
		j = j + 1;
	}
	sleep(10);
	pthread_mutex_init(&student_lock, NULL);
	//-----------/* starting student threads */----------//
	int p=0;
	while(p < k) 
	{	
		students[p] = student_init(p);
		pthread_create(&(students[p]->tnum), NULL, incoming_student, (void *) students[p]);	
		p = p + 1; 
	}
    int x = 0;
	while(x < n) 
	{
		pthread_join(robots[x]->tnum, NULL);
		x = x + 1;
	}
	int y=0;
	while(y < m) 
	{
		pthread_join(tables[y]->tnum, NULL);
		y = y + 1;
	}
	int z = 0;
	while(z < k) 
	{
		pthread_join(students[z]->tnum, NULL);
		z = z + 1;
	}
	int w = 0;
	while(w < m) 
	{
		pthread_mutex_destroy(&(tablelocks[w]));
		w = w + 1;
	}
	
	return 0;
}