Willis A. Hershey

This file defines two generic list structures, queue_t and stack_t, which are customizable to hold any data type, and are configured at compiletime (hence no list.h).

This file is not intended to be a library, it's intended to be recompiled every time with macro definitions based on the programmer's needs in some file, so the easiest way to do that is to copy this file into the /usr/include/ folder and include it with the line #include <list.c>

queue_t is a first-in-first-out list structure. Initialize a queue_t with queueInit(queue_t*), add to it with queueAdd(queue_t*,LISTTYPE), and remove from it with queueRemove(queue_t*,LISTTYPE*). Destroy the queue and anything in it with queueDestroy(queue_t*).

stack_t is a first-in-last-out list structure. Initialize a stack_t with stackInit(stack_t*), push onto it with stackPush(stack_t*,LISTTYPE), and pop from it with stackPop(stack_t*,LISTTYPE*). Destroy the stack and anything in it with stackDestroy(stack_t*).

To use these list structures, define the following macros however your needs dictate before this file is included in some other file (This file will not compile on its own). The only macro you are always required to define is LISTTYPE.

Define LISTTYPE as the type that will be stored in the list, i.e. int, double, some sort of struct, some sort of pointer etc. If you do not define this macro, the code will (almost certainly) not compile.

Optionally, define LIST_SUCCESS and LIST_FAILURE as any int values of your choice if you wish to customize the return values of queueInit, stackInit, queueAdd, stackPush, queueRemove, stackPop, queueDestroy, stackDestroy, etc. If you do not manually define these, they default to 1 and 0 respectively. If you do define them, it's probably best if they are not defined to the same value.

Optionally, define INVALID_LISTTYPE(c) as a boolean expression involving some LISTTPYE c. If this expression evaluates to true, queueAdd or stackPush will leave the list untouched, and immediately return LIST_FAILURE. For instance, if LISTTYPE is int, and all values should be positive, perhaps INVALID_LISTTYPE(c) should be defined as c<0. If LISTTYPE is a pointer, perhaps INVALID_LISTTYPE(c) should be defined to c==NULL (or !c). If you do not define this macro, the code will accept all values of LISTTYPE as valid, and will not check its value before adding it to the list. Please do not define INVALID_LISTTYPE(c) as any nonzero constant, as it will make queueAdd and stackPush return LIST_FAILURE every time, and make this entire file essentially useless.

Define THREAD_SAFE if you intend to give multiple threads access to the same list structure, and mutex will be automatically implemented via the semaphore.h library. If you do not define THREAD_SAFE and two or more threads attempt operations on some list structure at the same time, undefined behavior may occur. freeQueue and freeStack remain thread unsafe even in THREAD_SAFE mode. It is up to the programmer to insure that all but one thread are finished with the list structure before these functions are called. A call to queueRemove or stackPop in THREAD_SAFE mode will not block until a LISTTYPE has become available, it will simply return LIST_FAILURE immediately. If THREAD_SAFE is defined, the code will likely only compile if you ask your compiler to link the pthread libraries.

By default, all structures and functions are included, but if you wish to only use one of these structures, you may define INCLUDE_QUEUE or INCLUDE_STACK, to prevent the other from being compiled and taking up room in your executable file. If you define either of these macros, however, the search functions will not be included in the compilation unless you also define INCLUDE_SEARCH_FUNCTIONS.

LISTTYPE_EQUAL is used in the search functions to check equivalence between LISTTYPES. LISTTYPE_EQUAL(a,b) defaults to a==b, but if you wish to define some other sort of equivalence for your LISTTYPE, you may define this macro as any other boolean expression. If your LISTTYPE is some sort of struct, direct equivalence is most likely not supported by your compiler, so you must either create your own boolean expression for equivalence, or define this macro as some constant, and accept that the search functions will simply not work as intended (there is an example of manually-defined struct equivalence below). Alternatively you may want to define this macro even if direct equivalence is supported, for example, if your LISTTYPE is some sort of pointer, and you wish the code to consider two pointers to data on the same page as equivalent, then perhaps you would define LISTTYPE_EQUAL(a,b) as a/PAGESIZE==b/PAGESIZE.

SIZEOF_LISTNODE_T is a macro used to tell the stdlib malloc function how much memory is needed for each node in either list structure. If your LISTTYPE is some sort of datatype with dynamic size, you may redefine this macro to some other expression that returns type size_t. When SIZEOF_LISTNODE_T is invoked in the code, it is passed the LISTTYPE that is given to the queueAdd or stackPush function, so you may take that into account if you wish to define this as some function involving your LISTTYPE value.

Here is a list of functions included in the file with short descriptions. The two structures are alike in dignity, so for every function in queue_t, there is an equivalent function for stack_t with essentially the same behavior, so the descriptions are combined.

