//Willis Hershey wrote this whole damned thing and it is broken and beautiful someone please give him a job

#include <stdlib.h>

#ifdef THREAD_SAFE
	#include <semaphore.h> //This will likely only compile in THREAD_SAFE mode if you ask your compiler to link the pthread libraries
#endif
#ifndef INVALID_LISTTYPE
	#define INVALID_LISTTYPE(c) 0 //This is a boolean expression, so INVALID_LISTTYPE defaults to 'all values are valid'
#endif
#ifndef LIST_SUCCESS
	#define LIST_SUCCESS 1 //Default values for LIST_SUCCESS and LIST_FAILURE are 1 and 0 respectively. These can be overwritten to any integer value without problem
#endif                          //but it's probably best if they are not the same
#ifndef LIST_FAILURE
	#define LIST_FAILURE 0
#endif
#ifndef LISTTYPE_EQUAL
	#define LISTTYPE_EQUAL(a,b) a==b
#endif
#ifdef THREAD_SAFE
	#define wait(c) sem_wait(c)
	#define post(c) sem_post(c)
#else
	#define wait(c) {}
	#define post(c) {}
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct listNode{ //List node for both queue and stack
  LISTTYPE data; //Nodes contain a data element of some arbitrary type
  struct listNode *next; //and a pointer to another node
}listNode_t;

typedef struct queue{ //Queue struct
  listNode_t *head; //Queues contain a pointer to the node at the beginning of the queue
  listNode_t *tail; //and a pointer to the node at the end of the queue
#ifdef THREAD_SAFE
  sem_t turn; //and in THREAD_SAFE mode, they have a semaphore to implement mutex
#endif
}queue_t;

typedef struct stack{ //Stack struct
  listNode_t *top; //Stacks contain a pointer to the node at the top of the stack
#ifdef THREAD_SAFE
  sem_t turn; //and in THREAD_SAFE mode, a semaphore to implement mutex
#endif
}stack_t;

queue_t* newQueue(){ //Returns pointer to empty fifo queue. Returns NULL on error
  queue_t *out=(queue_t*)malloc(sizeof(queue_t)); 
  if(!out) //if malloc failure
	return NULL; //return NULL
  out->head=NULL; //set head and tail to NULL, to indicate empty queue
  out->tail=NULL;
#ifdef THREAD_SAFE
  if(sem_init(&out->turn,0,1)==-1){ //In thread safe mode, initialize the semaphore to 1
	free(out); //if initialization fails, free malloced data
	return NULL; //and return NULL
  }
#endif
  return out; //but if everything succeeds, return a pointer to the malloced queue
}

int queueAdd(queue_t *queue,LISTTYPE n){ //Adds LISTTYPE to valid queue. returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  if(!queue||(INVALID_LISTTYPE(n))) //If NULL queue pointer or invalid LISTTYPE value
	return LIST_FAILURE; //Return failure
  listNode_t *hold=(listNode_t*)malloc(sizeof(listNode_t)); //malloc a space for a new node
  if(!hold) //If malloc failure
	return LIST_FAILURE; //return failure
  hold->next=NULL; //This will be the new end of the list so the next pointer must be null
  hold->data=n; //put the data in the node
  wait(&queue->turn); //(This only does something in THREAD_SAFE mode. See macro definition of wait(c))
  if(queue->tail){ //If the queue already has a tail
	queue->tail->next=hold; //Then connect the current tail to this new node
	queue->tail=hold; //And make tail point to this new node, since this is the new tail
  }
  else //If the queue does not have a tail, it means it also does not have a head
	queue->head=queue->tail=hold; //so set them both to point at this new node
  post(&queue->turn); //(This only does something in THREAD_SAFE mode. See macro definition of post(c))
  return LIST_SUCCESS; //Return success
}

