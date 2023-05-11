# Async Workers : Synchronization in Multi-threaded Programs
> This project is a part of ITCS343 Principles of Operating Systems, Faculty of Information and Communication Technology, Mahidol University
  

A simulation of a producer-consumer system implemented in C. The system consists of multiple producers and consumers that interact through a shared buffer. The simulation allows for different actions when the buffer is full: wait, drop, or replace.


## Features
- Multiple producers and consumers: You can specify the number of producers and consumers in the system.
- Random request generation: Producers generate requests/packages at random intervals, simulating real-world scenarios.
- Customizable processing time: You can set the minimum and maximum processing time for the consumers to simulate different workloads.
- Action selection: Choose from three different actions when the buffer is full:
- Wait: Producers wait until space becomes available in the buffer.
- Drop: Producers drop the request if the buffer is full, keeping track of the number of dropped requests.
- Replace: Producers replace the oldest request in the buffer with the new one if it's full.
- Statistics tracking: The simulation calculates and provides statistics such as average waiting time, number of dropped requests, total requests, percentage of dropped requests, and total simulation time.

## Usage
1. Clone the repository and navigate to the project directory.
2. Compile the code using a C compiler.
   ```C
   gcc -Wall -Werror -I ../include -pthread -o async_worker async_worker.c
   ```
3. Run the compiled executable with the following command-line arguments:
   ```C
   ./async_worker <producers> <consumers> <total_requests> <min_processing_time> <max_processing_time> <action>
   ```
- `<producers>`: Number of producers in the system.
- `<consumers>`: Number of consumers in the system.
- `<total_requests>`: Total number of requests to be processed.
- `<min_processing_time>`: Minimum time (in milliseconds) for consumers to process a request.
- `<max_processing_time>`: Maximum time (in milliseconds) for consumers to process a request.
- `<action>`: Action to take when the buffer is full (1: wait, 2: drop, 3: replace).

## Dependencies
The project uses pthread library for thread management and synchronization. Please make sure to link the necessary pthread library while compiling the code.

## Acknowledgments
This project was inspired by the classic producer-consumer problem and aims to provide a practical simulation of the system. Special thanks to the authors of the pthread library for their valuable contributions.
The project also utilizes code examples and concepts from [`ostep-code`](#https://github.com/remzi-arpacidusseau/ostep-code) repository, which provides a collection of code samples and exercises related to the Operating Systems: Three Easy Pieces book.
