#pragma once

#include <istream>
#include <functional>

class BlockStreamReader
{
public:
	using CallBack = std::function<void(std::shared_ptr<char[]>&& data, size_t blockSize, size_t number)>;

	void Read(std::istream& stream, size_t blockSize, CallBack blockDataProcess);
};
