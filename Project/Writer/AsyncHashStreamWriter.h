#pragma once

#include <memory>
#include <ostream>

class AsyncHashStreamWriter
{
public:
	AsyncHashStreamWriter(size_t batchSize, std::ostream& stream);
	~AsyncHashStreamWriter();

	void WaitForFinish();

	void AddHashToWrite(size_t hash, size_t number);

private:
	void Work();

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};
