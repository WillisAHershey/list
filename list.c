//Willis Hershey wrote this whole damned thing and it is broken and beautiful someone please give him a job

#include <stdlib.h>

#ifdef THREAD_SAFE
 //This will likely only compile in THREAD_SAFE mode if you ask your compiler to link the pthread libraries
	#include <semaphore.h>
//wait will be called before accessing pointers in the list structure and post will be called once mischief is managed in every thread-safe function
	#define wait(c) sem_wait(c)
	#define post(c) sem_post(c)
#else
//But outside of THREAD_SAFE mode, these calls do nothing and should be optimized out by the compiler
	#define wait(c) {}
	#define post(c) {}
#endif
#ifndef INVALID_LISTTYPE
//INVALID_LISTTYPE is a boolean expression, and it defaults to zero, as in 'No. This LISTTYPE is not invalid'. Default value should be optimized out
	#define INVALID_LISTTYPE(c) 0
#endif
#ifndef LIST_SUCCESS
//Default values for LIST_SUCCESS and LIST_FAILURE are 1 and 0 respectively, but these can be defined as any other integer value
//...but it's probably for the best if these values are not the same
	#define LIST_SUCCESS 1
#endif
#ifndef LIST_FAILURE
	#define LIST_FAILURE 0
#endif
#ifndef SIZEOF_LISTNODE_T
//This allows the user to customize the amount of space their LISTTYPE takes up
	#define SIZEOF_LISTNODE_T(c) sizeof(listNode_t)
#endif
#ifndef LISTTYPE_ASSIGN
//This allows the user to define how the data is to be stored in the nodes and also removed from the list
	#define LISTTYPE_ASSIGN(a,b) a=b
#endif
#if !defined INCLUDE_QUEUE && !defined INCLUDE_STACK
	#define INCLUDE_QUEUE
	#define INCLUDE_STACK
	#define INCLUDE_SEARCH_FUNCTIONS
#endif
#if defined INCLUDE_SEARCH_FUNCTIONS && !defined LISTTYPE_EQUAL
//LISTTYPE_EQUAL() must be defined manually by the programer if LISTTYPE is some sort of struct, as direct equivalence is likely not supported by the compiler
	#define LISTTYPE_EQUAL(a,b) a==b
