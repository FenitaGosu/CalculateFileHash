#include <iostream>
#include <cstdlib>
#include <memory>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "CommandLineParser.h"
#include "BlockStreamReader.h"
#include "SimpleThreadPool.h"

int main(int argc, const char* argv[])
{
	try
	{
		std::filesystem::path input;
		std::filesystem::path output;
		size_t blockSize;

		{
			const auto parser = std::make_unique<CommandLineParser>("Programm usage:");

			parser->AddHelpDescription();

			parser->AddOption("input,i"		, "intput file path"	, CommandLineParser::ArgumentSettings::Required		, input);
			parser->AddOption("output,o"	, "output file path"	, CommandLineParser::ArgumentSettings::Required		, output);
			parser->AddOption("size,s"		, "block size"			, CommandLineParser::ArgumentSettings::DefaultValue	, blockSize, 1024 * 1024);

			parser->Parse(argc, argv);
		}

		if (blockSize == 0)
			throw std::invalid_argument("Block size cannot be 0");

		std::ifstream inputStream;
		inputStream.open(input, std::ifstream::binary | std::ios::in);

		if (!inputStream.is_open())
			throw std::runtime_error("Could not open input file");

		std::ofstream outpuStream;
		outpuStream.open(output, std::ofstream::binary | std::ios::out);

		if (!outpuStream.is_open())
			throw std::runtime_error("Could not open output file");

		const auto threadPhool = std::make_unique<SimpleThreadPool>(SimpleThreadPool::GetMaxTreadAvailable() - 1, 20);

		const auto reader = std::make_unique<BlockStreamReader>();

		reader->Read(inputStream, blockSize, [pool = threadPhool.get()](char* data, size_t blockSize, size_t number)
		{
			std::vector<char> copyData(data, data + blockSize);
			pool->AddTask([copyData, blockSize, number]()
			{
				std::cout << copyData.size() << " "<< blockSize << " " << number << std::endl;
			});
		});

		return EXIT_SUCCESS;
	}
	catch (std::exception& exp)
	{
		std::cerr << exp.what();
		return EXIT_FAILURE;
	}
}
