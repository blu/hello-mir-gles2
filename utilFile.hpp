#ifndef utilFile_H__
#define utilFile_H__

namespace util {

bool get_file_size(
	const char* const filename,
	size_t& size);

char* get_buffer_from_file(
	const char* const filename,
	size_t& size,
	const size_t roundToLog2 = 0);

} // namespace util

#endif // utilFile_H__
