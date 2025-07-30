//
// Created by Floof on 28-7-2025.
//

#ifndef COMMAND_H
#define COMMAND_H
#include <utility>
#include "../../../thirdparty/concurrentqueue/concurrentqueue.h"


struct ICommand {
	virtual ~ICommand() = default;
	virtual void execute() = 0;
	virtual void release() = 0;  // polymorphic pool return
};

template<typename Derived, typename F>
struct CommandBase : ICommand {
	F func;

	CommandBase(F&& f) : func(std::forward<F>(f)) {}
	void execute() override { func(); }

	void release() override {
		Derived::pool().deallocate(static_cast<Derived*>(this));
	}
};

class Pool {
public:
	Pool(size_t slotSize, size_t slotCount)
		: slot_size(slotSize), capacity(slotCount), freelist(slotCount)
	{
		data = operator new(slot_size * capacity);
		for (size_t i = 0; i < capacity; ++i) {
			void* slot = static_cast<uint8_t*>(data) + i * slot_size;
			freelist.enqueue(slot);
		}
	}
	~Pool() { operator delete(data); }

	void* allocate() {
		void* ptr = nullptr;
		if (!freelist.try_dequeue(ptr)) return nullptr;
		return ptr;
	}

	void deallocate(void* ptr) {
		freelist.enqueue(ptr);
	}
private:
	void* data = nullptr;
	size_t slot_size = 0;
	size_t capacity = 0;
	moodycamel::ConcurrentQueue<void*> freelist;
};

template<typename F>
struct Command : CommandBase<Command<F>, F> {
	using Base = CommandBase<Command<F>, F>;
	using Base::Base;

	// Static pool instance for this exact type
	static Pool& pool() {
		static Pool instance(sizeof(Command<F>), 1024);
		return instance;
	}
};




// Usage:
constexpr size_t COMMAND_SIZE = 128;
static Pool commandArena(COMMAND_SIZE, 1024); // 128 KB

template<typename F>
static ICommand* make_command(F&& func) {
	using CmdT = Command<F>;
	void* mem = CmdT::pool().allocate();
	if (!mem) return nullptr;
	return new (mem) CmdT(std::forward<F>(func));
}

static void destroy_command(ICommand* cmd) {
	cmd->release();
}

class CommandQueue {
public:
	template<typename F>
	void enqueue(F&& func) {
		ICommand* cmd = make_command(std::forward<F>(func));
		queue.enqueue(cmd);
	}

	void process() {
		ICommand* cmd = nullptr;
		while (queue.try_dequeue(cmd)) {
			cmd->execute();
			destroy_command(cmd);
		}
	}
private:
	moodycamel::ConcurrentQueue<ICommand*> queue;
};

#endif //COMMAND_H
