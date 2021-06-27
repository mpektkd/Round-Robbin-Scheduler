//implementation of queue by https://www.geeksforgeeks.org/queue-linked-list-implementation/


// A C program to demonstrate linked list based implementation of queue 
#include <stdio.h> 
#include <stdlib.h> 
#include "queue2.h"
#include <sys/wait.h>
#include <sys/types.h>
#include "proc-common.h"
#include <unistd.h>

//int size;
//int getsize(struct Queue *q){
  //  return q->size;
//}
// A utility function to create a new linked list node. 
struct QNode* newNode(pid_t k, int d, char *n) 
{ 
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode)); 
    temp -> id = d;
    temp -> name = n;
    temp->key = k; 
    temp->next = NULL; 
   // temp->previous = NULL;
    return temp; 
} 
  
// A utility function to create an empty queue 
struct Queue* createQueue() 
{ 

    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->size = 0;
    q->front = q->rear = NULL; 
    return q; 
} 
  
// The function to add a key k to q 
void enQueue(struct Queue* q, pid_t k, int d, char *name) 
{ 
    
    // Create a new LL node 
    struct QNode* temp = newNode(k,d,name); 
  
    // If queue is empty, then new node is front and rear both 
    if (q->rear == NULL) { 
        q->front = q->rear = temp; 
        return; 
    } 
  
    // Add the new node at the end of queue and change rear 
    q->rear->next = temp; 
   // temp->previous = q->rear;
    q->rear = temp; 
    //++(q->size);

} 
  
// Function to remove a key from given queue q 
void deQueue(struct Queue* q) 
{ 
    // If queue is empty, return NULL. 
    if (q->front == NULL) 
        return; 
  
    // Store previous front and move front one node ahead 
    struct QNode* temp = q->front; 
    
    q->front = q->front->next; 
    //q->front->previous = NULL;
    
    // If front becomes NULL, then change rear also as NULL 
    if (q->front == NULL) 
        q->rear = NULL; 
  
    free(temp); 
    //--(q->size);

    show_pstree(getpid());

    
} 

int find_id_2(struct Queue* queue1,struct Queue *queue2){
    //int counter = 0;
    struct QNode *q = queue1->front;
    int size = queue1->size+queue2->size;
    //int array[size];
    int array2[size];
    int array3[size];
    /*while(q != NULL){
        ++counter;
        q = q->next;
    }
    */
    //int array[counter];
    //int array2[counter];
    int i;
    for(i = 0; i < size;++i){
      //  array[i] = i;
        array2[i] = 0;
    }

    //q = queue->front;
    i = 0;
    while(q != NULL){
        array3[i] = q->id;
        q=q->next;
        ++i;
    }
    q = queue2->front;
    while(q != NULL){
        array3[i] = q->id;
        q=q->next;
        ++i;
    }
    int j=0;
    for(i=0;i<size;++i){
        while(j<size){
            if(i == array3[j])
                array2[i] = 1;
            ++j;
        }
        j = 0;
    }
        //q = queue->front;
    
    int smallest;
    for(i=0;i<size;++i){
        if(array2[i]==0 && i != 0){
            smallest = i;
            break;
        }
    }
    return smallest;
}


int find_id_1(struct Queue* queue){
    struct QNode *q = queue->front;
    
    int array2[queue->size];
    int i;
    for(i = 0; i < queue->size;++i){
        array2[i] = 0;
    }

    for(i = 0;i<queue->size;++i){
        while(q != NULL){
            if(i == q->id)
                array2[i] = 1;
            q=q->next;
        }
        q = queue->front;
    }

    int smallest;

    for(i=0;i<queue->size;++i){
        if((array2[i] == 0) && (i != 0)){
            smallest = i;
            break;
        }
    }
    return smallest;
}


void delete(struct Queue *q,pid_t pid){
    
    struct QNode *temp = q->front;
    struct QNode *temp2 = NULL;

    if(q->front->key == pid){
        q->front = q->front->next;
        
    }
    else{
        while(temp != NULL){
            if(temp->key == pid)
                break;
            temp2 = temp;
            temp = temp->next;
        }
        temp2->next = temp->next;
    }
    free(temp);
    /*
    while(temp != NULL){
            break;
        temp2 = temp;
        temp = temp->next;
    }
    if(temp == q->front){
        if(q->front == NULL){
            return;
        }
        else{
            q->front = q->front->next; 
            //if(q->front == NULL)
              //  q->rear = NULL;
            free(temp);
        }
        
    }
    else{
        temp2->next = temp->next;
        free(temp);
    }

*/
}