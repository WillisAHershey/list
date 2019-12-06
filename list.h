//Willis Hershey wrote this thing on 12/4/19 someone please give him a job

#include <stdlib.h>

#ifdef THREAD_SAFE
#include <semaphore.h>
#endif
//This file defines two generic list structures, queue_t and stack_t, which are customizable to hold any data type, and are configured at compiletime

//To use, define the following macros before this file is included in some other file. (This file will not compile on its own)

//Define LISTTYPE to the datatype stored in the list, i.e. int, double, some sort of pointer etc.

//Define LISTTYPE_FAILURE to some value of LISTTYPE that the program will return if an error occurs when executing a queueRemove or stackPop call

//If LISTTYPE_FAILURE does not make sense in your code, ie all possible values of LISTTYPE are valid, and you'd prefer the code to exit on error,
//Simply define EXIT_ON_ERROR, and it will do just that, but keep in mind that your program will crash if you try to remove or pop from an empty list.

//INVALID_LISTTYPE(c) is some boolean expression (or some set of boolean expressions ored together) which should be rejected by queueAdd and stackPush.
//(note: if you define INVALID_LISTTYPE as a non-zero constant, queueAdd and stackPush will always return LIST_FAILURE and this code becomes useless. Don't do that)
//INVALID_LISTTYPE defaults to 0, which means all inputs are accepted as valid unless this is redefined.

//Optionally, you may also define LIST_SUCCESS and LIST_FAILURE as any int values of your choice, but if you choose not to, they default to 1 and 0 respectively

//LISTTYPE and INVALID_LISTTYPE will be undefined at the end of this file, and will therefore be unavailable in your code after #include "list.h"
//(This is helpful if you want to include it several times) LIST_SUCCESS, LIST_FAILURE, and LISTTYPE_FAILURE will remain defined so they can be used for error checking

//If multiple threads will be accessing the same stack or queue at the same time, define THREAD_SAFE, and this file will automatically implement mutex through semaphores
//The functions queueNotEmpty and stackNotEmpty are not thread safe, even with flag, as they lose meaning in multi-threading models.
//freeQueue and freeStack are also not thread safe, the programmer must ensure all threads have finished accessing the lists before freeing the associated memory.


//FOR EXAMPLE, if you want to have a list of ints and you want -1 to be indicative of invalid input AND error, you might code the following:
//
//#define LISTTYPE int
//#define LISTTYPE_FAILURE -1
//#define INVALID_LISTTYPE(c) c==-1
//#define LIST_SUCCESS 0
//#define LIST_FAILURE -1
//#include "list.h"
//
//...so on and so forth
//
//Or alternatively if your type was an arbitrary pointer type, such as mystruct_t*, and NULL should be returned on error, but all values should be accepted in push and add
//
//#define LISTTYPE mystruct_t*
//#define LISTTYPE_FAILURE NULL
//#define INVALID_LISTTYPE(c) 0 //If INVALID_LISTTYPE(c) is 0, all LISTTYPE values are considered valid (read as INVALID_LISTTYPE(c) is always false)
//#include list.h
//
//...yada yada yada
//
//There is no special precompiler-magic setup so that this file can only be included once, for the simple reason that someone may legitimately want to include it
//several times. Consider the following scenario where the programmer defines two different types of lists for his or her program.

//#define LISTTYPE int
//#define LISTTYPE_FAILURE 0                //Define all of the necessary macros for integer lists
//#define INVALID_LISTTYPE(c) !c

//#define listNode intListNode
//#define queue intQueue
//#define stack intStack
//#define listNode_t intListNode_t
//#define queue_t intQueue_t
//#define stack_t intStack_t
//#define newQueue() newIntQueue()
//#define newStack() newIntStack()
//#define queueAdd(a,b) intQueueAdd(a,b)          //Give all of the structures and functions an alias of some sort
//#define stackPush(a,b) intStackPush(a,b)
//#define queueRemove(a) intQueueRemove(a)
//#define stackPop(a) intStackPop(a)
//#define freeQueue(a) freeIntQueue(a)
//#define freeStack(a) freeIntStack(a)

//#include "list.h"                    //Include the file

//#define LISTTYPE mystruct_t*
//#undef LISTTYPE_FAILURE
//#define LISTTYPE_FAILURE NULL          //Redefine the necessary macros for a pointer list
//#define INVALID_LISTTYPE(c) !c

//(if you wish to redefine LIST_SUCCESS and LIST_FAILURE you must undef them first)

//#undef listNode
//#undef queue
//#undef stack
//#undef listNode_t
//#undef queue_t
//#undef stack_t
//#undef newQueue
//#undef newStack
//#undef queueAdd                   //Undefine all of the aliases, so the functions and structures take on their default names
//#undef stackPush
//#undef queueRemove
//#undef stackPop
//#undef freeQueue
//#undef freeStack

//#include "list.h"        //And include the file again

//Albeit it's a lengthy and annoying bunch of code, but it works

//This code (at least in theory) produces no memory leaks unless you're careless with it and forget to call the corresponding free function to the type of list you use

#ifndef INVALID_LISTTYPE
	#define INVALID_LISTTYPE(c) 0 //INVALID_LISTTYPE defaults to 'all values are valid'
#endif
#ifndef LIST_SUCCESS
	#define LIST_SUCCESS 1 //Default values for LIST_SUCCESS and LIST_FAILURE are 1 and 0 respectively. These can be overwritten to any integer value without problem
#endif
#ifndef LIST_FAILURE
	#define LIST_FAILURE 0
#endif
#ifndef LISTTYPE_FAILURE
	#define EXIT_ON_ERROR
#endif

