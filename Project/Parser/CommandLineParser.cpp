#include <iostream>

#include <boost/program_options.hpp>

#include "CommandLineParser.h"

namespace
{
	const std::string HELP_STRING = "help";
}

struct CommandLineParser::Impl
{
	Impl(const std::string& description)
		: options(description)
	{
	}

	boost::program_options::options_description options;
};

CommandLineParser::CommandLineParser(const std::string& description)
	: m_impl(std::make_unique<Impl>(description))
{
}

CommandLineParser::~CommandLineParser() = default;

void CommandLineParser::AddHelpDescription(const std::string& decription)
{
	m_impl->options.add_options()(HELP_STRING.c_str(), decription.c_str());
}

void CommandLineParser::AddOption(const std::string& name, const std::string& decription, ArgumentSettings settings, std::filesystem::path& path, const std::filesystem::path& defaultValue)
{
	auto option = boost::program_options::value(&path);

	if (settings == ArgumentSettings::Required)
		option->required();
	else
		option->default_value(defaultValue);

	m_impl->options.add_options()(name.c_str(), option, decription.c_str());
}

void CommandLineParser::AddOption(const std::string& name, const std::string& decription, ArgumentSettings settings, size_t& value, size_t defaultValue)
{
	auto option = boost::program_options::value(&value);

	if (settings == ArgumentSettings::Required)
		option->required();
	else
		option->default_value(defaultValue);

	m_impl->options.add_options()(name.c_str(), option, decription.c_str());
}

void CommandLineParser::Parse(int argc, const char* argv[])
{
	boost::program_options::variables_map map;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, m_impl->options), map);

	if (map.count(HELP_STRING.c_str()))
	{
		std::cout << m_impl->options << std::endl;
		return;
	}

	notify(map);
}
