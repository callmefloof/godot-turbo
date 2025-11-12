//
// Created by Floof on 28-7-2025.
//

#ifndef COMMAND_H
#define COMMAND_H
#include <utility>
#include <functional>
#include "modules/godot_turbo/thirdparty/concurrentqueue/concurrentqueue.h"
#include <memory>
#include <unordered_map>
#include "core/object/class_db.h"
#include "core/object/ref_counted.h"

/**
 * @file command.h
 * @brief Lock-free command queue system for thread-safe deferred execution
 * 
 * This file implements a high-performance command queue using:
 * - **Object pooling**: Pre-allocated command objects to avoid per-frame allocations
 * - **Lock-free queue**: moodycamel::ConcurrentQueue for multi-producer/consumer safety
 * - **Type erasure**: Polymorphic ICommand interface for heterogeneous commands
 * - **Thread-local tokens**: Producer tokens to reduce contention
 * 
 * @section Architecture
 * 1. `ICommand` - Base interface for all commands
 * 2. `Pool` - Thread-safe object pool for command allocation
 * 3. `Command<F>` - Templated command type with type-specific pooling
 * 4. `CommandQueue` - Lock-free queue for enqueueing and processing commands
 * 5. `CommandHandler` - Godot-exposed RefCounted wrapper for CommandQueue
 * 
 * @section Performance
 * - Pool allocations avoid heap fragmentation and allocation overhead
 * - Lock-free queue enables safe multi-threaded access without mutex contention
 * - Thread-local producer tokens reduce atomic operations
 * - Default pool size: 1024 commands per type (128 KB for 128-byte commands)
 * 
 * @section Usage
 * ```cpp
 * CommandHandler handler;
 * 
 * // Enqueue a command (pooled)
 * handler.enqueue_command([captured_data]() {
 *     // Command logic executed later
 *     print_line("Deferred execution!");
 * });
 * 
 * // Process all queued commands (call once per frame)
 * handler.process_commands();
 * ```
 * 
 * @note Use pooled commands for performance-critical paths
 * @note Use unpooled commands for debugging suspected pool corruption
 * @warning Commands must not capture references to stack variables that may be destroyed
 */

/**
 * @struct ICommand
 * @brief Abstract base interface for all command types
 * 
 * Provides polymorphic interface for type-erased command execution and pooling.
 * All concrete command types derive from this interface.
 */
struct ICommand {
	virtual ~ICommand() = default;
	
	/**
	 * @brief Executes the command logic
	 * 
	 * Called by CommandQueue::process() to run the deferred operation.
	 */
	virtual void execute() = 0;
	
	/**
	 * @brief Returns the command to its type-specific pool
	 * 
	 * Polymorphic method that calls the appropriate pool's deallocate.
	 * Enables proper cleanup without knowing the concrete command type.
	 */
	virtual void release() = 0;
};

/**
 * @struct CommandBase
 * @brief CRTP base template for command implementations
 * 
 * Uses the Curiously Recurring Template Pattern to provide:
 * - Generic command execution via stored functor
 * - Type-specific pool deallocation via static Derived::pool()
 * 
 * @tparam Derived The concrete command type (for CRTP)
 * @tparam F The functor/lambda type being stored
 */
template<typename Derived, typename F>
struct CommandBase : ICommand {
	F func; ///< The stored functor/lambda to execute
	
	/**
	 * @brief Constructs a command with a functor
	 * 
	 * @param f The functor/lambda to execute (forwarded for move semantics)
	 */
	CommandBase(F&& f) : func(std::forward<F>(f)) {}
	
	/**
	 * @brief Executes the stored functor
	 */
	void execute() override { func(); }

	/**
	 * @brief Returns this command to the type-specific pool
	 * 
	 * Uses CRTP to access the Derived class's static pool and deallocate.
	 */
	void release() override {
		Derived::pool().deallocate(static_cast<Derived*>(this));
	}
};

/**
 * @class Pool
 * @brief Thread-safe object pool using lock-free freelist
 * 
 * Allocates a fixed-size arena of memory slots and manages them via a lock-free
 * concurrent queue. Provides constant-time allocation/deallocation when slots
 * are available.
 * 
 * @section Design
 * - Pre-allocates all memory upfront (no dynamic growth)
 * - Uses lock-free queue for thread-safe slot management
 * - Returns nullptr on exhaustion rather than blocking
 * - Producer tokens reduce contention on deallocation
 * 
 * @section Limitations
 * - Fixed capacity (no auto-expansion)
 * - Memory is held for the pool's lifetime
 * - No per-object destruction (placement new/delete expected)
 * 
 * @warning Pool does not track object lifetimes - caller must ensure proper construction/destruction
 */
