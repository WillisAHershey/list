Willis A. Hershey

This file defines two generic list structures, queue_t and stack_t, which are customizable to hold any data type, and are configured at compiletime (hence no list.c).

queue_t is a first-in-first-out list structure. Create a new empty queue with newQueue(), add to it with queueAdd(queue_t*,LISTTYPE), and remove from it with queueRemove(queue_t*). Free all memory associated with the queue with freeQueue(queue_t*), but keep in mind that any data in the queue will be lost.

stack_t is a first-in-last-out list structure. Create a new empty stack with newStack(), push onto it with stackPush(stack_t*,LISTTYPE), and pop from it with stackPop(stack_t*). Free all memory associated with the stack with freeStack(stack_t*), but keep in mind that any data in the stack will be lost.

To use these list structures, define the following macros however your needs dictate before this file is included in some other file. (This file will not compile on its own)

Define LISTTYPE as the type that will be stored in the list, i.e. int, double, some sort of pointer etc. If you do not define this macro, the code will (almost certainly) not compile.

Define LIST_SUCCESS and LIST_FAILURE to any int values of your choice if you wish to customize the return values of queueAdd, stackPush, queueRemove, stackPop, freeQueue, and freeStack. If you do not manually define these, they default to 1 and 0 respectively.

Define INVALID_LISTTYPE(c) as a boolean expression involving LISTTPYE c. If this boolean evaluates to true, queueAdd or stackPush will leave the list untouched, and immediately return LIST_FAILURE. For instance, if LISTTYPE is int, and all values should be positive, perhaps INVALID_LISTTYPE(c) should be defined to c<0. If LISTTYPE is a pointer, perhaps INVALID_LISTTYPE(c) should be defined to c==NULL (or !c). If you do not define this macro, the code will accept all values of LISTTYPE as valid, and will not check its value before adding it to the list. Please do not define INVALID_LISTTYPE(c) as any nonzero constant, as it will make queueAdd and stackPush return LIST_FAILURE every time, and make this entire file essentially useless.

LISTTYPE_EQUAL(a,b) defaults to a==b, but if you wish to define some other sort of equivalence for your LISTTYPE, you may redefine this macro to any other boolean expression. This macro is currently not used in the code.

Define THREAD_SAFE if you intend to give multiple threads access to the same list structure, and mutex will be automatically implemented via the semaphore.h library. If you do not define THREAD_SAFE and two or more threads attempt operations on some list structure at the same time, undefined behavior may occur. freeQueue and freeStack remain thread unsafe even in THREAD_SAFE mode. It is up to the programmer to insure that all but one thread are finished with the list structure before these functions are called.

Here is a list of functions included in the file with short descriptions. The two structures are alike in dignity, so for every function in queue_t, there is an equivalent function for stack_t with essentially the same behavior, so the descriptions are combined.

queue_t* newQueue()/stack_t* newstack() returns new empty queue/stack, or returns NULL on failure.

int queueAdd(queue_t*,LISTTYPE)/int stackPush(stack_t*,LISTTYPE) adds LISTTYPE from input to the end/top of a valid queue/stack, returns LIST_SUCCESS on success, and LIST_FAILURE on failure.

int queueRemove(queue_t*,LISTTYPE*)/int stackPop(stack_t*,LISTTYPE*) removes from the front/top of a valid queue/stack, and saves the value it removed at address of the LISTTYPE pointer passed to it. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure. If LIST_FAILURE is returned, then the queue/stack as well as the data pointed to by the LISTTYPE* will remain unchanged.

int freeQueue(queue_t*)/int freeStack(stack_t*) frees all memory associated with the queue/stack, and returns LIST_SUCCESS on success, and LIST_FAILURE on failure. If you attempt to perform operations on a queue/stack after this function has been called, or while this function is being called, undefined behavior may occur. This function is not thread safe even in THREAD_SAFE mode, so it is up to the programmer to ensure that all other threads have finished using the queue/stack before this function is called.

