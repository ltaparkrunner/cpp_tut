#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>

#include <iostream>

unsigned __stdcall Worker(void* arg) {
    int id = *static_cast<int*>(arg);
    std::cout << "hello from thread " << id << std::endl;
    return 0;
}

int main() {
    int id = 1;
    unsigned int tid = 0;
    HANDLE h = static_cast<HANDLE>(_beginthreadex(/*security*/nullptr, /*StackSize*/0,
        /* threadProc*/ &Worker, /*arg*/&id, /*initfFlag*/0, 
        /*threadIdPtr*/&tid));
    if(!h) {
        std::cerr << "beginthreadex failed" << std::endl;
        return 1;
    }
    WaitForSingleObject(h, INFINITE);
    CloseHandle(h);
}