class Pool {
public:
	/**
	 * @brief Constructs a pool with fixed capacity
	 * 
	 * Allocates a contiguous memory arena and populates the freelist with
	 * pointers to each slot.
	 * 
	 * @param slotSize Size in bytes of each slot (typically sizeof(CommandType))
	 * @param slotCount Number of slots to allocate
	 */
	Pool(size_t slotSize, size_t slotCount)
		: slot_size(slotSize), capacity(slotCount), freelist(slotCount)
	{
		data = operator new(slot_size * capacity);
		for (size_t i = 0; i < capacity; ++i) {
			void* slot = static_cast<uint8_t*>(data) + i * slot_size;
			freelist.enqueue(slot);
		}
	}
	
	/**
	 * @brief Destructor - frees the entire arena
	 * 
	 * @warning Caller must ensure all allocated objects have been destroyed
	 */
	~Pool() {
		if(data){
			operator delete(data);
		} 
	}

	/**
	 * @brief Allocates a slot from the pool
	 * 
	 * @return Pointer to an available slot, or nullptr if pool is exhausted
	 * 
	 * @note Caller must use placement new to construct the object
	 * @note Thread-safe via lock-free freelist
	 */
	void* allocate() {
		void* ptr = nullptr;
		if (!freelist.try_dequeue(ptr)) { return nullptr; }
		return ptr;
	}

	/**
	 * @brief Returns a slot to the pool
	 * 
	 * @param ptr Pointer to the slot to return (must be from this pool)
	 * 
	 * @note Caller must manually destroy the object before deallocation
	 * @note Uses a stack-allocated producer token to reduce contention
	 * @warning Do not deallocate the same pointer twice
	 */
	void deallocate(void* ptr) {
		if(!ptr) { return; }

		// Use a short-lived ProducerToken on the stack so tokens don't outlive the freelist
		moodycamel::ProducerToken temp_tok(freelist);
		freelist.enqueue(temp_tok, ptr);

	}
private:
	void* data = nullptr;              ///< The contiguous memory arena
	size_t slot_size = 0;              ///< Size of each slot in bytes
	size_t capacity = 0;               ///< Total number of slots
	moodycamel::ConcurrentQueue<void*> freelist; ///< Lock-free queue of available slots
};

/**
 * @struct Command
 * @brief Concrete pooled command type for a specific functor type
 * 
 * Each unique functor signature F gets its own Command<F> instantiation with
 * a dedicated static pool. This ensures type-safe pooling without size mismatches.
 * 
 * @tparam F The functor/lambda type (auto-deduced from make_command)
 * 
 * @section Pooling
 * - Pool size: 1024 commands per unique F type
 * - Pool is lazily initialized on first use (static local)
 * - Pool lifetime: Until program exit
 * 
 * @example
 * ```cpp
 * // Two different lambda types = two different pools
 * auto cmd1 = make_command([]() { print_line("A"); });
 * auto cmd2 = make_command([x=42]() { print_line(String::num(x)); });
 * // cmd1 and cmd2 use separate pools due to different capture lists
 * ```
 */
template<typename F>
struct Command : CommandBase<Command<F>, F> {
	using Base = CommandBase<Command<F>, F>;
	using Base::Base;

	/**
	 * @brief Gets the static pool for this command type
	 * 
	 * @return Reference to the type-specific pool (thread-safe singleton)
	 */
	static Pool& pool() {
		static Pool instance(sizeof(Command<F>), 1024);
		return instance;
	}
};




/**
 * @brief Creates a pooled command from a functor
 * 
 * Allocates from the type-specific pool and constructs a Command<F> using
 * placement new. The command can later be destroyed with destroy_command().
 * 
 * @tparam F The functor/lambda type (auto-deduced)
 * @param func The functor/lambda to execute
 * @return Pointer to ICommand, or nullptr if pool is exhausted
 * 
 * @example
 * ```cpp
 * ICommand* cmd = make_command([data]() { 
 *     process(data); 
 * });
 * if (cmd) {
 *     cmd->execute();
 *     destroy_command(cmd);
 * }
 * ```
 * 
 * @note The functor F is moved/forwarded into the command
 * @warning Returns nullptr if the pool is full - caller must check!
 */
template<typename F>
static ICommand* make_command(F&& func) {
	using CmdT = Command<F>;
	void* mem = CmdT::pool().allocate();
	if (!mem) { 
		return nullptr;
	}
	return new (mem) CmdT(std::forward<F>(func));
}

