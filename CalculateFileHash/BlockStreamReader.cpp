#include <stdexcept>

#include "BlockStreamReader.h"

void BlockStreamReader::Read(std::istream& stream, size_t blockSize, CallBack blockDataProcess)
{
	std::vector<char> buffer(blockSize);

	size_t i = 0;
	while (!stream.eof())
	{
		stream.read(buffer.data(), blockSize);
		const auto currentBlockSize = stream.gcount();
		
		if (stream.bad())
			throw std::runtime_error("Input stream corrupted");

		blockDataProcess(buffer.data(), currentBlockSize, i++);
	}
}
