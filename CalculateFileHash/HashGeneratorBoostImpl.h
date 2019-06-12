#pragma once

#include "IHashGenerator.h"

class HashGeneratorBoostImpl
	: public IHashGenerator
{
public:
	unsigned int GenerateCRC32(char* data, size_t size) const override;
};