int queueRemove(queue_t *queue,LISTTYPE *put){ //Places LISTTYPE from head of queue at address from input. Returns LIST_SUCCESS on success and LIST_FAIULRE on failure.
  if(!queue||!put) //If NULL queue pointer or NULL LISTTYPE pointer
	return LIST_FAILURE; //Return failure
  wait(&queue->turn); //(This only does something in THREAD_SAFE mode. See macro definition of wait(c))
  if(!queue->head){ //If queue does not have a head, then it has no elements
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See macro definition of post(c))
	return LIST_FAILURE; //Return failure
  }
  *put=queue->head->data; //Save the value at the head of the queue at the pointer address from input
  listNode_t *pt=queue->head; //Save a pointer to the current head (because we'll have to free the memory in a moment)
  queue->head=pt->next; //Set head to whatever the old head was pointing to (either another node or NULL)
  if(!queue->head) //If head has become null, it means this queue has no more elements
	queue->tail=NULL; //so set tail to null too
  post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  free(pt); //Free the data from the old head
  return LIST_SUCCESS; //And return success
}

int queueRemoveValue(queue_t *queue,LISTTYPE value){ //Removes the first instance of a particular value from a valid stack. Returns LIST_SUCCESS/FAILURE
  if(!queue||(INVALID_LISTTYPE(value))){ //If NULL queue pointer or invalid LISTTYPE value
	return LIST_FAILURE; //Return failure
  }
  wait(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of wait(c))
  if(!queue->head){ //If the queue's head is NULL, then the queue is empty
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	return LIST_FAILURE; //Return failure
  }
  listNode_t *pt=queue->head; //Save a pointer to the head of the queue
  if(LISTTYPE_EQUAL(pt->data,value)){ //If the head of the queue contains the value we're loking for
	queue->head=pt->next; //Point the queue's head at whatever is in its next pointer (either another node or NULL)
	if(!pt->next) //If head is now NULL, then there are no elements
		queue->tail=NULL; //So set tail to NULL too
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	free(pt); //Free memory associated with the old head
	return LIST_SUCCESS; //And return success
  }
  listNode_t *run=pt; //Otherwise, create a pointer that will trail behind pt, so we can link the list back together if and when we remove a node
  if(!pt->next){ //If pt points to NULL, then this queue has 1 element, and we already checked it
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	return LIST_FAILURE; //Return failure
  }
  for(pt=pt->next;pt!=queue->tail;pt=pt->next){ //So for every node in the queue that is not the head or the tail
	if(LISTTYPE_EQUAL(pt->data,value)){ //If we find a node that is equal to the value we're looking for
		run->next=pt->next; //Point the node behind the one we found to whatever comes after (either another node or NULL)
		post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
		free(pt); //Free the memory associated with the node
		return LIST_SUCCESS; //And return success
	}
	run=pt;
  }
  if(LISTTYPE_EQUAL(pt->data,value)){ //If the first instance of the value we're looking for is in the tail
	queue->tail=run; //Set the tail to the node right before the tail
	run->next=NULL; //NULL terminate the new tail
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	free(pt); //Free the memory associated with the old tail
	return LIST_SUCCESS; //And return success
  } //If control makes it to here, then the value we're looking for is not present in the queue
  post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  return LIST_FAILURE; //Return failure
}

int freeQueue(queue_t *queue){ //Frees memory associated with the queue. Access to any data left in the queue will be lost. Returns LIST_SUCCESS/FAILURE
  if(!queue) //If NULL queue pointer
	return LIST_FAILURE; //Return failure
  listNode_t *pt=queue->head,*hold; //We access the data in the queue without using the mutex, so all other threads must finish before this function is called
  while(pt){ //Progressively go through the queue and free the data from all the nodes
	hold=pt->next;
	free(pt);
	pt=hold;
  }
#ifdef THREAD_SAFE
  sem_destroy(&queue->turn); //In THREAD_SAFE mode, make sure to destroy the semaphore, lest there be some memory leak in your system's implementation
#endif
  free(queue); //Free memory associated with the queue itself
  return LIST_SUCCESS; //And return success
}

stack_t* newStack(){ //Returns pointer to new empty filo stack. Returns NULL on failure
  stack_t *out=(stack_t*)malloc(sizeof(stack_t)); //Malloc space for stack
  if(!out) //If malloc failure
	return NULL; //return NULL
  out->top=NULL; //Set the top pointer to NULL to indicate empty stack
#ifdef THREAD_SAFE
  if(sem_init(&out->turn,0,1)==-1){ //in THREAD_SAFE mode, initialize the semaphore to 1 to implement mutex
	free(out); //If the initialization fails, free the malloced data
	return NULL; //and return NULL
  }
#endif
  return out; //But if everything succeeds, return a pointer to the malloced stack
}

