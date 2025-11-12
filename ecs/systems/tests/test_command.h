#pragma once

#include "tests/test_macros.h"
#include "modules/godot_turbo/ecs/systems/command.h"
#include "core/object/ref_counted.h"
#include <atomic>
#include <thread>
#include <vector>

namespace TestCommand {

/**
 * @brief Test fixture for command system tests
 */
class CommandTestFixture {
public:
	std::atomic<int> counter{0};
	std::atomic<int> execution_order{0};
	
	void setup() {
		counter = 0;
		execution_order = 0;
	}
	
	void teardown() {
		// Cleanup if needed
	}
};

/**
 * @test ICommand basic interface
 */
TEST_CASE("[Command] ICommand interface exists and is polymorphic") {
	struct TestCommand : ICommand {
		bool executed = false;
		void execute() override { executed = true; }
		void release() override { delete this; }
	};
	
	ICommand* cmd = new TestCommand();
	CHECK(cmd != nullptr);
	
	cmd->execute();
	TestCommand* test_cmd = static_cast<TestCommand*>(cmd);
	CHECK(test_cmd->executed == true);
	
	cmd->release();
}

/**
 * @test Pool allocation and deallocation
 */
TEST_CASE("[Command] Pool allocates and deallocates slots") {
	Pool pool(64, 10); // 10 slots of 64 bytes each
	
	void* slot1 = pool.allocate();
	CHECK(slot1 != nullptr);
	
	void* slot2 = pool.allocate();
	CHECK(slot2 != nullptr);
	CHECK(slot1 != slot2);
	
	pool.deallocate(slot1);
	pool.deallocate(slot2);
}

/**
 * @test Pool exhaustion
 */
TEST_CASE("[Command] Pool returns nullptr when exhausted") {
	Pool pool(64, 2); // Only 2 slots
	
	void* slot1 = pool.allocate();
	void* slot2 = pool.allocate();
	void* slot3 = pool.allocate(); // Should fail
	
	CHECK(slot1 != nullptr);
	CHECK(slot2 != nullptr);
	CHECK(slot3 == nullptr);
	
	// Return one and try again
	pool.deallocate(slot1);
	void* slot4 = pool.allocate();
	CHECK(slot4 != nullptr);
	
	pool.deallocate(slot2);
	pool.deallocate(slot4);
}

/**
 * @test Pool deallocate nullptr is safe
 */
TEST_CASE("[Command] Pool deallocate handles nullptr safely") {
	Pool pool(64, 10);
	
	// Should not crash
	pool.deallocate(nullptr);
}

/**
 * @test Pool slot reuse
 */
TEST_CASE("[Command] Pool reuses deallocated slots") {
	Pool pool(64, 1); // Single slot
	
	void* slot1 = pool.allocate();
	CHECK(slot1 != nullptr);
	
	void* slot2 = pool.allocate(); // Should fail (pool exhausted)
	CHECK(slot2 == nullptr);
	
	pool.deallocate(slot1);
	
	void* slot3 = pool.allocate(); // Should succeed (slot reused)
	CHECK(slot3 != nullptr);
	CHECK(slot3 == slot1); // Same slot reused
	
	pool.deallocate(slot3);
}

/**
 * @test make_command creates pooled command
 */
TEST_CASE("[Command] make_command creates pooled command") {
	int counter = 0;
	
	ICommand* cmd = make_command([&counter]() {
		counter++;
	});
	
	CHECK(cmd != nullptr);
	CHECK(counter == 0);
	
	cmd->execute();
	CHECK(counter == 1);
	
	destroy_command(cmd);
}

/**
 * @test make_command with different lambda types
 */
TEST_CASE("[Command] make_command handles different lambda types") {
	int result1 = 0;
	float result2 = 0.0f;
	
	ICommand* cmd1 = make_command([&result1]() {
		result1 = 42;
	});
	
	ICommand* cmd2 = make_command([&result2]() {
		result2 = 3.14f;
	});
	
	cmd1->execute();
	cmd2->execute();
	
	CHECK(result1 == 42);
	CHECK(result2 == 3.14f);
	
	destroy_command(cmd1);
	destroy_command(cmd2);
}

/**
 * @test make_command with captured values
 */
TEST_CASE("[Command] make_command captures values correctly") {
	int x = 10;
	int y = 20;
	int result = 0;
	
	ICommand* cmd = make_command([x, y, &result]() {
		result = x + y;
	});
	
	x = 999; // Modify original, should not affect captured copy
	
	cmd->execute();
	CHECK(result == 30); // Should use captured values (10 + 20)
	
	destroy_command(cmd);
}

/**
 * @test make_command_unpooled creates unpooled command
 */
TEST_CASE("[Command] make_command_unpooled creates unpooled command") {
	int counter = 0;
	
	ICommand* cmd = make_command_unpooled([&counter]() {
		counter++;
	});
	
	CHECK(cmd != nullptr);
	cmd->execute();
	CHECK(counter == 1);
	
	destroy_command(cmd);
}

/**
 * @test CommandQueue enqueue and process
 */
TEST_CASE("[Command] CommandQueue enqueues and processes commands") {
	CommandQueue queue;
	int counter = 0;
	
	queue.enqueue([&counter]() {
		counter++;
	});
	
	CHECK(counter == 0); // Not executed yet
	CHECK(!queue.is_empty());
	
	queue.process();
	CHECK(counter == 1); // Executed
	CHECK(queue.is_empty());
}

/**
 * @test CommandQueue processes multiple commands
 */
TEST_CASE("[Command] CommandQueue processes multiple commands in FIFO order") {
	CommandQueue queue;
	std::vector<int> execution_order;
	
	queue.enqueue([&execution_order]() {
		execution_order.push_back(1);
	});
	
	queue.enqueue([&execution_order]() {
		execution_order.push_back(2);
	});
	
	queue.enqueue([&execution_order]() {
		execution_order.push_back(3);
	});
	
	queue.process();
	
	REQUIRE(execution_order.size() == 3);
	CHECK(execution_order[0] == 1);
	CHECK(execution_order[1] == 2);
	CHECK(execution_order[2] == 3);
}

/**
 * @test CommandQueue is_empty
 */
TEST_CASE("[Command] CommandQueue is_empty works correctly") {
	CommandQueue queue;
	
	CHECK(queue.is_empty());
	
	queue.enqueue([]() {});
	CHECK(!queue.is_empty());
	
	queue.process();
	CHECK(queue.is_empty());
}

/**
 * @test CommandQueue enqueue_raw
 */
TEST_CASE("[Command] CommandQueue enqueue_raw accepts pre-constructed commands") {
	CommandQueue queue;
	int counter = 0;
	
	ICommand* cmd = make_command([&counter]() {
		counter++;
	});
	
	queue.enqueue_raw(cmd);
	queue.process();
	
	CHECK(counter == 1);
}

/**
 * @test CommandQueue enqueue_raw with nullptr
 */
TEST_CASE("[Command] CommandQueue enqueue_raw handles nullptr safely") {
	CommandQueue queue;
	
	// Should not crash
	queue.enqueue_raw(nullptr);
	queue.process();
	
	CHECK(queue.is_empty());
}

/**
 * @test CommandQueue processes commands only once
 */
TEST_CASE("[Command] CommandQueue processes each command only once") {
	CommandQueue queue;
	int counter = 0;
	
	queue.enqueue([&counter]() {
		counter++;
	});
	
	queue.process();
	CHECK(counter == 1);
	
	queue.process(); // Process again
	CHECK(counter == 1); // Should still be 1
}

/**
 * @test CommandQueue with complex captures
 */
TEST_CASE("[Command] CommandQueue handles complex captured data") {
	CommandQueue queue;
	
	struct ComplexData {
		int value;
		String name;
		Vector3 position;
	};
	
	ComplexData data{42, "Test", Vector3(1, 2, 3)};
	ComplexData result;
	
	queue.enqueue([data, &result]() {
		result = data;
	});
	
	queue.process();
	
	CHECK(result.value == 42);
	CHECK(result.name == "Test");
	CHECK(result.position == Vector3(1, 2, 3));
}

/**
 * @test CommandHandler basic usage
 */
TEST_CASE("[Command] CommandHandler enqueues and processes commands") {
	Ref<CommandHandler> handler = memnew(CommandHandler);
	int counter = 0;
	
	handler->enqueue_command([&counter]() {
		counter++;
	});
	
	CHECK(counter == 0);
	
	handler->process_commands();
	CHECK(counter == 1);
}

/**
 * @test CommandHandler multiple enqueues
 */
TEST_CASE("[Command] CommandHandler handles multiple commands") {
	Ref<CommandHandler> handler = memnew(CommandHandler);
	int sum = 0;
	
	handler->enqueue_command([&sum]() { sum += 1; });
	handler->enqueue_command([&sum]() { sum += 2; });
	handler->enqueue_command([&sum]() { sum += 3; });
	
	handler->process_commands();
	CHECK(sum == 6);
}

/**
 * @test CommandHandler unpooled commands
 */
TEST_CASE("[Command] CommandHandler enqueue_command_unpooled works") {
	Ref<CommandHandler> handler = memnew(CommandHandler);
	int counter = 0;
	
	handler->enqueue_command_unpooled([&counter]() {
		counter++;
	});
	
	handler->process_commands();
	CHECK(counter == 1);
}

/**
 * @test CommandHandler mixed pooled and unpooled
 */
TEST_CASE("[Command] CommandHandler mixes pooled and unpooled commands") {
	Ref<CommandHandler> handler = memnew(CommandHandler);
	std::vector<int> execution_order;
	
	handler->enqueue_command([&execution_order]() {
		execution_order.push_back(1); // Pooled
	});
	
	handler->enqueue_command_unpooled([&execution_order]() {
		execution_order.push_back(2); // Unpooled
	});
	
	handler->enqueue_command([&execution_order]() {
		execution_order.push_back(3); // Pooled
	});
	
	handler->process_commands();
	
	REQUIRE(execution_order.size() == 3);
	CHECK(execution_order[0] == 1);
	CHECK(execution_order[1] == 2);
	CHECK(execution_order[2] == 3);
}

/**
 * @test CommandHandler RefCounted behavior
 */
TEST_CASE("[Command] CommandHandler works as RefCounted") {
	Ref<CommandHandler> handler1 = memnew(CommandHandler);
	CHECK(handler1.is_valid());
	
	Ref<CommandHandler> handler2 = handler1;
	CHECK(handler2.is_valid());
	CHECK(handler1.ptr() == handler2.ptr());
}

/**
 * @test Thread safety - multiple producers
 */
TEST_CASE("[Command] CommandQueue is thread-safe for enqueueing") {
	CommandQueue queue;
	std::atomic<int> counter{0};
	const int num_threads = 4;
	const int commands_per_thread = 100;
	
	std::vector<std::thread> threads;
	
	for (int t = 0; t < num_threads; ++t) {
		threads.emplace_back([&queue, &counter, commands_per_thread]() {
			for (int i = 0; i < commands_per_thread; ++i) {
				queue.enqueue([&counter]() {
					counter.fetch_add(1, std::memory_order_relaxed);
				});
			}
		});
	}
	
	// Wait for all threads to finish enqueueing
	for (auto& thread : threads) {
		thread.join();
	}
	
	// Process all commands
	queue.process();
	
	// Should have executed all commands
	CHECK(counter.load() == num_threads * commands_per_thread);
}

/**
 * @test Thread safety - enqueue while processing (separate threads)
 */
TEST_CASE("[Command] CommandQueue can enqueue from other threads during processing") {
	CommandQueue queue;
	std::atomic<bool> processing_started{false};
	std::atomic<int> counter{0};
	
	// Pre-fill with some commands
	for (int i = 0; i < 50; ++i) {
		queue.enqueue([&counter]() {
			counter++;
		});
	}
	
	// Thread that will enqueue during processing
	std::thread enqueue_thread([&queue, &processing_started, &counter]() {
		// Wait for processing to start
		while (!processing_started.load()) {
			std::this_thread::yield();
		}
		
		// Enqueue more commands while processing
		for (int i = 0; i < 50; ++i) {
			queue.enqueue([&counter]() {
				counter++;
			});
		}
	});
	
	// Process commands
	processing_started = true;
	queue.process();
	
	enqueue_thread.join();
	
	// Process the remaining commands
	queue.process();
	
	// Should have executed all 100 commands
	CHECK(counter.load() == 100);
}

/**
 * @test Performance - many small commands
 */
TEST_CASE("[Command] CommandQueue handles many small commands efficiently") {
	CommandQueue queue;
	const int num_commands = 10000;
	int counter = 0;
	
	for (int i = 0; i < num_commands; ++i) {
		queue.enqueue([&counter]() {
			counter++;
		});
	}
	
	queue.process();
	CHECK(counter == num_commands);
}

/**
 * @test Command with move-only capture
 */
TEST_CASE("[Command] CommandQueue handles move-only captured types") {
	CommandQueue queue;
	
	struct MoveOnly {
		int value;
		MoveOnly(int v) : value(v) {}
		MoveOnly(const MoveOnly&) = delete;
		MoveOnly& operator=(const MoveOnly&) = delete;
		MoveOnly(MoveOnly&&) = default;
		MoveOnly& operator=(MoveOnly&&) = default;
	};
	
	MoveOnly data(42);
	int result = 0;
	
	queue.enqueue([data = std::move(data), &result]() mutable {
		result = data.value;
	});
	
	queue.process();
	CHECK(result == 42);
}

/**
 * @test CommandBase execute calls functor
 */
TEST_CASE("[Command] CommandBase executes stored functor") {
	struct TestCommand : CommandBase<TestCommand, std::function<void()>> {
		using Base = CommandBase<TestCommand, std::function<void()>>;
		using Base::Base;
		
		static Pool& pool() {
			static Pool p(sizeof(TestCommand), 10);
			return p;
		}
	};
	
	int counter = 0;
	std::function<void()> func = [&counter]() { counter++; };
	
	void* mem = TestCommand::pool().allocate();
	TestCommand* cmd = new (mem) TestCommand(std::move(func));
	
	cmd->execute();
	CHECK(counter == 1);
	
	cmd->~TestCommand();
	TestCommand::pool().deallocate(cmd);
}

/**
 * @test CommandBase release returns to pool
 */
TEST_CASE("[Command] CommandBase release returns command to pool") {
	struct TestCommand : CommandBase<TestCommand, std::function<void()>> {
		using Base = CommandBase<TestCommand, std::function<void()>>;
		using Base::Base;
		
		static Pool& pool() {
			static Pool p(sizeof(TestCommand), 1);
			return p;
		}
	};
	
	std::function<void()> func = []() {};
	
	void* mem = TestCommand::pool().allocate();
	CHECK(mem != nullptr);
	TestCommand* cmd = new (mem) TestCommand(std::move(func));
	
	// Pool should be exhausted
	void* mem2 = TestCommand::pool().allocate();
	CHECK(mem2 == nullptr);
	
	// Release returns to pool
	cmd->~TestCommand();
	cmd->release();
	
	// Pool should have slot available again
	void* mem3 = TestCommand::pool().allocate();
	CHECK(mem3 != nullptr);
	
	TestCommand::pool().deallocate(mem3);
}

/**
 * @test Destructor cleanup
 */
TEST_CASE("[Command] CommandQueue destructor clears pending commands") {
	int counter = 0;
	
	{
		CommandQueue queue;
		
		// Enqueue but don't process
		queue.enqueue([&counter]() {
			counter++;
		});
		
		CHECK(counter == 0);
		// queue destructor should clean up
	}
	
	// Commands should be destroyed (not executed)
	CHECK(counter == 0);
}

/**
 * @test UnpooledCommand execution
 */
TEST_CASE("[Command] UnpooledCommand executes correctly") {
	int counter = 0;
	
	UnpooledCommand cmd([&counter]() {
		counter++;
	});
	
	cmd.execute();
	CHECK(counter == 1);
}

/**
 * @test UnpooledCommand release uses delete
 */
TEST_CASE("[Command] UnpooledCommand release uses delete") {
	int counter = 0;
	
	UnpooledCommand* cmd = new UnpooledCommand([&counter]() {
		counter++;
	});
	
	cmd->execute();
	CHECK(counter == 1);
	
	// Release should delete (not crash)
	cmd->release();
}

/**
 * @test Edge case - empty process
 */
TEST_CASE("[Command] CommandQueue process with no commands is safe") {
	CommandQueue queue;
	
	// Should not crash
	queue.process();
	queue.process();
	queue.process();
	
	CHECK(queue.is_empty());
}

/**
 * @test Edge case - large capture
 */
TEST_CASE("[Command] CommandQueue handles large captures") {
	CommandQueue queue;
	
	struct LargeData {
		uint8_t buffer[1024];
		int value;
	};
	
	LargeData data;
	data.value = 12345;
	for (int i = 0; i < 1024; ++i) {
		data.buffer[i] = static_cast<uint8_t>(i % 256);
	}
	
	int result = 0;
	
	queue.enqueue([data, &result]() {
		result = data.value;
	});
	
	queue.process();
	CHECK(result == 12345);
}

/**
 * @test Stress test - rapid enqueue/process cycles
 */
TEST_CASE("[Command] CommandQueue handles rapid enqueue/process cycles") {
	CommandQueue queue;
	int counter = 0;
	
	for (int cycle = 0; cycle < 100; ++cycle) {
		// Enqueue some commands
		for (int i = 0; i < 10; ++i) {
			queue.enqueue([&counter]() {
				counter++;
			});
		}
		
		// Process immediately
		queue.process();
	}
	
	CHECK(counter == 1000);
}

} // namespace TestCommand