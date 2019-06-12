#include <thread>

#include "SimpleThreadPool.h"

struct SimpleThreadPool::Impl
{

};

size_t SimpleThreadPool::GetMaxTreadAvailable()
{
	return std::thread::hardware_concurrency();
}

SimpleThreadPool::SimpleThreadPool(size_t threadCount)
{
}

SimpleThreadPool::~SimpleThreadPool()
{
}
