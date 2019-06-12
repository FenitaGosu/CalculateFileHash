#pragma once

#include <memory>
#include <functional>

class SimpleThreadPool
{
public:
	using Task = std::function<void()>;

public:
	static size_t GetMaxTreadAvailable();

public:
	SimpleThreadPool(size_t threadCount);
	~SimpleThreadPool();

	void AddTask(Task task);

private:
	void ThreadFunction();

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};
