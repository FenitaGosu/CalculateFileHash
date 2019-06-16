#include <queue>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <iterator>
#include <iostream>

#include "AsyncHashStreamWriter.h"

struct AsyncHashStreamWriter::Impl
{
	Impl(std::function<void()> threadFunction, size_t batchSize, std::ostream& stream)
		: queue([](const std::pair<size_t, size_t>& lhs, const std::pair<size_t, size_t>& rhs) { return lhs.first > rhs.first; })
		, batchSize(batchSize)
		, stream(stream)
		, thread(threadFunction)
		, lastNumber(-1)
	{
	}

	std::priority_queue<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>, std::function<bool(const std::pair<size_t, size_t>& lhs, const std::pair<size_t, size_t>& rhs)>> queue;

	std::atomic<size_t> batchSize;
	std::ostream& stream;

	std::thread thread;
	std::mutex queueMutex;
	std::condition_variable conVariable;
	std::atomic<bool> stoped{ false };

	std::atomic<int> lastNumber;
};

AsyncHashStreamWriter::AsyncHashStreamWriter(size_t batchSize, std::ostream& stream)
	: m_impl(std::make_unique<Impl>(std::bind(&AsyncHashStreamWriter::Work, this), batchSize, stream))
{
}

AsyncHashStreamWriter::~AsyncHashStreamWriter()
{
	if (!m_impl->stoped)
		WaitForFinish();
}

void AsyncHashStreamWriter::WaitForFinish()
{
	m_impl->stoped = true;
	m_impl->conVariable.notify_all();
	m_impl->thread.join();
}

void AsyncHashStreamWriter::AddHashToWrite(size_t hash, size_t number)
{
	if (m_impl->stoped)
		return;

	std::unique_lock<std::mutex> guard(m_impl->queueMutex);

	m_impl->queue.emplace(number, hash);
	m_impl->conVariable.notify_one();
}

void AsyncHashStreamWriter::Work()
{
	try
	{
		while (true)
		{
			std::vector<size_t> hashes;
			//std::vector<size_t> numbers;
			{
				std::unique_lock<std::mutex> guard(m_impl->queueMutex);

				m_impl->conVariable.wait(guard, [impl = m_impl.get()] { return impl->stoped || impl->queue.size() > impl->batchSize; });

				if (m_impl->stoped && m_impl->queue.empty())
					return;

				while (!m_impl->queue.empty())
				{
					auto& [number, hash] = m_impl->queue.top();

					if ((number - m_impl->lastNumber) != 1)
						break;

					m_impl->lastNumber = number;
					hashes.push_back(hash);
					//numbers.push_back(number);
					m_impl->queue.pop();
				}

				if (m_impl->stoped && !m_impl->queue.empty())
					throw std::logic_error("Not all blocks are added to the write");
			}

			std::copy(hashes.cbegin(), hashes.cend(), std::ostream_iterator<size_t>(m_impl->stream, "\n"));
			//std::copy(numbers.cbegin(), numbers.cend(), std::ostream_iterator<size_t>(m_impl->stream, "\n"));
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