typedef struct listNode{ //List node for both queue and stack
  LISTTYPE data;
  struct listNode *next;
}listNode_t;

typedef struct queue{ //Queue struct
  listNode_t *head;
  listNode_t *tail;
#ifdef THREAD_SAFE
  sem_t *turn;
#endif
}queue_t;

typedef struct stack{ //Stack struct
  listNode_t *top;
#ifdef THREAD_SAFE
  sem_t *turn;
#endif
}stack_t;

queue_t* newQueue(){ //Returns pointer to empty fifo queue. Returns NULL on error
  queue_t *out=(queue_t*)malloc(sizeof(queue_t));
  if(!out)
	return NULL;
  out->head=NULL;
  out->tail=NULL;
#ifdef THREAD_SAFE
  out->turn=sem_init(&out->turn,0,1);
#endif
  return out;
}

int queueAdd(queue_t *queue,LISTTYPE new){ //Adds LISTTYPE to valid queue. returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  if(!queue||(INVALID_LISTTYPE(new)))
	return LIST_FAILURE;
  listNode_t *hold=(listNode_t*)malloc(sizeof(listNode_t));
  if(!hold)
	return LIST_FAILURE;
  hold->next=NULL;
  hold->data=new;
#ifdef THREAD_SAFE
  sem_wait(&queue->turn);
#endif
  if(queue->tail){
	queue->tail->next=hold;
	queue->tail=hold;
  }
  else
	queue->head=queue->tail=hold;
#ifdef THREAD_SAFE
  sem_post(&queue->turn);
#endif
  return LIST_SUCCESS;
}

LISTTYPE queueRemove(queue_t *queue){ //Returns LISTTYPE at head of queue and removes it from queue, returns LISTTYPE_FAILURE on failure
#ifdef THREAD_SAFE
  if(!queue)
#else
  if(!queue||!queue->head)
#endif
#ifdef EXIT_ON_ERROR
	exit(1);
#else
	return LISTTYPE_FAILURE;
#endif
#ifdef THREAD_SAFE
  sem_wait(&queue->turn);
  if(!queue->head){
	  sem_post(&queue->turn);
#ifdef EXIT_ON_ERROR
	  exit(1);
#else
  	return LISTTYPE_FAILURE;
#endif
  }
#endif
  LISTTYPE out=queue->head->data;
  listNode_t *pt=queue->head;
  queue->head=pt->next;
  if(!queue->head)
	queue->tail=NULL;
#ifdef THREAD_SAFE
  sem_post(&queue->turn);
#endif
  free(pt);
  return out;
}

int queueNotEmpty(queue_t *queue){ //Returns 1 if there is an element in the queue, 0 on error and otherwise
  if(!queue||!queue->head)
	return 0;
  return 1;
}

int freeQueue(queue_t *queue){ //Frees memory associated with the queue. Access to any data left in the queue will be lost. Returns LIST_SUCCESS/FAILURE
  if(!queue)
	return LIST_FAILURE;
  listNode_t *pt=queue->head,*hold;
  while(pt){
	hold=pt->next;
	free(pt);
	pt=hold;
  }
  free(queue);
  return LIST_SUCCESS;
}

stack_t* newStack(){ //Returns pointer to new empty filo stack. Returns NULL on failure
  stack_t *out=(stack_t*)malloc(sizeof(stack_t));
  if(!out)
	return NULL;
  out->top=NULL;
#ifdef THREAD_SAFE
  sem_init(&out->turn,0,1);
#endif
  return out;
}

int stackPush(stack_t *stack,LISTTYPE new){ //Pushes a LISTTYPE onto valid stack. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  if(!stack||(INVALID_LISTTYPE(new)))
	return LIST_FAILURE;
  listNode_t *node=(listNode_t*)malloc(sizeof(listNode_t));
  if(!node)
	return LIST_FAILURE;
  node->data=new;
#ifdef THREAD_SAFE
  sem_wait(&stack->turn);
#endif
  node->next=stack->top;
  stack->top=node;
#ifdef THREAD_SAFE
  sem_post(&stack->turn);
#endif
  return LIST_SUCCESS;
}

LISTTYPE stackPop(stack_t *stack){ //Returns LISTTYPE on top of the stack, and pops it from the stack. Returns LISTTYPE_FAILURE on failure
#ifdef THREAD_SAFE
  if(!stack)
#else
  if(!stack||!stack->top)
#endif
#ifdef EXIT_ON_ERROR
	exit(1);
#else
	return LISTTYPE_FAILURE;
#endif
#ifdef THREAD_SAFE
  sem_wait(&stack->turn);
  if(!stack->top){
	  sem_post(&stack->turn);
#ifdef EXIT_ON_ERROR
	  exit(1);
#else
	  return LISTTYPE_FAILURE;
#endif
  }
#endif
  listNode_t *pt=stack->top->next;
  LISTTYPE out=stack->top->data;
  free(stack->top);
  stack->top=pt;
#ifdef THREAD_SAFE
  sem_post(&stack->turn);
#endif
  return out;
}

int stackNotEmpty(stack_t *stack){ //Returns 1 if there is an element on the stack, returns 0 on error and otherwise
  if(!stack||!stack->top)
	return 0;
  return 1;
}

int freeStack(stack_t *stack){ //Frees memory associated with stack. Access to any data left in the stack will be lost. Returns LIST_SUCCESS/FAILURE
  if(!stack)
	return LIST_FAILURE;
  listNode_t *pt=stack->top,*hold;
  while(pt){
	hold=pt->next;
	free(pt);
	pt=hold;
  }
  free(stack);
  return LIST_SUCCESS;
}

#undef LISTTYPE
#undef INVALID_LISTTYPE
