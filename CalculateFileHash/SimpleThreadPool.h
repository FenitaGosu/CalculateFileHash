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
	SimpleThreadPool(size_t threadCount, size_t maxTaskCount);
	~SimpleThreadPool();

	void AddTask(Task task);

private:
	void ThreadFunction();

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};