int queueInit(queue_t*)/int stackInit(stack_t*) Intitializes new empty queue/stack structure at the address from input. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure. This function does not allocate space for the queue/stack, it assumes the programmer has set aside some space for it in the stack or heap, and simply sets up shop at the address given. If the programmer mallocs space for a queue_t/stack_t it is up to them to free that space after queueDestroy/stackDestroy is called on it.

int queueAdd(queue_t*,LISTTYPE)/int stackPush(stack_t*,LISTTYPE) adds valid LISTTYPE from input to the end/top of a valid queue/stack, returns LIST_SUCCESS on success, and LIST_FAILURE on failure. LISTTYPE values are stored in listNode_t structures, which are malloced using the SIZEOF_LISTNODE_T(c) macro which defaults to sizeof(listNode_t). If your LISTTYPE requires dynamic sizing, this macro can be defined to some expression involving the LISTTYPE passed to this function, such as sizeof(listNode_t)+strlen(c.word)+1, assuming LISTTYPE is some struct with a char* named word. LISTTYPE values are stored in the nodes using the LISTTYPE_ASSIGN(dest,source) macro, which defaults to dest=source, but again this can be defined as another expression such as {dest.a=source.a;srcpy(dest.word,source.word);} however the programmer's needs dictate.

int queueRemove(queue_t*,LISTTYPE*)/int stackPop(stack_t*,LISTTYPE*) removes from the front/top of a valid queue/stack, and saves the value it removed at address of the LISTTYPE pointer passed to it. Returns LIST_SUCCESS on success, and LIST_FAILURE on failure. If LIST_FAILURE is returned, then the queue/stack as well as the data pointed to by the LISTTYPE* will remain unchanged. Passing a NULL LISTTYPE pointer to this function will not be percieved as an error; the remove/pop operation will proceed as usual, the data will simply be lost, and LIST_SUCCESS will be returned. The LISTTYPE value is saved at the address using the LISTTYPE_ASSIGN(dest,source) macro, which defaults to dest=source, but can be defined as any expression involving LISTTYPEs.

int queueDestroy(queue_t*)/int stackDestroy(stack_t*) frees all memory associated with the nodes inside the queue/stack. If this function is invoked on a queue/stack that contains nodes, access to data stored in those nodes will be lost. If you attempt to perform operations on a queue/stack after this function has been called, or while this function is being called, undefined behavior may occur. This function is not thread safe even in THREAD_SAFE mode, so it is up to the programmer to ensure that all other threads have finished using the queue/stack before this function is called. If your LISTTYPE is some sort of pointer to malloced memory, in order to prevent memory leaks you most likely have to queueRemove/stackPop and free the pointers manually until LIST_FAILURE is returned before calling this function. This function does not assume that the pointer passed to it is malloced, so if it is, it is up to the programmer to free it after this function returns.

The following are the search functions, which are included in compilation if neither INCLUDE_QUEUE nor INCLUDE_STACK is defined, or if INCLUDE_SEARCH_FUNCTIONS is defined.

int queueRemoveValue(queue_t*,LISTTYPE)/int stackRemoveValue(stack_t*,LISTTYPE) removes the first instance of some valid LISTTYPE from the queue/stack. Returns LIST_SUCCESS if value is found and removed, and LIST_FAILURE if value is not present or some sort of error occurs. If LIST_FAILURE is returned, then the queue/stack has not been changed. LISTTYPES in the listNodes are compared to the input LISTTYPE value with the LISTTYPE_EQUALS(a,b) macro, which can be defined by the programmer.

int queueRemoveAll(queue_t*,LISTTYPE)/int stackRemoveAll(stack_t*,LISTTYPE) removes all instances of some valid LISTTYPE value from the queue/stack. Returns the number of values removed, or -1 on error. LISTTYPES in the listNodes are compared to the input LISTTYPE value with the LISTTYPE_EQUALS(a,b) macro, which can be user defined by the programmer.

Example usage:

#define LISTTYPE int
#include "list.c"
queue_t queue;
queueInit(&queue);
queueAdd(&queue,3);
int a;
queueRemove(&queue,&a);
queueDestroy(&queue);
etc...
___________________________________________

typedef struct bogus{
  int i;
  float f;
  void *p;
}bogus_t;

#define LISTTYPE bogus_t
#define LISTTYPE_EQUALS(a,b) a.i==b.i&&a.f==b.f&&a.p==b.p
#include "list.c"
stack_t stack;
stackInit(&stack);
bogus_t a;
bogus_t b=(bogus_t){.i=7,.f=9.3,.p=NULL};
stackPush(&stack,b);
stackPop(&stack,&a);
stackDestroy(&stack);
etc...
___________________________________________

#define LISTTYPE double
#define LIST_SUCCESS 0
#define LIST_FAILURE -1
#define THREAD_SAFE
#include "list.c"
etc...
___________________________________________
