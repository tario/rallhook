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

#ifdef RUBY1_8
#include <node.h>
#include <env.h> // from ruby
#endif

#ifdef RUBY1_9
#include <ruby/node.h>
#endif

#include "rb_yield_fake.h"
#include "hook.h"
#include "ruby_symbols.h"
#include "distorm.h"

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <dlfcn.h>

#ifdef RUBY1_8

int last_avalue = 0;
typedef VALUE (*RBYIELD0)(VALUE val, VALUE self, VALUE klass, int flags, int avalue);

RBYIELD0 rb_yield_0_copy;
void* rb_yield_0_original;

VALUE expected_val;
int calibrate_convention_yield_0 = 0;
int yield_0_fastcall = 0;

static VALUE rb_yield_0_i(val, self, klass, flags, avalue)
    VALUE val, self, klass;	/* OK */
    int flags, avalue; 
{
	if (yield_0_fastcall) {
		__asm__("push %ebp\n");
		__asm__("push %esi\n");
		__asm__("push %edi\n");
		__asm__("push %ebx\n");
		__asm__("push %edx\n");
		__asm__("push %ecx\n");
		__asm__("mov 0x8(%ebp), %eax\n");
		__asm__("mov 0xc(%ebp), %edx\n");
		__asm__("mov 0x10(%ebp), %ecx\n");
		__asm__("push 0x18(%ebp)\n");
		__asm__("push 0x14(%ebp)\n");
		__asm__("call *rb_yield_0_copy\n");
		__asm__("add $0x8, %esp\n");
		__asm__("pop %ecx\n");
		__asm__("pop %edx\n");
		__asm__("pop %ebx\n");
		__asm__("pop %edi\n");
		__asm__("pop %esi\n");
		__asm__("pop %ebp\n");
		return read_eax();
	} else {
		return rb_yield_0_copy(val,self,klass,flags,avalue);
	}

}

static VALUE
rb_yield_0_fake(val, self, klass, flags, avalue)
    VALUE val, self, klass;	/* OK */
    int flags, avalue;
{
#ifdef __i386__
	if ( calibrate_convention_yield_0 ) {
		if (val == expected_val ) {
			yield_0_fastcall = 0;
		} else {
			yield_0_fastcall = 1;
		}
		calibrate_convention_yield_0 = 0;
		return Qnil;
	}
#endif

	last_avalue = avalue;
	return rb_yield_0_i(val,self,klass,flags,avalue);

}

void* hook_rb_yield_0(void* fake_function) {
	int inst_size = get_instructions_size(rb_yield_0_original, 256);
	return put_jmp_hook(rb_yield_0_original, fake_function, inst_size);

}

VALUE handling(VALUE exp, VALUE unused) {
return Qnil;
}

VALUE convention_detection_rescue_(VALUE unused ) {

calibrate_convention_yield_0 = 1;
expected_val = rb_ary_new3(1,LONG2FIX(333));
rb_yield_splat(expected_val);

return Qnil;
}

void init_rb_yield_fake() {

	void* handle = dlopen(current_libruby(),0x101);
	char* rb_funcall = (char*)dlsym(handle, "rb_funcall");
	Dl_info info;
	dladdr(rb_funcall, &info);

	unsigned char* base = (unsigned char*)info.dli_fbase;

	rb_yield_0_original = ruby_resolv(base, "rb_yield_0" );
	rb_yield_0_copy = (RBYIELD0)hook_rb_yield_0(rb_yield_0_fake);

	rb_rescue( convention_detection_rescue_, Qnil, handling, Qnil);

}

#endif

