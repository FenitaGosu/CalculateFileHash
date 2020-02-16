#pragma once

#include <memory>
#include <functional>

class SimpleThreadPool
{
public:
	using Task = std::function<void()>;

public:
	static size_t GetMaxThreadAvailable();

public:
	SimpleThreadPool(size_t threadCount, size_t maxTaskCount);
	~SimpleThreadPool();

	void WaitForFinish();

	void AddTask(Task task);

private:
	void Work();

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};
