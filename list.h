#include <stdlib.h>

#ifdef THREAD_SAFE
#include <semaphore.h>
#endif

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
#ifndef LISTTYPE_EQUAL
	#define LISTTYPE_EQUAL(a,b) a==b
#endif

typedef struct listNode{ //List node for both queue and stack
  LISTTYPE data;
  struct listNode *next;
}listNode_t;

typedef struct queue{ //Queue struct
  listNode_t *head;
  listNode_t *tail;
#ifdef THREAD_SAFE
  sem_t turn;
#endif
}queue_t;

typedef struct stack{ //Stack struct
  listNode_t *top;
#ifdef THREAD_SAFE
  sem_t turn;
#endif
}stack_t;

queue_t* newQueue(){ //Returns pointer to empty fifo queue. Returns NULL on error
  queue_t *out=(queue_t*)malloc(sizeof(queue_t));
  if(!out)
	return NULL;
  out->head=NULL;
  out->tail=NULL;
#ifdef THREAD_SAFE
  sem_init(&out->turn,0,1);
#endif
  return out;
}

int queueAdd(queue_t *queue,LISTTYPE new){ //Adds LISTTYPE to valid queue. returns LIST_SUCCESS on success, and LIST_FAILURE on failure
  if(!queue||(INVALID_LISTTYPE(new)))
	return LIST_FAILURE;
  listNode_t *hold;
#ifdef NO_DUPLICATES
  for(hold=queue->head;hold;hold=hold->next)
	if(LISTTYPE_EQUAL(new,hold->data))
#ifdef DUPLICATE_RETURN_SUCCESS
		return LIST_SUCCESS;
#else
		return LIST_FAILURE;
#endif
#endif
  hold=(listNode_t*)malloc(sizeof(listNode_t));
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
	  exit(EXIT_FAILURE);
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
#ifdef THREAD_SAFE
  sem_destroy(&queue->turn);
#endif
  free(queue);
  return LIST_SUCCESS;
}

int queueRemoveAll(queue_t* queue,LISTTYPE in){
  int out=0;
#ifdef THREAD_SAFE
  if(!queue||(INVALID_LISTTYPE(in)))
#else
  if(!queue||!queue->head||(INVALID_LISTTYPE(in)))
#endif
	return out;
#ifdef THREAD_SAFE
  sem_wait(&queue->turn);
  if(!queue->head){
	sem_post(&queue->turn);
	return out;
  }
#endif
  listNode_t *pt;
  while(queue->head&&(LISTTYPE_EQUAL(queue->head->data,in))){
	pt=queue->head;
	queue->head=pt->next;
	free(pt);
	++out;
  }
  if(!queue->head){
	queue->tail=NULL;
#ifdef THREAD_SAFE
	sem_post(&queue->turn);
#endif
	return out;
  }
  listNode_t *hold=queue->head;
  pt=hold->next;
  while(pt){
	if((LISTTYPE_EQUAL(pt->data,in))){
		hold->next=pt->next;
		free(pt);
		++out;
	}
	hold=hold->next;
	if(hold)
		pt=hold->next;
	else
		pt=NULL;
  }
#ifdef THREAD_SAFE
  sem_post(&queue->turn);
#endif
  return out;
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
  listNode_t *hold;
#ifdef NO_DUPLICATES
  for(hold=stack->top;hold;hold=hold->next)
	if(LISTTYPE_EQUAL(new,hold->data))
#ifdef DUPLICATE_RETURN_SUCCESS
		return LIST_SUCCESS;
#else
		return LIST_FAILURE;
#endif
#endif
  hold=(listNode_t*)malloc(sizeof(listNode_t));
  if(!hold)
	return LIST_FAILURE;
  hold->data=new;
#ifdef THREAD_SAFE
  sem_wait(&stack->turn);
#endif
  hold->next=stack->top;
  stack->top=hold;
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
	  exit(EXIT_FAILURE);
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
#ifdef THREAD_SAFE
  sem_destroy(&stack->turn);
#endif
  free(stack);
  return LIST_SUCCESS;
}

int stackRemoveAll(stack_t* stack,LISTTYPE in){
  int out=0;
#ifdef THREAD_SAFE
  if(!stack||(INVALID_LISTTYPE(in)))
#else
  if(!stack||!stack->top||(INVALID_LISTTYPE(in)))
#endif
	return out;
#ifdef THREAD_SAFE
  sem_wait(&stack->turn);
  if(!stack->top){
	sem_post(&stack->turn);
	return out;
  }
#endif
  listNode_t *pt;
  while(stack->top&&(LISTTYPE_EQUAL(stack->top->data,in))){
	pt=stack->top;
	stack->top=pt->next;
	free(pt);
	++out;
  }
  if(!stack->top){
#ifdef THREAD_SAFE
	sem_post(&stack->turn);
#endif
	return out;
  }
  listNode_t *hold=stack->top;
  pt=hold->next;
  while(pt){
	if((LISTTYPE_EQUAL(pt->data,in))){
		hold->next=pt->next;
		free(pt);
		++out;
	}
	hold=hold->next;
	if(hold)
		pt=hold->next;
	else
		pt=NULL;
  }
#ifdef THREAD_SAFE
  sem_post(&stack->turn);
#endif
  return out;
}

#undef LISTTYPE
#undef INVALID_LISTTYPE
