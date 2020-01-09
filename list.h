//Willis Hershey wrote this whole damned thing and it is broken and beautiful someone please give him a job

#include <stdlib.h>

#ifdef THREAD_SAFE
	#include <semaphore.h> //This will likely only compile in THREAD_SAFE mode if you ask your compiler to link the pthread libraries
	#define wait(c) sem_wait(c) //Every thread-safe function will wait() before accessing listNode_t pointers
	#define post(c) sem_post(c) //And post() after it has finished reading/updating them. In THREAD_SAFE mode, semaphore.h functions are invoked
#else
	#define wait(c) {} //But outside of THREAD_SAFE mode, these calls become nops, and are removed in the preprocessor stage of compilation
	#define post(c) {}
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
#ifndef SIZEOF_LISTNODE_T
	#define SIZEOF_LISTNODE_T sizeof(listNode_t)
#endif
#if !defined INCLUDE_QUEUE && !defined INCLUDE_STACK
	#define INCLUDE_QUEUE
	#define INCLUDE_STACK
	#define INCLUDE_SEARCH_FUNCTIONS
#endif
#if defined INCLUDE_SEARCH_FUNCTIONS && !defined LISTTYPE_EQUAL
	#define LISTTYPE_EQUAL(a,b) a==b
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct listNode{ //List node for both queue and stack
  struct listNode *next; //Nodes contain a pointer to another node
  LISTTYPE data; //And a data element of some arbitrary user-specified type
}listNode_t;

#ifdef INCLUDE_QUEUE

typedef struct queue{ //Queue struct
  listNode_t *head; //Queues contain a pointer to the node at the beginning of the queue
  listNode_t *tail; //and a pointer to the node at the end of the queue
#ifdef THREAD_SAFE
  sem_t turn; //and in THREAD_SAFE mode, they have a semaphore to implement mutex
#endif
}queue_t;

queue_t* newQueue(){ //Returns pointer to empty fifo queue. Returns NULL on error
  queue_t *out=(queue_t*)malloc(sizeof(queue_t)); 
  if(!out) //If malloc failure
	return NULL; //Return NULL
  out->head=NULL; //Set head and tail to NULL, to indicate empty queue
  out->tail=NULL;
#ifdef THREAD_SAFE
  if(sem_init(&out->turn,0,1)==-1){ //In thread safe mode, initialize the semaphore to 1
	free(out); //If initialization fails, free malloced data
	return NULL; //And return NULL
  }
#endif
  return out; //But if everything succeeds, return a pointer to the malloced queue
}

int queueAdd(queue_t *queue,LISTTYPE n){ //Adds valid LISTTYPE to valid queue. returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  if(!queue||(INVALID_LISTTYPE(n))) //If NULL queue pointer or invalid LISTTYPE value
	return LIST_FAILURE; //Return failure
  listNode_t *hold=(listNode_t*)malloc(SIZEOF_LISTNODE_T); //Malloc space for a new node
  if(!hold) //If malloc failure
	return LIST_FAILURE; //Return failure
  hold->next=NULL; //This will be the new end of the list so the next pointer must be null
  hold->data=n; //Put the data in the node
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
  if(!queue) //If NULL queue pointer
	return LIST_FAILURE; //Return failure
  wait(&queue->turn); //(This only does something in THREAD_SAFE mode. See macro definition of wait(c))
  if(!queue->head){ //If queue does not have a head, then it has no elements
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See macro definition of post(c))
	return LIST_FAILURE; //Return failure
  }
  if(put) //If address is not NULL
	*put=queue->head->data; //Save the value at the head of the queue at the pointer address from input
  listNode_t *pt=queue->head; //Save a pointer to the current head (because we'll have to free the memory in a moment)
  queue->head=pt->next; //Set head to whatever the old head was pointing to (either another node or NULL)
  if(!queue->head) //If head has become null, it means this queue has no more elements
	queue->tail=NULL; //So set tail to null too
  post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  free(pt); //Free the data from the old head
  return LIST_SUCCESS; //And return success
}

int freeQueue(queue_t *queue){ //Frees memory associated with the queue. Access to any data left in the queue will be lost. Returns LIST_SUCCESS/FAILURE
  if(!queue) //If NULL queue pointer
	return LIST_FAILURE; //Return failure
  listNode_t *pt,*hold; //We access the data in the queue without using the mutex, so all other threads must finish before this function is called
  for(pt=queue->head;pt;pt=hold){ //Progressively go through the queue and free the data from all the nodes. (Requires that queue is NULL terminated)
	hold=pt->next;
	free(pt);
  }
#ifdef THREAD_SAFE
  sem_destroy(&queue->turn); //In THREAD_SAFE mode, make sure to destroy the semaphore, lest there be some memory leak in your system's implementation
#endif
  free(queue); //Free memory associated with the queue itself
  return LIST_SUCCESS; //And return success
}

