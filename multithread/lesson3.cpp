/*
#include <pthread.h>
int pthread_create(
    pthread_t *thread, 
    const pthread_attr_t *attr, 
    void *(*start_routine) (void *), 
    void *arg);


#include "processthreadsapi.h"
HANDLE CreateThread(
  [in, optional]  LPSECURITY_ATTRIBUTES   lpThreadAttributes,
  [in]            SIZE_T                  dwStackSize,
  [in]            LPTHREAD_START_ROUTINE  lpStartAddress,
  [in, optional]  __drv_aliasesMem LPVOID lpParameter,
  [in]            DWORD                   dwCreationFlags,
  [out, optional] LPDWORD                 lpThreadId
);

A thread in an executable that calls the C run-time library (CRT) 
should use the _beginthreadex and _endthreadex functions for thread management 
rather than CreateThread and ExitThread; this requires the use of 
the multithreaded version of the CRT.

uintptr_t _beginthread( // NATIVE CODE
   void( __cdecl *start_address )( void * ),
   unsigned stack_size,
   void *arglist
);

void _endthreadex(
   unsigned retval
);

*/

#include <iostream>
#include <thread>

void SayHello() {
    std::cout << "Hello, world\n";
}

int main() {
    std::thread t{SayHello};
    t.join();
    // t.detach();
    std::jthread t{SayHello};
}

class MoveOnlyTask {
public:
    explicit MoveOnlyTask(std::unique_ptr<int> data): m_data(std::move(data))
    {}
    void operator()() {
        std::cout << "Hello from MoveOnlyTask with data: " << *m_data << std::endl;
    }
private:
    std::unique_ptr<int> m_data;
};

void TestMoveOnlyTask() {
    MoveOnlyTask task{ std::make_unique<int>(42) };
    std::jthread t{ std::move(task)};

    std::jthread t{[]{
        std::cout << "Hello from lambda\n";
        std::cout << "std::this_thread::get_id(): " << std::this_thread::get_id() << std::endl;
    }};
}
