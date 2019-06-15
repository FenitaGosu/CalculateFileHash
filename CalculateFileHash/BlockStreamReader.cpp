#include <stdexcept>

#include "BlockStreamReader.h"

void BlockStreamReader::Read(std::istream& stream, size_t blockSize, CallBack blockDataProcess)
{
	std::vector<char> buffer(blockSize);

	size_t i = 0;
	while (!stream.eof())
	{
		std::shared_ptr<char[]> buffer(new char[blockSize]);

		stream.read(buffer.get(), blockSize);
		const auto currentBlockSize = stream.gcount();
		
		if (stream.bad())
			throw std::runtime_error("Input stream corrupted");

		blockDataProcess(std::move(buffer), currentBlockSize, i++);
	}
}
