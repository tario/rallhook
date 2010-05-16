/*

This file is part of the rallhook project, http://github.com/tario/rallhook

Copyright (c) 2009-2010 Roberto Dario Seminara <robertodarioseminara@gmail.com>

rallhook is free software: you can redistribute it and/or modify
it under the terms of the gnu general public license as published by
the free software foundation, either version 3 of the license, or
(at your option) any later version.

rallhook is distributed in the hope that it will be useful,
but without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.  see the
gnu general public license for more details.

you should have received a copy of the gnu general public license
along with rallhook.  if not, see <http://www.gnu.org/licenses/>.

*/

#include "hook.h"
#include <sys/mman.h>
#include "distorm.h"
#include "string.h"
#include "errno.h"

int get_instructions_size(void* code, int size) {
	_DecodedInst decodedInstructions[32];

	_OffsetType offset = 0;

	int decodedInstructionsCount;

	#ifdef __x86_64__
		_DecodeType dt = Decode32Bits;
	#elif __i386__
		_DecodeType dt = Decode64Bits;
	#else
		#error "unknown architecture"
	#endif

	distorm_decode(offset, code, size, dt, decodedInstructions, 32, &decodedInstructionsCount);

	int i;
	int totalsize = 0;
	int minsize = get_jmp_size();
	for (i = 0; i < decodedInstructionsCount; i++) {
		totalsize = totalsize + decodedInstructions[i].size;
		if (totalsize >= minsize) {
			return totalsize;
		}
	}

	return totalsize;

}


void unprotect(void* ptr) {
	unsigned long int mask = 0xFFFFFFFFFFFF000;
	int ret = mprotect( (void*) ( ( (unsigned long int)ptr ) & mask ), 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);

	if (ret == -1) {
		rb_bug("mprotect failed: %s", strerror(errno));
	}
}

void inconditional_jump(void* where, void* to) {

#if __x86_64__
	unsigned char* p = (unsigned char*)where;

	p[0] = 0x48; // movl XXX, %rax
	p[1] = 0xb8;

	void** address = (void**)(p+2);

	p[10] = 0xff; // jmp %rax
	p[11] = 0xe0;

	*address = to;
#elif __i386__
	unsigned char* p = (unsigned char*)where;

	p[0] = 0x68; // pushl
	void** address = (void**)(p+1);
	*address = to;
	p[5] = 0xc3; // ret
#else
	#error "unknow architecture"
#endif
}


void* put_jmp_hook(void* function_address, void* fake_function, int instructions_size) {
	typedef unsigned char uchar;

	uchar* p_copy = (uchar*)malloc(0x1000);
	uchar* p = (uchar*)function_address;

	unprotect(p_copy);
	unprotect(p);

	memcpy(p_copy, p, instructions_size);

	inconditional_jump(p, fake_function);
	inconditional_jump(p_copy+instructions_size, p+instructions_size);

	return (void*)p_copy;
}

int get_jmp_size() {
	#if __x86_64__
	return 12;
	#elif __i386__
	return 6;
	#else
		#error "unknown architecture"
	#endif
}
