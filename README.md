   ****Department of Informatics and Telecommunication
University of Athens****

# **Operational Systems (K22) / Winter Semester 2021-2022 Project 1**
## Instructions
Compile:
```console
make
``` 
Run  with:
```console
./main filename numChildren numActions 
```
Compile & Run with:
```console
make run
```
*(modify args from Makefile)  

Start over with:
```console
make clean
```
 *There is a "sample.txt" file, which I used for debugging

## Implementation
### File Handling

 - **main.c** is executed by user. Gets 3 args from command line (`filename, numChildren, numActions`). Creates and initialize memory segment and semaphores. Then creates requested number children, executes children process file: client.c and passes to them from command line 3 args: `execlp("./client", "./client", argv1, argv3, argv4, NULL);`Also executes parent process' file: server.c and passes to it from command line 5 args:
	```c
	argv1 = shm_id
	argv2 = numChildren
	argv3 = numActions
	argv4 = numLines
	argv5 = fname
	```	
	
 - **server.c** is executed by main.c. Gets 5 args from command line. Attaches shared memory segment and opens semaphores created in main.c. Then processes all requests sent by children. Waits for all children to finish executing and then closes and unlinks all semaphores. And finally detach and destroy memory segment
 - **client.c** is executed numChildren times by main.c. Gets 3 args from command line. Attaches shared memory segment and opens semaphores created in main.c. Then each child sends numActions number requests. After all requests have been completed closes all semaphores. And finally detaches memory segment.
 - **common.h** contains header files needed, custom type declaration, and custom functions declaration
 - **common.c** contains custom functions implementation

### Shared Memory
Shared memory is represented by a struct
```c
struct  shared_memory
{
char  foundLine[FSIZE]; // Array containing requested line in each request
int  requestedLine;		// Int containing requested line number in each request
};
```
**Shared Memory's Actions**

 1. Create and Initialize memory segment `shmget` in main.c
 2. Attach memory segment `shmat` in main.c, server.c and client.c
 3. Read / Write in main.c, server.c and client.c
 4. Detach memory segment `shmdt` in server.c and client.c
 5. Destroy memory segment `shmctl` in server.c 

- main.c creates and initializes memory segment. Then attaches it to a struct pointer
- server.c attaches memory segment to a struct pointer. Read / Write to memory segment. And finally, after all children have finished executing detaches and destroys memory segment.
- client.c attaches memory segment to a struct pointer. Read / Write to memory segment. And finally, after a child has finished executing detaches memory segment.

### Semaphores
3 semaphores were used here.

 1. **semClientWrite** is used to control children. It starts **UNLOCKED**. Locks when a random child manages to obtain control of the semaphore and blocks all the other children from entering Critical Section 1. Unlocks when a child has received a response to its request and printed the requested line.
 2. **semServer** is used to control parent. It starts **LOCKED**. Unlocks when a child send a "signal" that a request has been sent. Locks when parent has finished processing the received request and sent back a response.
 3. **semClientRead** is used to make child wait parent's response. It starts **LOCKED**. Unlocks when parent sends a response to child. Locks when child sends a "signal" to parent that there is a request made.

**Semaphores' Actions**

 1. Create semaphores `sem_open` in main.c
 2. Initialize semaphores `sem_init` in main.c
 3. Open semaphores `sem_open` in server.c and client.c
 4. Post semaphores `sem_post` / Wait semaphores `sem_wait` in server.c and client.c
 5. Close semaphores `sem_close` in main.c, server.c and client.c
 6. Unlink semaphores `sem_unlink` in server.c

- main.c creates and initializes all semaphores and then closes them cause we won't be using them here
- server.c opens all semaphores, posts and waits on them. After all childeren have finished executing closes them and unlinks them
- client.c opens all semaphores, posts and waits on them. After a child finishes its execution closes all semaphores

**Error Handling**
As much as possible error handling have been performed. Error messages prompted to help debugging if something goes wrong

**Pseudocode of Critical Sections**
| parent  | child   |
| ------------ | ------------ |
|  wait(semServer) | wat(semClientWrite)  |
|  bla1.1 | bla2.1  |
|  bla1.2 |  bla2.2 |
|  post(semClientRead) | post(semServer)  |
|   | wait(semClientRead)  |
|   |  bla2.3 |
|   |  post(semClientWrite) |

Semaphores' initial values:
	semServer = 0, semClientRead = 0, semClientWrite = 1



----
**Github public repo: [https://github.com/kosmaspoiraz/os-2022-project-1](https://github.com/kosmaspoiraz/os-2022-project-1)**
&copy;[Kosmas Poirazoglou](https://www.linkedin.com/in/kosmas-poirazoglou/)