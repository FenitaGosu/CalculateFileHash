#include <queue>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <iterator>

#include "AsyncHashStreamWriter.h"

struct AsyncHashStreamWriter::Impl
{
	Impl(std::function<void()> threadFunction, size_t batchSize, std::ostream& stream)
		: queue([](const std::pair<size_t, size_t>& lhs, const std::pair<size_t, size_t>& rhs) { return lhs.first > rhs.first; })
		, batchSize(batchSize)
		, stream(stream)
		, thread(threadFunction)
	{
	}

	std::priority_queue<std::pair<size_t, size_t>, std::vector<std::pair<size_t, size_t>>, std::function<bool(const std::pair<size_t, size_t>& lhs, const std::pair<size_t, size_t>& rhs)>> queue;

	std::atomic<size_t> batchSize;
	std::ostream& stream;

	std::thread thread;
	std::mutex queueMutex;
	std::condition_variable m_cVariable;
	std::atomic<bool> m_isStop{ false };
};

AsyncHashStreamWriter::AsyncHashStreamWriter(size_t batchSize, std::ostream& stream)
	: m_impl(std::make_unique<Impl>(std::bind(&AsyncHashStreamWriter::Work, this), batchSize, stream))
{
}

AsyncHashStreamWriter::~AsyncHashStreamWriter()
{
	if (!m_impl->m_isStop)
		WaitForFinish();
}

void AsyncHashStreamWriter::WaitForFinish()
{
	m_impl->m_isStop = true;
	m_impl->m_cVariable.notify_all();
	m_impl->thread.join();
}

void AsyncHashStreamWriter::AddHashToWrite(size_t hash, size_t number)
{
	if (m_impl->m_isStop)
		return;

	std::unique_lock<std::mutex> guard(m_impl->queueMutex);

	m_impl->queue.emplace(number, hash);
	m_impl->m_cVariable.notify_one();
}

void AsyncHashStreamWriter::Work()
{
	while (true)
	{
		std::vector<size_t> hashes;
		//std::vector<size_t> numbers;

		{
			std::unique_lock<std::mutex> guard(m_impl->queueMutex);

			m_impl->m_cVariable.wait(guard, [impl = m_impl.get()]() { return impl->m_isStop || impl->queue.size() > impl->batchSize; });

			if (m_impl->m_isStop && m_impl->queue.empty())
				return;

			auto [currentNumber, currentHash] = m_impl->queue.top();
			hashes.push_back(currentHash);
			//numbers.push_back(currentNumber);

			m_impl->queue.pop();

			while (!m_impl->queue.empty())
			{
				auto [number, hash] = m_impl->queue.top();

				if ((number - currentNumber) != 1)
					break;

				currentNumber = number;
				hashes.push_back(hash);
				//numbers.push_back(currentNumber);

				m_impl->queue.pop();
			}
		}

		std::copy(hashes.cbegin(), hashes.cend(), std::ostream_iterator<size_t>(m_impl->stream, "\n"));
		//std::copy(numbers.cbegin(), numbers.cend(), std::ostream_iterator<size_t>(m_impl->stream, "\n"));
	}
}
