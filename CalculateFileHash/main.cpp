#include <iostream>
#include <cstdlib>
#include <memory>
#include <filesystem>

#include "CommandLineParser.h"

int main(int argc, const char* argv[])
{
	try
	{
		const auto parser = std::make_unique<CommandLineParser>("Programm usage:");

		std::filesystem::path input;
		std::filesystem::path output;
		int blockSize;

		parser->AddHelpDescription();

		parser->AddPathOption	("input,i"		, "intput file path"	, CommandLineParser::ArgumentSettings::Required		, input);
		parser->AddPathOption	("output,o"		, "output file path"	, CommandLineParser::ArgumentSettings::Required		, output);
		parser->AddIntOption	("size,s"		, "block size"			, CommandLineParser::ArgumentSettings::DefaultValue	, blockSize, 1);

		parser->Parse(argc, argv);

		return EXIT_SUCCESS;
	}
	catch (std::exception& exp)
	{
		std::cerr << exp.what();
		return EXIT_FAILURE;
	}
}