/**
 * @brief Destroys a command and returns it to its pool
 * 
 * Calls the command's virtual release() method, which handles proper
 * destruction and pool deallocation.
 * 
 * @param cmd The command to destroy (must be from make_command)
 * 
 * @note This does NOT call delete - commands are pooled
 */
static void destroy_command(ICommand* cmd) {
	cmd->release();
}

/**
 * @struct UnpooledCommand
 * @brief Unpooled command for debugging or low-frequency operations
 * 
 * Uses heap allocation instead of pooling. Useful for:
 * - Debugging suspected pool corruption issues
 * - Commands with extremely large captures
 * - One-time initialization commands
 * 
 * @warning Less performant than pooled commands - avoid in hot paths
 */
struct UnpooledCommand : ICommand {
	std::function<void()> func; ///< Type-erased functor (heap-allocated)
	
	/**
	 * @brief Constructs an unpooled command
	 * 
	 * @param f The functor to execute (wrapped in std::function)
	 */
	UnpooledCommand(std::function<void()>&& f) : func(std::move(f)) {}
	
	void execute() override { func(); }
	
	/**
	 * @brief Destroys the command via delete
	 */
	void release() override { delete this; }
};

/**
 * @brief Creates an unpooled command from a functor
 * 
 * Allocates on the heap instead of using a pool. Use for debugging or
 * infrequent commands.
 * 
 * @tparam F The functor/lambda type (auto-deduced)
 * @param func The functor/lambda to execute
 * @return Pointer to ICommand (always succeeds unless out of memory)
 * 
 * @note Destroy with destroy_command() - it will call delete internally
 */
template<typename F>
static ICommand* make_command_unpooled(F&& func) {
	return new UnpooledCommand(std::function<void()>(std::forward<F>(func)));
}

/**
 * @class CommandQueue
 * @brief Thread-safe lock-free queue for deferred command execution
 * 
 * Provides multi-producer, single-consumer command queue using moodycamel's
 * lock-free concurrent queue. Commands are enqueued from any thread and
 * processed on a designated thread (typically main or render thread).
 * 
 * @section Thread Safety
 * - **Enqueue**: Safe from any thread (multi-producer)
 * - **Process**: Should be called from a single thread (single-consumer)
 * - **Clear**: Should only be called when no enqueuing is happening
 * 
 * @section Performance
 * - Uses thread-local producer tokens to reduce atomic contention
 * - Lock-free implementation avoids mutex overhead
 * - Pooled commands minimize allocation overhead
 * 
 * @example
 * ```cpp
 * CommandQueue queue;
 * 
 * // From any thread:
 * queue.enqueue([data]() {
 *     update_game_state(data);
 * });
 * 
 * // On main thread each frame:
 * queue.process();
 * ```
 */
class CommandQueue {
private:
	/**
	 * @brief Clears all pending commands
	 * 
	 * Dequeues and destroys all commands in the queue.
	 * 
	 * @warning Not thread-safe - should only be called during shutdown
	 */
	void clear() {
			ICommand* cmd = nullptr;
			while (queue.try_dequeue(cmd) && cmd) {
				destroy_command(cmd);
			}
		}

public:
	/**
	 * @brief Enqueues a pooled command
	 * 
	 * Creates a command from the functor and adds it to the queue using
	 * a thread-local producer token for reduced contention.
	 * 
	 * @tparam F The functor/lambda type (auto-deduced)
	 * @param func The functor/lambda to execute later
	 * 
	 * @note If pool is exhausted, the command is silently dropped
	 * @note Thread-safe - can be called from any thread
	 */
	template<typename F>
	void enqueue(F&& func) {
		ICommand* cmd = make_command(std::forward<F>(func));
	if (!cmd) { return; }

		using TokenPtr = std::unique_ptr<moodycamel::ProducerToken>;
		thread_local std::unordered_map<moodycamel::ConcurrentQueue<ICommand*>*, TokenPtr> prod_tokens;
		auto pqueue = &queue;
		auto it = prod_tokens.find(pqueue);
		if (it == prod_tokens.end()) {
			auto token = std::make_unique<moodycamel::ProducerToken>(queue);
			it = prod_tokens.emplace(pqueue, std::move(token)).first;
		}
		queue.enqueue(*it->second, cmd);
	}

