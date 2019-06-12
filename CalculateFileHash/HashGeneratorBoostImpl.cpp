#include <boost/crc.hpp>

#include "HashGeneratorBoostImpl.h"

unsigned int HashGeneratorBoostImpl::GenerateCRC32(char* data, size_t size) const
{
	boost::crc_32_type crc32;
	crc32.process_bytes(data, size);

	return crc32.checksum();
}
