/// @file thread_pool.ixx
/// @author Xein
/// @date 6/11/25

module;

#include <condition_variable>
#include <functional>
#include <mutex>
#include <print>
#include <queue>
#include <thread>
#include <vector>

export module thread_pool;

namespace toast {

export class ThreadPool {
public:
	/// @brief Initializes the thread pool
	/// @param size Number of workers to create
	void Init(size_t size);

	/// @brief Adds a job to the queue to be picked by a worker
	void QueueJob(std::function<void()>&& job);

	/// @brief Ends the thread pool
	void Destroy();

	[[nodiscard]]
	bool busy();

private:
	void ThreadLoop();

	bool m_shouldStop = false;
	std::mutex m_queueMutex;
	std::condition_variable m_conditionMutex;
	std::vector<std::thread> m_workers;
	std::queue<std::function<void()>> m_jobs;
};

void ThreadPool::Init(size_t size) {
	const size_t max_thread_num = std::thread::hardware_concurrency();
	if (size == 0) {
		size = max_thread_num;
	}
	size_t target_thread_num = std::min(size, max_thread_num);
	for (size_t i = 0; i < target_thread_num; ++i) {
		m_workers.emplace_back(&ThreadPool::ThreadLoop, this);
	}

	std::println("Created thread pool with {0} workers", target_thread_num);
}

void ThreadPool::QueueJob(std::function<void()>&& job) {
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_jobs.emplace(job);
	}
	m_conditionMutex.notify_one();
}

void ThreadPool::Destroy() {
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_shouldStop = true;
	}
	m_conditionMutex.notify_all();

	for (std::thread& active_thread : m_workers) {
		active_thread.join();
	}

	m_workers.clear();

	std::println("Destroyed thread pool");
}

bool ThreadPool::busy() {
	bool pool_busy = false;
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		pool_busy = !m_jobs.empty();
	}
	return pool_busy;
}

void ThreadPool::ThreadLoop() {
	while (true) {
		std::function<void()> job;

		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_conditionMutex.wait(lock, [this] {
				return !m_jobs.empty() || m_shouldStop;
			});
			if (m_shouldStop) {
				return;
			}
			job = m_jobs.front();
			m_jobs.pop();
		}

		job();
	}
}

}