#endif
#ifdef __cplusplus
//Although C++ is garbage, I will allow you to use this file as C++ code. You're welcome
extern "C" {
#endif

typedef struct listNode{ //List node for both queue and stack
  //Contains a pointer to the next node, and an element of some arbitrary user-specified type
  struct listNode *next;
  LISTTYPE data;
}listNode_t;

#ifdef INCLUDE_QUEUE

typedef struct queue{ //Queue struct
  //Queues contain a pointer to the first node in the queue and a pointer to the last one
  listNode_t *head;
  listNode_t *tail;
#ifdef THREAD_SAFE
  //And a semaphore to implement mutex
  sem_t turn;
#endif
}queue_t;

int queueInit(queue_t *queue){ //Initializes queue at given address. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  //If null queue pointer return failure
  if(!queue)
	return LIST_FAILURE;
  //Set head and tail to null to indicate empty queue
  queue->head=NULL;
  queue->tail=NULL;
#ifdef THREAD_SAFE
  //Initialize mutex semaphore (return failure on initialization failure)
  if(sem_init(&queue->turn,0,1)==-1)
	return LIST_FAILURE;
#endif
  //And return success
  return LIST_SUCCESS;
}

int queueAdd(queue_t *queue,LISTTYPE n){ //Adds valid LISTTYPE to valid queue. returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  //If null queue pointer or invalid value return failure
  if(!queue||(INVALID_LISTTYPE(n)))
	return LIST_FAILURE;
  //Malloc memory for new node (return failure on malloc failure)
  listNode_t *hold=malloc(SIZEOF_LISTNODE_T(n));
  if(!hold)
	return LIST_FAILURE;
  //Null terminate node and store value inside it
  hold->next=NULL;
  LISTTYPE_ASSIGN(hold->data,n);
  //If the queue already has a valid tail connect it to the new node and set the new node as the tail, otherwise set head and tail both to this new node
  wait(&queue->turn);
  if(queue->tail){
	queue->tail->next=hold;
	queue->tail=hold;
  }
  else
	queue->head=queue->tail=hold;
  //Return success
  post(&queue->turn);
  return LIST_SUCCESS;
}

int queueRemove(queue_t *queue,LISTTYPE *put){ //Places LISTTYPE from head of queue at address from input. Returns LIST_SUCCESS on success and LIST_FAIULRE on failure.
  //If null queue pointer return failure
  if(!queue)
	return LIST_FAILURE;
  //If the queue's head is null it has no values, so we can't remove. Return failure
  wait(&queue->turn);
  if(!queue->head){
	post(&queue->turn);
	return LIST_FAILURE;
  }
  //If the pointer where we're supposed to put the data is not null, save the value there
  if(put)
	LISTTYPE_ASSIGN(*put,queue->head->data);
  //Point the head to the next node, if the head is now null set the tail to null too, free the memory from the old head, and return success
  listNode_t *pt=queue->head;
  queue->head=pt->next;
  if(!queue->head)
	queue->tail=NULL;
  post(&queue->turn);
  free(pt);
  return LIST_SUCCESS;
}

int queueDestroy(queue_t *queue){ //Frees memory associated with the queue. Access to any data left in the queue will be lost. Returns LIST_SUCCESS/FAILURE
  //If null queue pointer return failure
  if(!queue)
	return LIST_FAILURE;
  //For every node left in the queue free it (not thread safe)
  listNode_t *pt,*hold;
  for(pt=queue->head;pt;pt=hold){
	hold=pt->next;
	free(pt);
  }
#ifdef THREAD_SAFE
  //Destroy the semaphore
  sem_destroy(&queue->turn);
#endif
  //And return success
  return LIST_SUCCESS;
}

#ifdef INCLUDE_SEARCH_FUNCTIONS

int queueRemoveValue(queue_t *queue,LISTTYPE value){ //Removes the first instance of a particular value from a valid stack. Returns LIST_SUCCESS/FAILURE
  //If null queue pointer or invalid value return failure
  if(!queue||(INVALID_LISTTYPE(value)))
	return LIST_FAILURE;
  //If the queue's head is null, it has no values so we can't remove. Return failure
  wait(&queue->turn);
  if(!queue->head){
	post(&queue->turn);
	return LIST_FAILURE;
  }
  //If the head node's value matches, point it to the next node. If the next node is null, set tail to null too. Free memory, and return success
  listNode_t *pt=queue->head;
  if(LISTTYPE_EQUAL(pt->data,value)){
	queue->head=pt->next;
	if(!pt->next)
		queue->tail=NULL;
	post(&queue->turn);
	free(pt);
	return LIST_SUCCESS;
  }
  //If the first node in the queue is also the last, then we've already checked it and it didn't match, so return failure
  if(!pt->next){
	post(&queue->turn);
	return LIST_FAILURE;
  }
  //For every node in the queue that is not the head or tail, if it matches, point the previous node to the next node, free the memory, and return success
  listNode_t *run=pt;
  for(pt=pt->next;pt!=queue->tail;pt=pt->next){
	if(LISTTYPE_EQUAL(pt->data,value)){
		run->next=pt->next;
		post(&queue->turn);
		free(pt);
		return LIST_SUCCESS;
	}
	run=pt;
  }
  //If the first matching value is in the tail, point tail to the node right before it, null terminate the queue, free the memory, and return success
  if(LISTTYPE_EQUAL(pt->data,value)){
	queue->tail=run;
	run->next=NULL;
	post(&queue->turn);
	free(pt);
	return LIST_SUCCESS;
  }
  //Otherwise the value is not in the queue. Return failure
  post(&queue->turn);
  return LIST_FAILURE;
}

int queueRemoveAll(queue_t *queue,LISTTYPE value){ //Removes all instances of some LISTTYPE value from a valid queue. Returns the number of values removed.
  //If null queue pointer or invalid value return zero
  int out=0;
  if(!queue||(INVALID_LISTTYPE(value)))
	return out;
  //If the head of the queue is null, then it has no values and we can't remove. Return zero
  wait(&queue->turn);
  if(!queue->head){
	post(&queue->turn);
	return out;
  }
  //For every consecutive matching node beginning at the head, free it and increment the output. Set the head to the first non-matching node.
  listNode_t *pt,*hold;
  for(pt=queue->head;pt&&(LISTTYPE_EQUAL(pt->data,value));pt=hold){
	hold=pt->next;
	free(pt);
	++out;
  }
  queue->head=pt;
  //If every node in the queue matched, set tail also to null, and return the number of nodes removed
  if(!pt){
	queue->tail=NULL;
	post(&queue->turn);
	return out;
  }
  //If the new head is the last node (which tail is already pointing to), return the number of nodes removed
  if(!pt->next){
	post(&queue->turn);
	return out;
  }
  //For each remaining node that is not the tail, if it matches, point the previous node to the next one, free the memory, and increment the output
  listNode_t *last=pt;
  pt=pt->next;
  while(pt!=queue->tail)
	if(LISTTYPE_EQUAL(pt->data,value)){
		hold=pt->next;
		free(pt);
		++out;
		last->next=hold;
		pt=hold;
	}
	else{
		last=pt;
		pt=pt->next;
	}
  //If the tail node matches, free the memory, increment output, set the tail to the last node that wasn't removed, null terminate the queue, and return num nodes removed
  if(LISTTYPE_EQUAL(pt->data,value)){
	free(pt);
	++out;
	queue->tail=last;
	last->next=NULL;
  }
  post(&queue->turn);
  return out;
}

#endif
#endif

#ifdef INCLUDE_STACK

typedef struct stack{ //Stack struct
  //Contains a pointer to the node at the top
  listNode_t *top;
#ifdef THREAD_SAFE
  //and a semaphore to implement mutex
  sem_t turn;
#endif
}stack_t;

int stackInit(stack_t *stack){ //Initializes filo stack at given address. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  //If null pointer return failure
  if(!stack)
	return LIST_FAILURE;
  //Set top to null to indicate the stack is empty
  stack->top=NULL;
#ifdef THREAD_SAFE
  //Initialize the mutex semaphore (return failure if initialization fails)
  if(sem_init(stack->turn,0,1)==-1)
	return LIST_FAILURE;
#endif
  return LIST_SUCCESS;
}

int stackPush(stack_t *stack,LISTTYPE n){ //Pushes valid LISTTYPE onto valid stack. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  //If null stack pointer or invalid value return failure
  if(!stack||(INVALID_LISTTYPE(n)))
	return LIST_FAILURE;
  //Malloc space for node (return failure on malloc failure)
  listNode_t *hold=malloc(SIZEOF_LISTNODE_T(n));
  if(!hold)
	return LIST_FAILURE;
  //Place the value in the new node, point the new node at the old top, point the top to the new node, and return success
  LISTTYPE_ASSIGN(hold->data,n);
  wait(&stack->turn);
  hold->next=stack->top;
  stack->top=hold;
  post(&stack->turn);
  return LIST_SUCCESS;
}

int stackPop(stack_t *stack,LISTTYPE *put){ //Pops LISTTYPE from top of the stack and saves it in address from input. Returns LISTTYPE_FAILURE on failure
  //if NUll pointer return failure
  if(!stack)
	return LIST_FAILURE;
   //(This only does something in THREAD_SAFE mode. See the macro definition of wait(c))
  wait(&stack->turn); 
  //If the stack's top in NULL, it is empty, so we can't pop. Return failure.
  if(!stack->top){
	post(&stack->turn);
	return LIST_FAILURE;
  }
  //If the pointer where we're supposed to pop the data is not NULL, put the data there
  if(put)
  	LISTTYPE_ASSIGN(*put,stack->top->data);
  //Set the new top of the stack, free the data associated with the old top, and return success
  listNode_t *pt=stack->top;
  stack->top=pt->next;
  post(&stack->turn);
  free(pt);
  return LIST_SUCCESS;
}

int stackDestroy(stack_t *stack){ //Frees memory associated with stack. Access to any data left in the stack will be lost. Returns LIST_SUCCESS/FAILURE
  //If null stack pointer return failure
  if(!stack)
	return LIST_FAILURE;
  //Progressively go through the stack and free every remaining node (not thread safe)
  listNode_t *pt,*hold;
  for(pt=stack->top;pt;pt=hold){
	hold=pt->next;
	free(pt);
  }
#ifdef THREAD_SAFE
  //Destroy the mutex semaphore
  sem_destroy(&stack->turn);
#endif
  return LIST_SUCCESS;
}

#ifdef INCLUDE_SEARCH_FUNCTIONS

int stackRemoveValue(stack_t *stack,LISTTYPE value){ //Removes the first instance of a particular value from a valid stack. Returns LIST_SUCCESS/FAILURE
  //If null stack pointer or invalid value return failure
  if(!stack||(INVALID_LISTTYPE(value)))
	return LIST_FAILURE;
  //If the top is null, the stack has no elements so we can't remove. Return failure
  wait(&stack->turn);
  if(!stack->top){
	post(&stack->turn);
	return LIST_FAILURE;
  }
  //If the value at the top of the stack matches, set the top to the next node, free memory for old top, and return success
  listNode_t *pt=stack->top;
  if(LISTTYPE_EQUAL(pt->data,value)){
	stack->top=pt->next;
	post(&stack->turn);
	free(pt);
	return LIST_SUCCESS;
  }
  //Otherwise, for every non-top node in the stack, if its value matches, point the node before it to the node after, free memory, and return success
  listNode_t *run=pt;
  for(pt=pt->next;pt;pt=pt->next){
	if(LISTTYPE_EQUAL(pt->data,value)){
		run->next=pt->next;
		post(&stack->turn);
		free(pt);
		return LIST_SUCCESS;
	}
	run=pt;
  }
  //If value is not found in the stack return failure
  post(&stack->turn);
  return LIST_FAILURE;
}

int stackRemoveAll(stack_t *stack,LISTTYPE value){ //Removes all instances of some LISTTYPE value from valid stack. Returns number of values removed
  //If null stack pointer or invalid value return zero
  int out=0;
  if(!stack||(INVALID_LISTTYPE(value)))
	return out;
  //If top of stack is null, it has no nodes. Return zero
  wait(&stack->turn);
  if(!stack->top){
	post(&stack->turn);
	return out;
  }
  //For each consecutive matching value beginning at the top, free it and increment the output, then point top to the first non-matching node 
  listNode_t *pt,*hold;
  for(pt=stack->top;pt&&(LISTTYPE_EQUAL(pt->data,value));pt=hold){
	hold=pt->next;
	free(pt);
	++out;
  }
  stack->top=pt;
  //If every value in the stack matched and the top is now null, return the number of elements removed
  if(!pt){
	post(&stack->turn);
	return out;
  }
  //For each remaining node in the stack, if it matches, point the node before to the node after, free memory, and increment output
  listNode_t *last=pt;
  pt=pt->next;
  while(pt)
	if(LISTTYPE_EQUAL(pt->data,value)){
		hold=pt->next;
		free(pt);
		++out;
		last->next=hold;
		pt=hold;
	}
	else{
		last=pt;
		pt=pt->next;
	}
  //When we run out of nodes, return the number removed
  post(&stack->turn);
  return out;
}

#endif
#endif

#undef wait
#undef post

#ifdef __cplusplus
}
#endif
