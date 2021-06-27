#ifndef QUEUE2_H_
#define QUEUE2_H_

//int flag;
int getsize();
// A linked list (LL) node to store a queue entry 
struct QNode { 
    int id;
    pid_t key; 
    char *name;
    struct QNode* next; 
   // struct QNode* previous;
}; 
  
// The queue, front stores the front node of LL and rear stores the 
// last node of LL 
struct Queue { 
    int size;
    struct QNode *front, *rear; 
}; 

struct QNode* newNode(pid_t k, int d, char *name);
struct Queue* createQueue();
void enQueue(struct Queue* q, pid_t k, int d, char *name);
void deQueue(struct Queue* q);
void delete(struct Queue* q,int id);
int find_id_1(struct Queue* q);
int find_id_2(struct Queue* queue1,struct Queue* queue2);
#endif