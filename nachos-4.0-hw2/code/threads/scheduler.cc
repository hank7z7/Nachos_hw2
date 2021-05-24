// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------



//<TODO>
// Declare sorting rule of SortedList for L1ReadyQueue & L2ReadyQueue ReadyQueue
// Hint: Funtion Type should be "static int" //why?
int cmp_SJF(Thread *a, Thread *b)
{
    if(a->getRemaningWaitTime() == b->getRemaningWaitTime()) {
        //FCFS
        if(a->getID() > b->getID()) return 1;
        else if(a->getID() < b->getID()) return -1;
        else return 0;
    }
    return a->getRemaningWaitTime() > b->getRemaningWaitTime() ? 1 : -1;
}

// testing smaller id has higher priority
int cmp_ID(Thread *a, Thread *b)
{
    return a->getID() > b->getID() ? 1 : -1;
}

//<TODO>*todo: compare function */

Scheduler::Scheduler(/*SchedulerType type*/)
{
	//schedulerType = type; // enum(where?)
    // readyList = new List<Thread *>; 
    //<TODO>
    /* */
    // Initialize L1ReadyQueue, L2ReadyQueue, L3ReadyQueue ReadyQueue
    // seems that list<> is fine,
    // Sortedlist<> is oriented from list<>
    // compare function
    L1ReadyQueue = new SortedList<Thread *>(cmp_SJF);
    L2ReadyQueue = new SortedList<Thread *>(cmp_ID);
    L3ReadyQueue = new List<Thread *>;
    //<TODO>
	toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    //<TODO>
    // Remove L1ReadyQueue, L2ReadyQueue, L3ReadyQueue ReadyQueue
    delete L1ReadyQueue; // WHY
    delete L2ReadyQueue;
    delete L3ReadyQueue;
    //<TODO>
    // delete readyList; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    // DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
    thread->setStatus(READY);
    Statistics* stats = kernel->stats;
    //<TODO>
    int queue_level = 0;
    // L3ReadyQueue queue
    if(thread->getPriority() < 50) {
        queue_level = 3;
        L3ReadyQueue->Append(thread);
    } 
    // L2ReadyQueue queue
    else if(thread->getPriority() < 100) {
        queue_level = 2;
        L2ReadyQueue->Insert(thread);
    }
    // L1ReadyQueue queue
    // take care of preemptive
    else if (thread->getPriority() < 150) {
        queue_level = 1;
        L1ReadyQueue->Insert(thread);
    }
    DEBUG(dbgMLFQ, "[InsertToQueue] Tick[" << stats->totalTicks << "]: Thread [" << thread->getID() << 
    "] is inserted into queue L[" << queue_level << "]");
    // According to priority of Thread, put them into corresponding ReadyQueue.
    // After inserting Thread into ReadyQueue, don't forget to reset some values.
    // Hint: L1ReadyQueue ReadyQueue is preemptive SRTN(Shortest Remaining Time Next).
    // When putting a new thread into L1ReadyQueue ReadyQueue, you need to check whether preemption or not.
    //<TODO>

    // dufault
    //readyList->Append(thread);
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    /*if (readyList->IsEmpty()) {
    return NULL;
    } else {
        return readyList->RemoveFront();
    }*/

    //<TODO>
    // a.k.a. Find Next (Thread in ReadyQueue) to Run
    if(!L1ReadyQueue->IsEmpty()){
        DEBUG(dbgMLFQ, "[RemoveFromQueue] Tick[" << kernel->stats->totalTicks << "]: Thread [" << L1ReadyQueue->Front()->getID() << "] is removed from queue L[1]");
        return L1ReadyQueue->RemoveFront();
    }
    if(!L2ReadyQueue->IsEmpty()){
        DEBUG(dbgMLFQ, "[RemoveFromQueue] Tick[" << kernel->stats->totalTicks << "]: Thread [" << L2ReadyQueue->Front()->getID() << "] is removed from queue L[2]");
        return L2ReadyQueue->RemoveFront();
    }
    if(!L3ReadyQueue->IsEmpty()){
        DEBUG(dbgMLFQ, "[RemoveFromQueue] Tick[" << kernel->stats->totalTicks << "]: Thread [" << L3ReadyQueue->Front()->getID() << "] is removed from queue L[3]");
        return L3ReadyQueue->RemoveFront();
    }
    return NULL;
    //<TODO>
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
 
//	cout << "Current Thread" <<oldThread->getName() << "    Next Thread"<<nextThread->getName()<<endl;
   
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	     toBeDestroyed = oldThread;
    }
   
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (oldThread->space != NULL) {	// if this thread is a user program,

        oldThread->SaveUserState(); 	// save the user's CPU registers
	    oldThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    // DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".
 
    //cout << "Switching from: " << oldThread->getID() << " to: " << nextThread->getID() << endl;

    // TODO: debug message
    DEBUG(dbgMLFQ, "[ContextSwitch] Tick[" << kernel->stats->totalTicks << "]: Thread [" << nextThread->getID() << "] is now selected for execution, thread[" << oldThread->getID() << "] is replaced, and it has executed " << oldThread->getRunTime() << "ticks");
    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << kernel->currentThread->getID());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
