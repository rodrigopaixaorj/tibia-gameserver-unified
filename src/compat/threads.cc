#include "threads.hh"

Semaphore::Semaphore(int Value){
	this->value = Value;
}

Semaphore::~Semaphore(void){
}

void Semaphore::up(void){
	std::unique_lock<std::mutex> Lock(this->mutex);
	this->value += 1;
	this->condition.notify_one();
}

void Semaphore::down(void){
	std::unique_lock<std::mutex> Lock(this->mutex);
	while(this->value <= 0){
		this->condition.wait(Lock);
	}
	this->value -= 1;
}

bool Semaphore::try_down(void){
	std::unique_lock<std::mutex> Lock(this->mutex);
	if(this->value > 0){
		this->value -= 1;
		return true;
	}
	return false;
}

ThreadHandle StartThread(ThreadFunction *Function, void *Argument, bool Detach){
	std::thread *T = new std::thread([Function, Argument]() {
		Function(Argument);
	});

	if(Detach){
		T->detach();
		delete T;
		return nullptr;
	}
	return T;
}

ThreadHandle StartThread(ThreadFunction *Function, void *Argument, size_t StackSize, bool Detach){
	(void)StackSize;
	return StartThread(Function, Argument, Detach);
}

int JoinThread(ThreadHandle Handle){
	if(Handle != nullptr && Handle->joinable()){
		Handle->join();
		delete Handle;
		return 0;
	}
	return -1;
}

void DelayThread(int Seconds, int MicroSeconds){
	std::this_thread::sleep_for(std::chrono::seconds(Seconds) + std::chrono::microseconds(MicroSeconds));
}