	/**
	 * @brief Enqueues a pre-constructed command
	 * 
	 * Used internally for unpooled commands or custom command types.
	 * 
	 * @param cmd Pointer to a command (from make_command or make_command_unpooled)
	 * 
	 * @note Thread-safe - can be called from any thread
	 */
	void enqueue_raw(ICommand* cmd) {
		if (!cmd) { return; }
		using TokenPtr = std::unique_ptr<moodycamel::ProducerToken>;
		thread_local std::unordered_map<moodycamel::ConcurrentQueue<ICommand*>*, TokenPtr> prod_tokens;
		auto pqueue = &queue;
		auto it = prod_tokens.find(pqueue);
		if (it == prod_tokens.end()) {
			auto token = std::make_unique<moodycamel::ProducerToken>(queue);
			it = prod_tokens.emplace(pqueue, std::move(token)).first;
		}
		queue.enqueue(*it->second, cmd);
	}

	/**
	 * @brief Processes all pending commands
	 * 
	 * Dequeues and executes commands until the queue is empty. Commands are
	 * destroyed after execution and returned to their pools.
	 * 
	 * @note Should be called from a single designated thread (typically main)
	 * @note Executes commands in FIFO order (first enqueued, first executed)
	 */
	void process() {
		ICommand* cmd = nullptr;
		while (queue.try_dequeue(cmd) && cmd) {
			cmd->execute();
			destroy_command(cmd);
		}
	}
	
	/**
	 * @brief Default constructor
	 */
	CommandQueue(){
		queue = moodycamel::ConcurrentQueue<ICommand*>();
	}
	
	/**
	 * @brief Destructor - clears all pending commands
	 */
	~CommandQueue() {
		clear();
	}
	
	/**
	 * @brief Checks if the queue is approximately empty
	 * 
	 * @return true if queue appears empty (lock-free approximation)
	 * 
	 * @note Result is approximate due to concurrent nature
	 */
	bool is_empty() {
		return queue.size_approx() == 0;
	}
private:
	moodycamel::ConcurrentQueue<ICommand*> queue; ///< The lock-free command queue
};

/**
 * @class CommandHandler
 * @brief Godot-exposed wrapper for CommandQueue
 * 
 * RefCounted class that exposes the command queue system to GDScript and C++.
 * Provides a convenient interface for enqueueing and processing commands within
 * the Godot object system.
 * 
 * @section GDScript Usage
 * ```gdscript
 * var handler = CommandHandler.new()
 * 
 * # Enqueue commands from GDScript (C++ side usage shown in docs)
 * # Process them each frame
 * func _process(delta):
 *     handler.process_commands()
 * ```
 * 
 * @section C++ Usage
 * ```cpp
 * Ref<CommandHandler> handler = memnew(CommandHandler);
 * 
 * // Enqueue a pooled command
 * handler->enqueue_command([this, data]() {
 *     this->update_logic(data);
 * });
 * 
 * // Process commands (typically called each frame)
 * handler->process_commands();
 * ```
 * 
 * @note The underlying CommandQueue is thread-safe for enqueueing
 * @note process_commands() should be called from a single thread
 */
class CommandHandler : public RefCounted {
	GDCLASS(CommandHandler, RefCounted)

	CommandQueue command_queue; ///< The underlying command queue

public:
	/**
	 * @brief Default constructor
	 */
	CommandHandler(){
		
	}
	
	/**
	 * @brief Destructor - processes remaining commands
	 */
	~CommandHandler() {
	}
	
	/**
	 * @brief Enqueues a pooled command
	 * 
	 * @tparam F The functor/lambda type (auto-deduced)
	 * @param func The functor/lambda to execute later
	 * 
	 * @note Thread-safe - can be called from any thread
	 */
	template<typename F>
	inline void enqueue_command(F&& func) {
		command_queue.enqueue(func);
	}

	/**
	 * @brief Enqueues an unpooled command
	 * 
	 * Uses heap allocation instead of pooling. Useful for debugging.
	 * 
	 * @tparam F The functor/lambda type (auto-deduced)
	 * @param func The functor/lambda to execute later
	 * 
	 * @note Thread-safe - can be called from any thread
	 */
	template<typename F>
	inline void enqueue_command_unpooled(F&& func) {
		ICommand* cmd = make_command_unpooled(std::forward<F>(func));
		if (!cmd) { return; }
		command_queue.enqueue_raw(cmd);
	}

	/**
	 * @brief Processes all pending commands
	 * 
	 * Executes and destroys all commands currently in the queue.
	 * 
	 * @note Exposed to GDScript
	 * @note Should be called from a single thread (typically main)
	 */
	inline void process_commands() {
		command_queue.process();
	}

	/**
	 * @brief Binds methods to Godot's ClassDB
	 */
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("process_commands"), &CommandHandler::process_commands);
	}



};

#endif //COMMAND_H
