#include "xmltweaker_internal.h"
#include <stdio.h>
#include <fstream>
#include <unordered_set>
#include "util/util.h"
#include "signatures/sigdef.h"

extern "C" {
#include "wren.h"
}

using namespace std;
using namespace pd2hook;
using namespace tweaker;

static unordered_set<char*> buffers;

idstring *tweaker::last_loaded_name = NULL, *tweaker::last_loaded_ext = NULL;

void tweaker::init_xml_tweaker() {
	char *tmp;

	if (try_open_base_vr) {
		tmp = (char*)try_open_base_vr;
		tmp += 0x3D;
		last_loaded_name = *((idstring**)tmp);

		tmp = (char*)try_open_base_vr;
		tmp += 0x23;
		last_loaded_ext = *((idstring**)tmp);
	}
	else {
		tmp = (char*)try_open_base;
		tmp += 0x26;
		last_loaded_name = *((idstring**)tmp);

		tmp = (char*)try_open_base;
		tmp += 0x14;
		last_loaded_ext = *((idstring**)tmp);
	}
}

void* __cdecl tweaker::tweak_pd2_xml(char* text) {
	const char* new_text = transform_file(text);
	size_t length = strlen(new_text) + 1; // +1 for the null

	char* buffer = (char*)malloc(length);
	buffers.insert(buffer);

	strcpy_s(buffer, length, new_text);

	//if (!strncmp(new_text, "<network>", 9)) {
	//	std::ofstream out("output.txt");
	//	out << new_text;
	//	out.close();
	//}

	/*static int counter = 0;
	if (counter++ == 1) {
		printf("%s\n", buffer);

		const int kMaxCallers = 62;
		void* callers[kMaxCallers];
		int count = CaptureStackBackTrace(0, kMaxCallers, callers, NULL);
		for (int i = 0; i < count; i++)
			printf("*** %d called from .text:%08X\n", i, callers[i]);
		Sleep(20000);
	}*/

	return (char*)buffer;
}

void __cdecl tweaker::free_tweaked_pd2_xml(char* text) {
	if (buffers.erase(text)) {
		free(text);
	}
}

static void convert_block(const char* str, uint64_t* var, int* start_ptr, int len, int val) {
	int start = *start_ptr;
	*start_ptr -= len;

	uint64_t alt = *var;

	int i = 0;
	for (int possibleVal = start; possibleVal > start - len; possibleVal--) {
		i++;

		if (!(val >= possibleVal)) continue;

		int pow = 8 * (8 - i);

		*var += (uint64_t)(uint8_t)str[possibleVal - 1] << pow;
	}
}

typedef  unsigned long  long ub8;   /* unsigned 8-byte quantities */
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;

#define hashsize(n) ((ub8)1<<(n))
#define hashmask(n) (hashsize(n)-1)

#define mix64(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>43); \
  b -= c; b -= a; b ^= (a<<9); \
  c -= a; c -= b; c ^= (b>>8); \
  a -= b; a -= c; a ^= (c>>38); \
  b -= c; b -= a; b ^= (a<<23); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>35); \
  b -= c; b -= a; b ^= (a<<49); \
  c -= a; c -= b; c ^= (b>>11); \
  a -= b; a -= c; a ^= (c>>12); \
  b -= c; b -= a; b ^= (a<<18); \
  c -= a; c -= b; c ^= (b>>22); \
}

idstring Hash64(const ub1* k, ub8 length, ub8 level)
{
	register ub8 a, b, c, len;

	/* Set up the internal state */
	len = length;
	a = b = level;                         /* the previous hash value */
	c = 0x9e3779b97f4a7c13LL; /* the golden ratio; an arbitrary value */

							  /*---------------------------------------- handle most of the key */
	while (len >= 24)
	{
		a += (k[0] + ((ub8)k[1] << 8) + ((ub8)k[2] << 16) + ((ub8)k[3] << 24)
			+ ((ub8)k[4] << 32) + ((ub8)k[5] << 40) + ((ub8)k[6] << 48) + ((ub8)k[7] << 56));
		b += (k[8] + ((ub8)k[9] << 8) + ((ub8)k[10] << 16) + ((ub8)k[11] << 24)
			+ ((ub8)k[12] << 32) + ((ub8)k[13] << 40) + ((ub8)k[14] << 48) + ((ub8)k[15] << 56));
		c += (k[16] + ((ub8)k[17] << 8) + ((ub8)k[18] << 16) + ((ub8)k[19] << 24)
			+ ((ub8)k[20] << 32) + ((ub8)k[21] << 40) + ((ub8)k[22] << 48) + ((ub8)k[23] << 56));
		mix64(a, b, c);
		k += 24; len -= 24;
	}

	/*------------------------------------- handle the last 23 bytes */
	c += length;
	switch (len)              /* all the case statements fall through */
	{
	case 23: c += ((ub8)k[22] << 56);
	case 22: c += ((ub8)k[21] << 48);
	case 21: c += ((ub8)k[20] << 40);
	case 20: c += ((ub8)k[19] << 32);
	case 19: c += ((ub8)k[18] << 24);
	case 18: c += ((ub8)k[17] << 16);
	case 17: c += ((ub8)k[16] << 8);
		/* the first byte of c is reserved for the length */
	case 16: b += ((ub8)k[15] << 56);
	case 15: b += ((ub8)k[14] << 48);
	case 14: b += ((ub8)k[13] << 40);
	case 13: b += ((ub8)k[12] << 32);
	case 12: b += ((ub8)k[11] << 24);
	case 11: b += ((ub8)k[10] << 16);
	case 10: b += ((ub8)k[9] << 8);
	case  9: b += ((ub8)k[8]);
	case  8: a += ((ub8)k[7] << 56);
	case  7: a += ((ub8)k[6] << 48);
	case  6: a += ((ub8)k[5] << 40);
	case  5: a += ((ub8)k[4] << 32);
	case  4: a += ((ub8)k[3] << 24);
	case  3: a += ((ub8)k[2] << 16);
	case  2: a += ((ub8)k[1] << 8);
	case  1: a += ((ub8)k[0]);
		/* case 0: nothing left to add */
	}
	mix64(a, b, c);
	/*-------------------------------------------- report the result */
	return c;
}

idstring tweaker::idstring_hash(string text) {
	return Hash64((const unsigned char*)text.c_str(), text.length(), 0);
}