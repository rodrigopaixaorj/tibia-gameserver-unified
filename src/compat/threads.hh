#ifndef TIBIA_THREADS_HH_
#define TIBIA_THREADS_HH_ 1

#include "compat.hh"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

typedef std::thread* ThreadHandle;
typedef int (ThreadFunction)(void *);

constexpr ThreadHandle INVALID_THREAD_HANDLE = nullptr;

ThreadHandle StartThread(ThreadFunction *Function, void *Argument, bool Detach = true);
ThreadHandle StartThread(ThreadFunction *Function, void *Argument, size_t StackSize, bool Detach = true);
int JoinThread(ThreadHandle Handle);
void DelayThread(int Seconds, int MicroSeconds);

struct Semaphore {
	Semaphore(int Value = 1);
	~Semaphore(void);
	void up(void);
	void down(void);
	bool try_down(void);

	int value;
	std::mutex mutex;
	std::condition_variable condition;
};

// Atomic Helpers
typedef std::atomic<int> AtomicInt;

inline int AtomicLoad(const AtomicInt *Atom){
	return Atom->load(std::memory_order_seq_cst);
}

inline void AtomicStore(AtomicInt *Atom, int Val){
	Atom->store(Val, std::memory_order_seq_cst);
}

inline int AtomicFetchAdd(AtomicInt *Atom, int Val){
	return Atom->fetch_add(Val, std::memory_order_seq_cst);
}

inline bool AtomicCompareExchange(AtomicInt *Atom, int *Expected, int Desired){
	return Atom->compare_exchange_strong(*Expected, Desired, std::memory_order_seq_cst);
}

#endif // TIBIA_THREADS_HH_