#ifdef USER_PROGRAM
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	    oldThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        DEBUG(dbgThread, "toBeDestroyed->getID(): " << toBeDestroyed->getID());
        delete toBeDestroyed;
	    toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    // readyList->Apply(ThreadPrint);
    L1ReadyQueue->Apply(ThreadPrint);
    L2ReadyQueue->Apply(ThreadPrint);
    L3ReadyQueue->Apply(ThreadPrint);
}

// <TODO>

// Function 1. Function definition of sorting rule of L1ReadyQueue ReadyQueue

// Function 2. Function definition of sorting rule of L2ReadyQueue ReadyQueue

// Function 3. Scheduler::UpdatePriority()
// Hint:
// 1. ListIterator can help.
// 2. Update WaitTime and priority in Aging situations
// 3. After aging, Thread may insert to different ReadyQueue

// 
// Aging machanism

void 
Scheduler::UpdatePriority()
{
    ListIterator<Thread *> *it1 = new ListIterator<Thread *>(L1ReadyQueue);
    ListIterator<Thread *> *it2 = new ListIterator<Thread *>(L2ReadyQueue);
    ListIterator<Thread *> *it3 = new ListIterator<Thread *>(L3ReadyQueue);
    // hard to implement
    // increased by 10 after waiting for more than 400 thicks
    for(; !it1->IsDone(); it1->Next()) {
        it1->Item()->setWaitTime(it1->Item()->getWaitTime()+1);
        if(it1->Item()->getWaitTime() >= 400){
            int oldPriority = it1->Item()->getPriority();
            it1->Item()->setPriority(oldPriority + 10);
            it1->Item()->setWaitTime(0);
            int newPriority = it1->Item()->getPriority();
            //DEBUG(dbgMLFQ, "Putting thread on ready list: " << thread->getName());

        }
    }

    // need some move machanism
    for(; !it2->IsDone(); it2->Next()) {
        it2->Item()->setWaitTime(it2->Item()->getWaitTime()+1);
        if(it2->Item()->getWaitTime() >= 400){
            int oldPriority = it2->Item()->getPriority();
            it2->Item()->setPriority(oldPriority + 10);
            it2->Item()->setWaitTime(0);
            int newPriority = it2->Item()->getPriority();
            //DEBUG(dbgMLFQ, "Putting thread on ready list: " << thread->getName());

        }
    }

    for(; !it3->IsDone(); it3->Next()) {
        it3->Item()->setWaitTime(it3->Item()->getWaitTime()+1);
        if(it3->Item()->getWaitTime() >= 400){
            int oldPriority = it3->Item()->getPriority();
            it3->Item()->setPriority(oldPriority + 10);
            it3->Item()->setWaitTime(0);
            int newPriority = it3->Item()->getPriority();
            //DEBUG(dbgMLFQ, "Putting thread on ready list: " << thread->getName());

        }
    }
}


// <TODO>