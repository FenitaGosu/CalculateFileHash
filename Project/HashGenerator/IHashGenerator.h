#pragma once

class IHashGenerator
{
public:
	virtual ~IHashGenerator() = default;

	virtual unsigned int GenerateCRC32(char* data, size_t size) const = 0;
};
