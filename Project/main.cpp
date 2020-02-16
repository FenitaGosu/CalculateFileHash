#include <iostream>
#include <cstdlib>
#include <memory>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <algorithm>

#include "Parser/CommandLineParser.h"
#include "Reader/BlockStreamReader.h"
#include "ThreadPool/SimpleThreadPool.h"
#include "Writer/AsyncHashStreamWriter.h"
#include "HashGenerator/HashGeneratorBoostImpl.h"

namespace
{
	constexpr size_t DEFAULT_BLOCK_SIZE			= 1024 * 1024;
	constexpr size_t DEFAULT_TASK_COUNT			= 20;
	constexpr size_t DEFAULT_DATA_BLOCK_COUNT	= 20;
}

void PrepareParser(std::unique_ptr<CommandLineParser>& parser, std::filesystem::path& input, std::filesystem::path& output, size_t& blockSize, size_t& maxTaskCount, size_t& maxDataBlockCount)
{
	parser->AddHelpDescription();

	parser->AddOption("input,i"				, "intput file path"				, CommandLineParser::ArgumentSettings::Required, input);
	parser->AddOption("output,o"			, "output file path"				, CommandLineParser::ArgumentSettings::Required, output);
	parser->AddOption("size,s"				, "block size"						, CommandLineParser::ArgumentSettings::DefaultValue, blockSize, DEFAULT_BLOCK_SIZE);
	parser->AddOption("taskCount,t"			, "max task count"					, CommandLineParser::ArgumentSettings::DefaultValue, maxTaskCount, DEFAULT_TASK_COUNT);
	parser->AddOption("dataBlockCount,d"	, "max data block count for output"	, CommandLineParser::ArgumentSettings::DefaultValue, maxDataBlockCount, DEFAULT_DATA_BLOCK_COUNT);
}

void CheckArguments(size_t blockSize, size_t maxTaskCount, size_t maxDataBlockCount)
{
	if (blockSize == 0)
		throw std::invalid_argument("Block size cannot be 0");

	if (maxTaskCount == 0)
		throw std::invalid_argument("Max task count cannot be 0");

	if (maxDataBlockCount == 0)
		throw std::invalid_argument("Max data block count for output cannot be 0");
}

std::pair<std::ifstream, std::ofstream> OpenStreams(const std::filesystem::path& input, const std::filesystem::path& output)
{
	std::ifstream inputStream;
	inputStream.open(input, std::ifstream::binary | std::ios::in);

	if (!inputStream.is_open())
		throw std::runtime_error("Could not open input file");

	std::ofstream outputStream;
	outputStream.open(output, std::ios::out);

	if (!outputStream.is_open())
		throw std::runtime_error("Could not open output file");

	return std::make_pair<std::ifstream, std::ofstream>(std::move(inputStream), std::move(outputStream));
}

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
			auto parser = std::make_unique<CommandLineParser>("Programm usage:");
			PrepareParser(parser, input, output, blockSize, maxTaskCount, maxDataBlockCount);
			parser->Parse(argc, argv);
		}

		CheckArguments(blockSize, maxTaskCount, maxDataBlockCount);

		auto [inputStream, outputStream] = OpenStreams(input, output);

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
