#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>
#include <atomic>

#include "SimpleThreadPool.h"

struct SimpleThreadPool::Impl
{
	std::vector<std::thread> m_workers;
	std::queue<Task> m_tasks;
	std::mutex tasksMutex;
	std::condition_variable m_cVariable;
	std::atomic<bool> m_isStop{ false };
};

size_t SimpleThreadPool::GetMaxTreadAvailable()
{
	return std::thread::hardware_concurrency();
}

SimpleThreadPool::SimpleThreadPool(size_t threadCount)
	: m_impl(std::make_unique<Impl>())
{
	m_impl->m_workers.reserve(threadCount);

	std::generate_n(std::back_insert_iterator<std::vector<std::thread>>(m_impl->m_workers), threadCount, [this]()
	{
		return std::thread(std::bind(&SimpleThreadPool::ThreadFunction, this));
	});
}

SimpleThreadPool::~SimpleThreadPool()
{
	m_impl->m_isStop = true;
	m_impl->m_cVariable.notify_all();

	std::for_each(m_impl->m_workers.begin(), m_impl->m_workers.end(), std::bind(&std::thread::join, std::placeholders::_1));
}

void SimpleThreadPool::AddTask(Task task)
{
	std::unique_lock<std::mutex> guard(m_impl->tasksMutex);

	if (m_impl->m_isStop)
		return;

	m_impl->m_tasks.push(task);
}

void SimpleThreadPool::ThreadFunction()
{
	while (true)
	{
		std::function<void()> currentTask;

		{
			std::unique_lock<std::mutex> guard(m_impl->tasksMutex);

			m_impl->m_cVariable.wait(guard, [impl = m_impl.get()]() { return impl->m_isStop || !impl->m_tasks.empty(); });

			if (m_impl->m_isStop && m_impl->m_tasks.empty())
				return;

			currentTask = m_impl->m_tasks.front();
			m_impl->m_tasks.pop();
		}

		currentTask();
	}
}
