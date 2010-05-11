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

#define __USE_GNU
#include <dlfcn.h>

typedef struct
{
  __const char *dli_fname;	/* File name of defining object.  */
  void *dli_fbase;		/* Load address of that object.  */
  __const char *dli_sname;	/* Name of nearest symbol.  */
  void *dli_saddr;		/* Exact value of nearest symbol.  */
} Dl_info;


void* rb_call_original = 0;

void* hook_rb_call(void* fake_function) {
	int replaced = 0;

	if (rb_call_original == 0) {
		void* handle = dlopen("/usr/lib/libruby1.8.so.1.8.7",0x101);
		char* rb_funcall = (char*)dlsym(handle, "rb_funcall");
		Dl_info info;
		dladdr(rb_funcall, &info);

		unsigned char* base = (unsigned char*)info.dli_fbase;

		rb_call_original = ruby_resolv(base, "rb_call");
	}

	if (memcmp(rb_call_original, "\x48\x89\x5c\x24\xd0\x4c\x89\x64\x24\xe0\x48\x89\xd3" ,13)==0) {
		replaced = 1;
		return put_jmp_hook(rb_call_original, fake_function, 13);
	}

	return 0;

}

