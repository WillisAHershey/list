Willis A. Hershey

This file defines two generic list structures, queue_t and stack_t, which are customizable to hold any data type, and are configured at compiletime (hence no list.c).

queue_t is a first-in-first-out list structure. Create a new empty queue with newQueue(), add to it with queueAdd(queue_t*,LISTTYPE), and remove from it with queueRemove(queue_t*). You may check to see if the queue has any members with queueNotEmpty(queue_t*). Free all memory associated with the queue with freeQueue(queue_t*), but keep in mind that any data in the queue will be lost.

stack_t is a first-in-last-out list structure. Create a new empty stack with newStack(), push onto it with stackPush(stack_t*,LISTTYPE), and pop from it with stackPop(stack_t*). You may check to see if the stack has any members with stackNotEmpty(stack_t*). Free all memory associated with the stack with freeStack(stack_t*), but keep in mind that any data in the stack will be lost.

To use these list structures, define the following macros however your needs dictate before this file is included in some other file. (This file will not compile on its own)

Define LISTTYPE as the type that will be stored in the list, i.e. int, double, some sort of pointer etc. If you do not define this macro, the code will (most likely) not compile.

Define LISTTYPE_FAILURE as a value of LISTTYPE that should be returned from queueRemove and stackPop if an error occurs. For instance if LISTTYPE is int, perhaps 0 or -1 should indicate error. If LISTTYPE is a pointer of some sort, perhaps NULL should be returned on error. If LISTTYPE_FAILURE is not defined, the code will compile in EXIT_ON_ERROR mode, which means if an error occurs (i.e. malloc failure, bad pointer, empty list), the program will exit. Keep in mind that in EXIT_ON_ERROR mode, if you try to remove from an empty queue or pop from an empty stack, your program will exit. You can prevent this by checking to make sure the list still has a member with either queueNotEmpty() or stackNotEmpty(), however this work-around does not do well in THREAD_SAFE mode (described below).

Define LIST_SUCCESS and LIST_FAILURE to any int values of your choice if you wish to customize the return values of queueAdd, stackPush, freeQueue and freeStack. If you do not manually define these, they default to 1 and 0 respectively.

Define INVALID_LISTTYPE(c) as a boolean expression involving LISTTPYE c. If this boolean evaluates to true, queueAdd or stackPush will leave the list untouched, and immediately return LIST_FAILURE. For instance, if LISTTYPE is int, and all values should be positive, perhaps INVALID_LISTTYPE(c) should be defined to c<0. If LISTTYPE is a pointer, perhaps INVALID_LISTTYPE(c) should be defined to c==NULL (or !c). If you do not define this macro, the code will accept all values of LISTTYPE as valid, and will not check its value before adding it to the list. Please do not define INVALID_LISTTYPE(c) as any nonzero constant, as it will make queueAdd and stackPush return LIST_FAILURE every time, and make this entire file essentially useless.

Define NO_DUPLICATES if you wish queueAdd and stackPush to check to see if the input LISTTYPE is already present in the list, and decline to add it a second time. Defining this macro will increase the time complexity of queueAdd and stackPush from constant to O(n) time. Default return value when a duplicate is found (in the NO_DUPLICATES case) is LIST_FAILURE, but this can be overriden to LIST_SUCCESS by defining DUPLICATE_RETURN_SUCCESS.

LISTTYPE_EQUAL(a,b) defaults to a==b, but if you wish to define some other sort of equivalence for your LISTTYPE, you may redefine this macro to any other boolean expression. This macro is used in removeAllFromQueue, removeAllFromStack, and in queueAdd and stackPush if NO_DUPLICATES is defined.

Define THREAD_SAFE if you intend to give multiple threads access to the same list structure, and mutex will be automatically implemented via the semaphore.h library. If you do not define THREAD_SAFE and two or more threads attempt operations on some list structure at the same time, undefined behavior may occur. freeQueue, freeStack, queueNotEmpty and stackNotEmpty remain thread unsafe even in THREAD_SAFE mode. It is up to the programmer to insure that all but one thread are finished with the list structure before these functions are called.

The file is rather difficult to read because of all of the preprocessor nonsense, so here is a list of functions included in the file with short descriptions. The two structures are alike in dignity, so for every function in queue_t, there is an equivalent function for stack_t with essentially the same behavior, so the descriptions are combined.

queue_t* newQueue()/stack_t* newstack() returns new empty queue/stack, or returns NULL on malloc failure.

int queueAdd(queue_t*,LISTTYPE)/int stackPush(stack_t*,LISTTYPE) adds LISTTYPE from input to the end/top of a valid queue/stack, returns LIST_SUCCESS on success, and LIST_FAILURE on failure.

LISTTYPE queueRemove(queue_t*)/LISTTYPE stackPop(stack_t*) removes from the front/top of a valid queue/stack, and returns the value it removed on success. On failure it returns LISTTYPE_ERROR, or exits if LISTTYPE_ERROR is not defined. 

int queueNotEmpty(queue_t*)/int stackNotEmpty(stack_t*) returns 1 if there is a member in the queue/stack, and returns 0 if there is not. This function exists in THREAD_SAFE mode, but is not thread safe, as it loses its meaning in multithreading models. If multiple threads are accessing a list at the same time, and you check to make sure it has an element, there is no guarantee that the answer will remain true for any length of time. If you wish to use this function in THREAD_SAFE mode, you must ensure that all but one thread are finished using the list before calling this function.

int freeQueue(queue_t*)/int freeStack(stack_t*) frees all memory associated with the queue/stack, and returns LIST_SUCCESS on success, and LIST_FAILURE if you pass it a null pointer. If you attempt to perform operations on a queue/stack after this function has been called, or while this function is being called, undefined behavior will occur.

int removeAllFromQueue(queue_t*,LISTTYPE)/int removeAllFromStack(stack_t*,LISTTYPE) goes through the queue/stack and removes any value from it if LISTTYPE_EQUAL(a,b) evaluates to true. It returns the number of elements it removed. This function will not check to make sure this output does not overflow in the case that we remove more than MAX_INT values.

