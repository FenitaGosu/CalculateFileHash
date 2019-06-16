#include <iostream>
#include <cstdlib>
#include <memory>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <algorithm>

#include "CommandLineParser.h"
#include "BlockStreamReader.h"
#include "SimpleThreadPool.h"
#include "AsyncHashStreamWriter.h"
#include "HashGeneratorBoostImpl.h"

int main(int argc, const char* argv[])
{
	try
	{
		std::filesystem::path input;
		std::filesystem::path output;
		size_t blockSize;
		size_t maxTaskCount;
		size_t maxDataBlockCount;

		{
			const auto parser = std::make_unique<CommandLineParser>("Programm usage:");

			parser->AddHelpDescription();

			parser->AddOption("input,i"				, "intput file path"				, CommandLineParser::ArgumentSettings::Required		, input);
			parser->AddOption("output,o"			, "output file path"				, CommandLineParser::ArgumentSettings::Required		, output);
			parser->AddOption("size,s"				, "block size"						, CommandLineParser::ArgumentSettings::DefaultValue	, blockSize, 1024 * 1024);
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			parser->AddOption("taskCount,t"			, "max task count"					, CommandLineParser::ArgumentSettings::DefaultValue, maxTaskCount, 20);
			parser->AddOption("dataBlockCount,d"	, "max data block count for output"	, CommandLineParser::ArgumentSettings::DefaultValue, maxDataBlockCount, 20);

			parser->Parse(argc, argv);
		}

		if (blockSize == 0)
			throw std::invalid_argument("Block size cannot be 0");

		if (maxTaskCount == 0)
			throw std::invalid_argument("Max task count cannot be 0");

		if (maxDataBlockCount == 0)
			throw std::invalid_argument("Max data block count for output cannot be 0");

		std::ifstream inputStream;
		inputStream.open(input, std::ifstream::binary | std::ios::in);

		if (!inputStream.is_open())
			throw std::runtime_error("Could not open input file");

		std::ofstream outputStream;
		outputStream.open(output, std::ios::out);

		if (!outputStream.is_open())
			throw std::runtime_error("Could not open output file");

		const auto threadPool	= std::make_unique<SimpleThreadPool>(std::max(SimpleThreadPool::GetMaxThreadAvailable() - 2, 1ULL), maxTaskCount);
		const auto writeHelper	= std::make_unique<AsyncHashStreamWriter>(maxDataBlockCount, outputStream);
		const auto readHelper	= std::make_unique<BlockStreamReader>();

		const std::unique_ptr<IHashGenerator> hashGenerator = std::make_unique<HashGeneratorBoostImpl>();

		readHelper->Read(inputStream, blockSize, [pool = threadPool.get(), writer = writeHelper.get(), hasher = hashGenerator.get()](std::shared_ptr<char[]>&& data, size_t blockSize, size_t number)
		{	
			pool->AddTask([blockData = std::move(data), blockSize, number, writer, hasher]()
			{
				const auto hash = hasher->GenerateCRC32(blockData.get(), blockSize);
				writer->AddHashToWrite(hash, number);
			});
		});

		threadPool->WaitForFinish();
		writeHelper->WaitForFinish();

		return EXIT_SUCCESS;
	}
	catch (std::exception& exp)
	{
		std::cerr << exp.what();
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "Something went wrong";
		return EXIT_FAILURE;
	}
}