#ifdef INCLUDE_SEARCH_FUNCTIONS

int queueRemoveValue(queue_t *queue,LISTTYPE value){ //Removes the first instance of a particular value from a valid stack. Returns LIST_SUCCESS/FAILURE
  if(!queue||(INVALID_LISTTYPE(value))) //If NULL queue pointer or invalid LISTTYPE value
	return LIST_FAILURE; //Return failure
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

int queueRemoveAll(queue_t *queue,LISTTYPE value){ //Removes all instances of some LISTTYPE value from a valid queue. Returns the number of values removed.
  int out=0; //This will be our output value. It will be incremented every time we free a node.
  if(!queue||(INVALID_LISTTYPE(value))) //If NULL queue pointer or invalid LISTTYPE value
	return out; //Return zero
  wait(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of wait(c))
  if(!queue->head){ //If the queue's head is NULL, then it has no elements
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	return out; //Return zero
  }
  listNode_t *pt,*hold; //pt will run through the list, and hold holds the next address for pt while pt frees a pointer
  for(pt=queue->head;pt&&(LISTTYPE_EQUAL(pt->data,value));pt=hold){ //For every consecutive LISTTYPE that matches value beginning at the head
	hold=pt->next;
	free(pt); //Free the memory
	++out; //And increment the output
  }
  if(!pt){ //If pt is now NULL (If every member of the list matched value)
	queue->head=queue->tail=NULL; //Set head and tail both to NULL, as the list is now empty
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	return out; //And return the number of nodes removed
  }
  queue->head=pt; //Otherwise, the first value that doesn't match is the new head.
  if(!pt->next){ //If the new head is also the tail, then we're done
	post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro deifnition of post(c))
	return out; //Return the number of nodes removed
  }
  listNode_t *last=pt; //Last will represent the last node in the queue that was not removed.
  for(pt=pt->next;pt!=queue->tail;pt=pt->next){ //For every other node in the queue that is not the tail
	if(LISTTYPE_EQUAL(pt->data,value)){ //If it matches in value
		hold=pt->next; //Save a pointer to the node after pt
		free(pt); //Free the memory for the node that matches
		++out; //Increment the output
		last->next=hold; //Point the last node to the node after the node we removed
		pt=last; //And point pt at the last node, so that it jumps to the correct place
	}
	else
		last=pt; //Otherwise, this is the last node that wasn't removed
  }
  if(LISTTYPE_EQUAL(pt->data,value)){ //If the tail matches in value
	free(pt); //Then we free the tail node
	++out; //Increment the output
	queue->tail=last; //Make the tail point to the last node that wasn't removed
	last->next=NULL; //And NULL terminate the list
  }
  post(&queue->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  return out; //Return the number of nodes removed
}

#endif
#endif

#ifdef INCLUDE_STACK

typedef struct stack{ //Stack struct
  listNode_t *top; //Stacks contain a pointer to the node at the top of the stack
#ifdef THREAD_SAFE
  sem_t turn; //and in THREAD_SAFE mode, a semaphore to implement mutex
#endif
}stack_t;

stack_t* newStack(){ //Returns pointer to new empty filo stack. Returns NULL on failure
  stack_t *out=(stack_t*)malloc(sizeof(stack_t)); //Malloc space for stack
  if(!out) //If malloc failure
	return NULL; //Return NULL
  out->top=NULL; //Set the top pointer to NULL to indicate empty stack
#ifdef THREAD_SAFE
  if(sem_init(&out->turn,0,1)==-1){ //In THREAD_SAFE mode, initialize the semaphore to 1 to implement mutex
	free(out); //If the initialization fails, free the malloced data
	return NULL; //And return NULL
  }
#endif
  return out; //But if everything succeeds, return a pointer to the malloced stack
}

int stackPush(stack_t *stack,LISTTYPE n){ //Pushes valid LISTTYPE onto valid stack. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  if(!stack||(INVALID_LISTTYPE(n))) //If NULL stack pointer, or invalid LISTTYPE value
	return LIST_FAILURE; //Return failure
  listNode_t *hold=(listNode_t*)malloc(SIZEOF_LISTNODE_T); //Malloc space for new node
  if(!hold) //If malloc failure
	return LIST_FAILURE; //Return failure
  hold->data=n; //Store the LISTTYPE value in the new node
  wait(&stack->turn); //(This only does something in THREAD_SAFE mode. See macro definition of wait(c))
  hold->next=stack->top; //Make the new node points to whatever the old top of the stack was (either another node or NULL)
  stack->top=hold; //Make the stack's top point to this new node
  post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  return LIST_SUCCESS; //And return success
}

int stackPop(stack_t *stack,LISTTYPE *put){ //Pops LISTTYPE from top of the stack and saves it in address from input. Returns LISTTYPE_FAILURE on failure
  if(!stack) //If NULL stack pointer
	return LIST_FAILURE; //Return failure
  wait(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of wait(c))
  if(!stack->top){ //If the stack's top is NULL, then the stack is empty
	post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	return LIST_FAILURE; //Return failure
  }
  if(put) //If address is not NULL
  	*put=stack->top->data; //Store the data from the current top at the address from input
  listNode_t *pt=stack->top; //Save a pointer to the current top (because we'll have to free the memory in a moment)
  stack->top=pt->next; //Make the stack's top point to whatever the old top was pointing to (either another node or NULL)
  post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  free(pt); //Free the data from the old stack's top
  return LIST_SUCCESS; //And return success
}

int freeStack(stack_t *stack){ //Frees memory associated with stack. Access to any data left in the stack will be lost. Returns LIST_SUCCESS/FAILURE
  if(!stack) //If NULL stack pointer
	return LIST_FAILURE; //Return failure
  listNode_t *pt,*hold; //We access the stack's data without mutex, so all other threads must finish before this function is called
  for(pt=stack->top;pt;pt=hold){ //Progressively go through the stack and free all of the nodes still in it
	hold=pt->next;
	free(pt);
  }
#ifdef THREAD_SAFE
  sem_destroy(&stack->turn); //In THREAD_SAFE mode, make sure to destroy the semaphore, lest there be some memory leak in your system's implementation
#endif
  free(stack); //Free memory associated with the stack itself
  return LIST_SUCCESS; //And return success
}

#ifdef INCLUDE_SEARCH_FUNCTIONS

int stackRemoveValue(stack_t *stack,LISTTYPE value){ //Removes the first instance of a particular value from a valid stack. Returns LIST_SUCCESS/FAILURE
  if(!stack||(INVALID_LISTTYPE(value))) //If NULL queue pointer or invalid LISTTYPE value
	return LIST_FAILURE; //Return failure
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

int stackRemoveAll(stack_t *stack,LISTTYPE value){ //Removes all instances of some LISTTYPE value from valid stack. Returns number of values removed
  int out=0; //This will serve as our output. It represents the number of values removed
  if(!stack||(INVALID_LISTTYPE(value))) //If NULL stack pointer or invalid LISTTYPE value
	return out; //Return zero
  wait(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of wait(c))
  if(!stack->top){ //If the top of the stack is NULL, then it has no values
	post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
	return out; //Return zero
  }
  listNode_t *pt,*hold; //Pt will iterate through the nodes in the stack, hold will hold the spot for point to jump to
  for(pt=stack->top;pt&&(LISTTYPE_EQUAL(pt->data,value));pt=hold){ //For each consecutive matching value beginning at the top of stack
	hold=pt->next;
	free(pt); //Free the node
	++out; //And increment the output
  }
  if(!pt){ //If every value in the stack matched
	stack->top=NULL; //Set the top to NULL to indicate the stack is empty
	post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro defninition of post(c))
	return out; //And return the number of values removed
  }
  stack->top=pt; //Otherwise, this first node that didn't have a matching value is the new top
  listNode_t *last=pt; //Last will represent the last value not removed, so we set it to the new top
  for(pt=pt->next;pt;pt=pt->next){ //For every remaining valaue in the stack
	if(LISTTYPE_EQUAL(pt->data,value)){ //If it matches in value
		hold=pt->next;
		free(pt); //Free it
		++out; //Increment the output
		last->next=hold; //Point the last node that didn't match to the one after the one that did
		pt=last;
	}
	else
		last=pt; //Otherwise, this is the last node that wasn't removed
  }
  post(&stack->turn); //(This only does something in THREAD_SAFE mode. See the macro definition of post(c))
  return out; //Return the number of values removed
}

#endif
#endif

#undef wait
#undef post

#ifdef __cplusplus
}
#endif
