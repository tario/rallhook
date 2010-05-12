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

#include "hook_rb_call.h"
#include "hook.h"
#include "ruby_symbols.h"
#include "ruby_version.h"
#include "distorm.h"

#define __USE_GNU
#include <dlfcn.h>

void* rb_call_original = 0;

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


void* hook_rb_call(void* fake_function) {
	int replaced = 0;

	if (rb_call_original == 0) {
		void* handle = dlopen(current_libruby(),0x101);
		char* rb_funcall = (char*)dlsym(handle, "rb_funcall");
		Dl_info info;
		dladdr(rb_funcall, &info);

		unsigned char* base = (unsigned char*)info.dli_fbase;

		#ifdef RUBY1_8
		rb_call_original = ruby_resolv(base, "rb_call");
		#endif

		// in the ruby 1.9 source, the rb_call0 acts as rb_call (and vm_call0 acts as rb_call0)
		#ifdef RUBY1_9
		rb_call_original = ruby_resolv(base, "rb_call0");
		#endif

		if (rb_call_original == 0) {
			return 0;
		}

	}

/*	if (memcmp(rb_call_original, "\x48\x89\x5c\x24\xd0\x4c\x89\x64\x24\xe0\x48\x89\xd3" ,13)==0) {
		replaced = 1;*/
//	}

/*	if (memcmp(rb_call_original, "\x48\x89\x6c\x24\xd8\x4c\x89\x74\x24\xf0\x48\x89\xd5" ,13)==0) {
		replaced = 1;*/
//	}

	int inst_size = get_instructions_size(rb_call_original, 256);
	return put_jmp_hook(rb_call_original, fake_function, inst_size);

}

