Willis A. Hershey

This file defines two generic list structures, queue_t and stack_t, which are customizable to hold any data type, and are configured at compiletime (hence no list.c).

queue_t is a first-in-first-out list structure. Create a new empty queue with newQueue(), add to it with queueAdd(queue_t*,LISTTYPE), and remove from it with queueRemove(queue_t*,LISTTYPE*). Free all memory associated with the queue with freeQueue(queue_t*).

stack_t is a first-in-last-out list structure. Create a new empty stack with newStack(), push onto it with stackPush(stack_t*,LISTTYPE), and pop from it with stackPop(stack_t*,LISTTYPE*). Free all memory associated with the stack with freeStack(stack_t*).

To use these list structures, define the following macros however your needs dictate before this file is included in some other file (This file will not compile on its own). The only macro you are required to define is LISTTYPE, unless your LISTTYPE is a struct, in which case you must also define LISTTYPE_EQUALS (There are more details on this below). After this file is included, LISTTYPE, LIST_SUCCESS, LIST_FAILURE, INVALID_LISTTYPE(), and LISTTYPE_EQUAL() will all be defined for the rest of your file for your use. If you define THREAD_SAFE it will also remain defined after the #include.

Define LISTTYPE as the type that will be stored in the list, i.e. int, double, some sort of struct, some sort of pointer etc. If you do not define this macro, the code will (almost certainly) not compile.

Optionally, define LIST_SUCCESS and LIST_FAILURE as any int values of your choice if you wish to customize the return values of queueAdd, stackPush, queueRemove, stackPop, freeQueue, and freeStack. If you do not manually define these, they default to 1 and 0 respectively.

Optionally, define INVALID_LISTTYPE(c) as a boolean expression involving LISTTPYE c. If this expression evaluates to true, queueAdd or stackPush will leave the list untouched, and immediately return LIST_FAILURE. For instance, if LISTTYPE is int, and all values should be positive, perhaps INVALID_LISTTYPE(c) should be defined to c<0. If LISTTYPE is a pointer, perhaps INVALID_LISTTYPE(c) should be defined to c==NULL (or !c). If you do not define this macro, the code will accept all values of LISTTYPE as valid, and will not check its value before adding it to the list. Please do not define INVALID_LISTTYPE(c) as any nonzero constant, as it will make queueAdd and stackPush return LIST_FAILURE every time, and make this entire file essentially useless.

LISTTYPE_EQUAL(a,b) defaults to a==b, but if you wish to define some other sort of equivalence for your LISTTYPE, you may define this macro as any other boolean expression. This macro is used in queueRemoveValue and stackRemoveValue to check LISTTYPES in the list structures against an input LISTTYPE. If your LISTTYPE is some sort of struct, direct equivalence is most likely not supported by your compiler, so you must either create your own boolean expression for equivalence, or define this macro as some constant, and accept that queueRemoveValue and stackRemoveValue will simply not work as intended (there is an example of manually-defined struct equivalence below). Alternatively you may want to define this macro even if direct equivalence is supported, for example, if your LISTTYPE is some sort of pointer, and you wish the code to consider two pointers to data on the same page as equivalent, then perhaps you would define LISTTYPE_EQUAL(a,b) as a/PAGESIZE==b/PAGESIZE.

Define THREAD_SAFE if you intend to give multiple threads access to the same list structure, and mutex will be automatically implemented via the semaphore.h library. If you do not define THREAD_SAFE and two or more threads attempt operations on some list structure at the same time, undefined behavior may occur. freeQueue and freeStack remain thread unsafe even in THREAD_SAFE mode. It is up to the programmer to insure that all but one thread are finished with the list structure before these functions are called. If THREAD_SAFE is defined, the code will likely only compile if you ask your compiler to link the pthread libraries.

This file does not use a preprocessor trap to prevent this file to be included multiple times. This was done so that the coder may create list structures to contain multiple different LISTTYPES. Doing this requires a painful amount of #defines and #undefs, but it is possible. I checked. I intended to make an example of this, but I got tired of it and stopped. If you really want to do it and can't figure out how send me an email or something. 

Here is a list of functions included in the file with short descriptions. The two structures are alike in dignity, so for every function in queue_t, there is an equivalent function for stack_t with essentially the same behavior, so the descriptions are combined.

queue_t* newQueue()/stack_t* newstack() returns new empty queue/stack, or returns NULL on failure.

int queueAdd(queue_t*,LISTTYPE)/int stackPush(stack_t*,LISTTYPE) adds LISTTYPE from input to the end/top of a valid queue/stack, returns LIST_SUCCESS on success, and LIST_FAILURE on failure.

int queueRemove(queue_t*,LISTTYPE*)/int stackPop(stack_t*,LISTTYPE*) removes from the front/top of a valid queue/stack, and saves the value it removed at address of the LISTTYPE pointer passed to it. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure. If LIST_FAILURE is returned, then the queue/stack as well as the data pointed to by the LISTTYPE* will remain unchanged.

int queueRemoveValue(queue_t*,LISTTYPE)/int stackRemoveValue(stack_t*,LISTTYPE) removes the first instance of some input LISTTYPE from the queue/stack. Returns LIST_SUCCESS if the value is found and removed, and LIST_FAILURE if the value is not present, or some other error occurs. If LIST_FAILURE is returned, the queue/stack will remain unchanged. This function makes use of the LISTTYPE_EQUAL(a,b) macro, which can be redefined.

int freeQueue(queue_t*)/int freeStack(stack_t*) frees all memory associated with the queue/stack, and returns LIST_SUCCESS on success, and LIST_FAILURE on failure. If you attempt to perform operations on a queue/stack after this function has been called, or while this function is being called, undefined behavior may occur. This function is not thread safe even in THREAD_SAFE mode, so it is up to the programmer to ensure that all other threads have finished using the queue/stack before this function is called.

Example usage:

#define LISTTYPE int
#include "list.h"
queue_t *queue=newQueue();
queueAdd(queue,3);
int a;
queueRemove(queue,&a);
freeQueue(queue);
etc...
___________________________________________

typedef struct bogus{
  int i;
  float f;
  void *p;
}bogus_t;

#define LISTTYPE bogus_t
#define LISTTYPE_EQUALS(a,b) a.i==b.i&&a.f==b.f&&a.p==b.p
#include "list.h"
stack_t *stack=newStack();
bogus_t a;
bogus_t b=(bogus_t){7,9.3,NULL};
stackPush(stack,b);
stackPop(stack,&a);
freeStack(stack);
etc...
___________________________________________

#define LISTTYPE double
#define LIST_SUCCESS 0
#define LIST_FAILURE -1
#define THREAD_SAFE
#include "list.h"
etc...
___________________________________________