int stackPush(stack_t *stack,LISTTYPE n){ //Pushes a LISTTYPE onto valid stack. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  if(!stack||(INVALID_LISTTYPE(n))) //If NULL stack pointer, or invalid LISTTYPE value
	return LIST_FAILURE; //return failure
  listNode_t *hold=(listNode_t*)malloc(sizeof(listNode_t)); //Malloc space for new node
  if(!hold) //If malloc failure
	return LIST_FAILURE; //return failure
  hold->data=n; //Store the LISTTYPE value in the new node
  wait(&stack->turn); //(This only does something in THREAD_SAFE mode. See macro definition of wait(c))
  hold->next=stack->top; //Make the new node point to whatever the old top of the stack was (either another node or NULL)
  stack->top=hold; //Make the stack's top point to this new node
  post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  return LIST_SUCCESS; //And return success
}

int stackPop(stack_t *stack,LISTTYPE *put){ //Returns LISTTYPE on top of the stack, and pops it from the stack. Returns LISTTYPE_FAILURE on failure
  if(!stack||!put) //If NULL stack pointer, or NULL LISTTYPE pointer
	return LIST_FAILURE; //Return failure
  wait(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of wait(c))
  if(!stack->top){ //If the stack's top is NULL
	post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	return LIST_FAILURE; //Return failure
  }
  *put=stack->top->data; //Store the data from the current top at the address from input
  listNode_t *pt=stack->top; //Save a pointer to the current top (because we'll have to free the memory in a moment)
  stack->top=pt->next; //Make the stack's top point to whatever the old top was pointing to (either another node or NULL)
  post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  free(pt); //Free the data from the old stack's top
  return LIST_SUCCESS; //And return success
}

int stackRemoveValue(stack_t *stack,LISTTYPE value){ //Removes the first instance of a particular value from a valid stack Returns LIST_SUCCESS/FAILURE
  if(!stack||(INVALID_LISTTYPE(value))){ //If NULL queue pointer or invalid LISTTYPE value
	return LIST_FAILURE; //Return failure
  }
  wait(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of wait(c))
  if(!stack->top){ //If the stack's top is NULL, then the stack is empty
	post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	return LIST_FAILURE; //Return failure
  }
  listNode_t *pt=stack->top; //Save a pointer to the head of the stack
  if(LISTTYPE_EQUAL(pt->data,value)){ //If the top of the stack contains the value we're loking for
	stack->top=pt->next; //Point the stack's top at whatever is in its next pointer (either another node or NULL)
	post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	free(pt); //Free memory associated with the old top
	return LIST_SUCCESS; //And return success
  }
  listNode_t *run=pt; //Otherwise, create a pointer that will trail behind pt, so we can link the list back together if and when we remove a node
  for(pt=pt->next;pt;pt=pt->next){ //So for every other node in the stack
	if(LISTTYPE_EQUAL(pt->data,value)){ //If we find a node with a LISTTYPE equal to the value we're looking for
		run->next=pt->next; //Point the node behind the one we found to whatever comes after (either another node or NULL)
		post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
		free(pt); //Free the memory associated with the node
		return LIST_SUCCESS; //And return success
	}
	run=pt;
  } //If control makes it past this point, it means no element in the stack contains the value we're looking for
  post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  return LIST_FAILURE; //Return failure
}

int freeStack(stack_t *stack){ //Frees memory associated with stack. Access to any data left in the stack will be lost. Returns LIST_SUCCESS/FAILURE
  if(!stack) //If NULL stack pointer
	return LIST_FAILURE; //Return failure
  listNode_t *pt=stack->top,*hold; //We access the stack's data without mutex, so all other threads must finish before this function is called
  while(pt){ //Progressively go through the stack and free all of the nodes still in it
	hold=pt->next;
	free(pt);
	pt=hold;
  }
#ifdef THREAD_SAFE
  sem_destroy(&stack->turn); //In THREAD_SAFE mode, make sure to destroy the semaphore, lest there be some memory leak in your system's implementation
#endif
  free(stack); //Free memory associated with the stack itself
  return LIST_SUCCESS; //And return success
}

#ifdef __cplusplus
}
#endif
