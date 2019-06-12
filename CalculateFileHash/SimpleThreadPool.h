#pragma once

#include <memory>

class SimpleThreadPool
{
public:
	static size_t GetMaxTreadAvailable();

public:
	SimpleThreadPool(size_t threadCount);
	~SimpleThreadPool();

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};
