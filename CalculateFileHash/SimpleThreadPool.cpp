#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <iostream>

#include "SimpleThreadPool.h"

struct SimpleThreadPool::Impl
{
	Impl(size_t maxTaskCount)
		: maxTaskCount(maxTaskCount)
	{
	}

	std::vector<std::thread> workers;
	std::queue<Task> tasks;
	std::mutex tasksMutex;
	std::condition_variable condVariable;
	std::atomic<bool> stoped{ false };

	std::atomic<size_t> maxTaskCount;
	std::mutex maxTaskMutex;
	std::condition_variable maxCondVariable;
};

size_t SimpleThreadPool::GetMaxThreadAvailable()
{
	return std::thread::hardware_concurrency();
}

SimpleThreadPool::SimpleThreadPool(size_t threadCount, size_t maxTaskCount)
	: m_impl(std::make_unique<Impl>(maxTaskCount))
{
	m_impl->workers.reserve(threadCount);

	std::generate_n(std::back_insert_iterator<std::vector<std::thread>>(m_impl->workers), threadCount, [this]()
	{
		return std::thread(std::bind(&SimpleThreadPool::Work, this));
	});
}

SimpleThreadPool::~SimpleThreadPool()
{
	if (!m_impl->stoped)
		WaitForFinish();
}

void SimpleThreadPool::WaitForFinish()
{
	m_impl->stoped = true;
	m_impl->condVariable.notify_all();

	std::for_each(m_impl->workers.begin(), m_impl->workers.end(), std::bind(&std::thread::join, std::placeholders::_1));
}

void SimpleThreadPool::AddTask(Task task)
{
	if (m_impl->stoped)
		return;

	{
		std::unique_lock<std::mutex> guard(m_impl->maxTaskMutex);
		m_impl->maxCondVariable.wait(guard, [impl = m_impl.get()]
		{
			std::unique_lock<std::mutex> guard(impl->tasksMutex);
			return impl->tasks.size() < impl->maxTaskCount;
		});
	}

	std::unique_lock<std::mutex> guard(m_impl->tasksMutex);

	m_impl->tasks.push(task);
	m_impl->condVariable.notify_one();
}

void SimpleThreadPool::Work()
{
	try
	{
		while (true)
		{
			std::function<void()> currentTask;

			{
				std::unique_lock<std::mutex> guard(m_impl->tasksMutex);

				m_impl->condVariable.wait(guard, [impl = m_impl.get()]() { return impl->stoped || !impl->tasks.empty(); });

				if (m_impl->stoped && m_impl->tasks.empty())
					return;

				currentTask = std::move(m_impl->tasks.front());
				m_impl->tasks.pop();
				m_impl->maxCondVariable.notify_one();
			}

			currentTask();
		}
	}
	catch (std::exception& exp)
	{
		std::cerr << exp.what();
		return;
	}
	catch (...)
	{
		std::cerr << "Something went wrong";
		return;
	}
}
