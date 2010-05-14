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

#include <ruby.h>
#include "rb_yield_fake.h"
#include "hook.h"
#include "ruby_symbols.h"
#include "distorm.h"

#define __USE_GNU
#include <dlfcn.h>

typedef VALUE (*RBYIELD0)(VALUE val, VALUE self, VALUE klass, int flags, int avalue);

RBYIELD0 rb_yield_0_copy;
void* rb_yield_0_original;

static VALUE
rb_yield_0_fake(val, self, klass, flags, avalue)
    VALUE val, self, klass;	/* OK */
    int flags, avalue;
{
	return rb_yield_0_copy(val,self,klass,flags,avalue);
}

void* hook_rb_yield_0(void* fake_function) {
	int inst_size = get_instructions_size(rb_yield_0_original, 256);
	return put_jmp_hook(rb_yield_0_original, fake_function, inst_size);

}

void init_rb_yield_fake() {

#ifdef RUBY1_8
	void* handle = dlopen(current_libruby(),0x101);
	char* rb_funcall = (char*)dlsym(handle, "rb_funcall");
	Dl_info info;
	dladdr(rb_funcall, &info);

	unsigned char* base = (unsigned char*)info.dli_fbase;

	rb_yield_0_original = resolv(base, "rb_yield_0" );
	rb_yield_0_copy = (RBYIELD0)hook_rb_yield_0(rb_yield_0_fake);
#endif

}

