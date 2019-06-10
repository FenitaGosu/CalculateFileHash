#pragma once

#include <memory>
#include <string>
#include <filesystem>

class CommandLineParser
{
public:
	enum class ArgumentSettings
	{
		Required = 1,
		DefaultValue,
	};

	CommandLineParser(const std::string& description = std::string());
	~CommandLineParser();

	void AddHelpDescription(const std::string& decription = std::string());
	void AddPathOption(const std::string& name, const std::string& decription, ArgumentSettings settings, std::filesystem::path& path, const std::filesystem::path& defaultValue = std::filesystem::path());
	void AddIntOption(const std::string& name, const std::string& decription, ArgumentSettings settings, int& value, int defaultValue = int());

	void Parse(int argc, const char* argv[]);

private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};
