#include "kernel/system.h"
#include "kernel/thread.h"
#include "kernel/synch.h"

Lock* lock;
Condition* condition;
int sharedVariable = 0;

void WriterThread(int arg) {
    for (int i = 0; i < 5; ++i) {
        lock->Acquire();
        while (sharedVariable != 0) {
            condition->Wait(lock);
        }
        printf("Writer %d: Writing...\n", arg);
        sharedVariable = arg;
        condition->Broadcast();
        lock->Release();
        currentThread->Yield();
    }
}

void ReaderThread(int arg) {
    for (int i = 0; i < 5; ++i) {
        lock->Acquire();
        while (sharedVariable == 0) {
            condition->Wait(lock);
        }
        printf("Reader %d: Reading sharedVariable = %d\n", arg, sharedVariable);
        sharedVariable = 0;
        condition->Broadcast();
        lock->Release();
        currentThread->Yield();
    }
}

int main() {
    lock = new Lock("Test Lock");
    condition = new Condition("Test Condition");

    Thread *writer1 = new Thread("Writer 1");
    Thread *writer2 = new Thread("Writer 2");
    Thread *reader1 = new Thread("Reader 1");
    Thread *reader2 = new Thread("Reader 2");

    writer1->Fork(WriterThread, 1);
    writer2->Fork(WriterThread, 2);
    reader1->Fork(ReaderThread, 1);
    reader2->Fork(ReaderThread, 2);

    currentThread->Yield(); // Laisser démarrer les threads

    Thread::ThreadFinish();
    return 0;
}
