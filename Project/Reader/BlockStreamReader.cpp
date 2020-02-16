#include <stdexcept>
#include <iostream>

#include "BlockStreamReader.h"

void BlockStreamReader::Read(std::istream& stream, size_t blockSize, CallBack blockDataProcess)
{
	try
	{
		size_t i = 0;
		while (!stream.eof())
		{
			std::shared_ptr<char[]> buffer(new char[blockSize]); /// C++17 correctly

			stream.read(buffer.get(), blockSize);
			const auto currentBlockSize = stream.gcount();

			if (stream.bad())
				throw std::runtime_error("Input stream corrupted");

			blockDataProcess(std::move(buffer), currentBlockSize, i++);
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
