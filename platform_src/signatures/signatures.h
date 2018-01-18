#ifndef __SIGNATURE_HEADER__
#define __SIGNATURE_HEADER__

#include <string>
#include <vector>

struct SignatureF {
	const char* funcname;
	const char* signature;
	const char* mask;
	int offset;
	void* address;
};

class SignatureSearch {
public:
	SignatureSearch(const char* funcname, void* address, const char* signature, const char* mask, int offset);
	static void Search();
};

class FuncDetour {
public:
	FuncDetour(void** oldF, void* newF);
	~FuncDetour();
	void Attach();
	void Detach();
protected:
	void** oldFunction;
	void* newFunction;
};


#endif // __SIGNATURE_HEADER__