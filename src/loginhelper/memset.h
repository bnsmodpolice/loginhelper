#pragma once

void memsetVP(void* _Dst, int _Val, size_t _Size) {
	DWORD  curProtection;
	VirtualProtect(_Dst, _Size, PAGE_EXECUTE_READWRITE, &curProtection);
	memset(_Dst, _Val, _Size);
	VirtualProtect(_Dst, _Size, curProtection, &curProtection);
}

void setaddr(unsigned char* addr, unsigned char* values, size_t size) {
	DWORD  curProtection;
	VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &curProtection);
	for (int i = 0; i < size; i++)
		addr[i] = values[i];
	VirtualProtect(addr, size, curProtection, &curProtection);
}

class MemsetElement {
public:
	uintptr_t handle_offset;
	std::vector<unsigned char> bytearray;
	size_t size() {
		return bytearray.size();
	}
	unsigned char* getoffset(uintptr_t handle) {
		return (unsigned char*)(handle + handle_offset);
	}
	void apply(uintptr_t handle) {
		setaddr(this->getoffset(handle), &this->bytearray[0], this->size());
	}

	MemsetElement(uintptr_t _handle_offset, std::vector<unsigned char> _bytearray) {
		handle_offset = _handle_offset;
		bytearray = _bytearray;
	}
};

class MemsetPattern {
public:
	uintptr_t handle_offset;
	std::string pattern;
	int offset;
	std::vector<unsigned char> bytearray;
	size_t size() {
		return bytearray.size();
	}
	unsigned char* getoffset(uintptr_t handle) {
		return (unsigned char*)(handle + handle_offset);
	}
	uintptr_t findoffset(uintptr_t handle, const gsl::span<uint8_t> data) {
		if (handle_offset != 0)
			return handle_offset;
		auto results = std::search(data.begin(), data.end(), pattern_searcher(pattern.c_str()));
		if (results == data.end())
		{
			return handle_offset = 0;
		}
		uintptr_t result = (uintptr_t)&results[0];
		return handle_offset = result + offset - handle;
	}

	MemsetPattern(const char* _pattern, int _offset, std::vector<unsigned char> _bytearray, uintptr_t _handle_offset = 0) {
		pattern = _pattern;
		offset = _offset;
		bytearray = _bytearray;
		handle_offset = _handle_offset;
	}
};

std::vector<MemsetPattern> memsetplist = {
	//MemsetPattern("FF FF", 3, {0}),
};