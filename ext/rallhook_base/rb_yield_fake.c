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

#ifdef __i386__
VALUE expected_val;
int calibrate_convention_yield_0 = 0;
int yield_0_fastcall = 0;
#endif

int write_eax(int value) {
	return value;
}

static VALUE rb_yield_0_i(VALUE val, VALUE self, VALUE klass, int flags, int avalue)
{
	last_avalue = avalue;

#ifdef __i386__
	if (yield_0_fastcall == 1) {

		int array[6] = {val, self, klass, flags, avalue };
		write_eax(array);

		__asm__("push %ebp\n");	// save all registers
		__asm__("push %esi\n");
		__asm__("push %edi\n");
		__asm__("push %ebx\n");
		__asm__("push %edx\n");
		__asm__("push %ecx\n");
		__asm__("mov 0x4(%eax), %edx\n"); // self
		__asm__("mov 0x8(%eax), %ecx\n"); // klass
		__asm__("push 0x10(%eax)\n"); //avalue
		__asm__("push 0x0c(%eax)\n"); //flags
		__asm__("mov (%eax), %eax\n"); // val
		__asm__("call *rb_yield_0_copy\n");
		__asm__("add $0x8, %esp\n");
		__asm__("pop %ecx\n");
		__asm__("pop %edx\n");
		__asm__("pop %ebx\n");
		__asm__("pop %edi\n");
		__asm__("pop %esi\n");
		__asm__("pop %ebp\n");
		return read_eax();
	} else if (yield_0_fastcall ==  2) {
		int array[6] = {val, self, klass, flags, avalue };
		write_eax(array);

		__asm__("push %ebp\n");	// save all registers
		__asm__("push %esi\n");
		__asm__("push %edi\n");
		__asm__("push %ebx\n");
		__asm__("push %edx\n");
		__asm__("push %ecx\n");
		__asm__("mov 0x4(%eax), %edx\n");
		__asm__("push 0x10(%eax)\n");
		__asm__("push 0x0c(%eax)\n");
		__asm__("push 0x08(%eax)\n");
		__asm__("mov (%eax), %eax\n");
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
#endif
		return rb_yield_0_copy(val,self,klass,flags,avalue);
#ifdef __i386__
	}
#endif

}

#ifdef __i386__
     #define _WORD int
    _WORD eax, edx, ecx, eip;

static VALUE
rb_yield_0_fake_regs(_WORD eax, _WORD edx, _WORD ecx, _WORD* esp)
{
	// , val, self, klass, flags, avalue
	esp++;
	if ( calibrate_convention_yield_0 ) {
		if (esp[0] == expected_val ) {
			yield_0_fastcall = 0;
		} else if ( (VALUE)eax == expected_val ){

			if (edx == 0 && ecx == 0 && esp[0] == 0 && (esp[1] == 2 || esp[1] == 0) ) {
				yield_0_fastcall = 1;
			} else if ( edx == 0 && esp[0] == 0 && esp[1] == 0 && (esp[2] == 2 || esp[2] == 0) ){
				yield_0_fastcall = 2;
			} else {
				yield_0_fastcall = 1;
			}
		}

		calibrate_convention_yield_0 = 0;
		return Qnil;
	}

	if (yield_0_fastcall == 1) {
		return rb_yield_0_i((VALUE)eax,(VALUE)edx,(VALUE)ecx,(int)esp[0],(int)esp[1]);
	} else if (yield_0_fastcall == 2){
		return rb_yield_0_i((VALUE)eax,(VALUE)edx,(VALUE)esp[0],(int)esp[1],(int)esp[2]);
	} else {
		return rb_yield_0_i((VALUE)esp[0],(VALUE)esp[1],(VALUE)esp[2],(int)esp[3],(int)esp[4]);
	}

}
#endif

#ifdef __x86_64__
static VALUE
rb_yield_0_fake(VALUE val,VALUE self,VALUE klass,int flags,int avalue) {
	last_avalue = avalue;
	return rb_yield_0_copy(val,self,klass,flags,avalue);
}

#endif


void* hook_rb_yield_0(void* fake_function) {
	int inst_size = get_instructions_size(rb_yield_0_original, 256);
	return put_jmp_hook_with_regs(rb_yield_0_original, fake_function, inst_size);

}

#ifdef __i386__
VALUE handling(VALUE exp, VALUE unused) {
return Qnil;
}

VALUE convention_detection_rescue_(VALUE unused ) {

calibrate_convention_yield_0 = 1;
expected_val = rb_ary_new3(1,LONG2FIX(333));
rb_yield_splat(expected_val);

return Qnil;
}
#endif

void init_rb_yield_fake() {

	void* handle = dlopen(current_libruby(),0x101);
	char* rb_funcall = (char*)dlsym(handle, "rb_funcall");
	Dl_info info;
	dladdr(rb_funcall, &info);

	unsigned char* base = (unsigned char*)info.dli_fbase;

	rb_yield_0_original = ruby_resolv(base, "rb_yield_0" );
#ifdef __i386__
	rb_yield_0_copy = (RBYIELD0)hook_rb_yield_0(rb_yield_0_fake_regs);
#elif __x86_64__
	int inst_size = get_instructions_size(rb_yield_0_original, 256);
	rb_yield_0_copy = (RBYIELD0)put_jmp_hook(rb_yield_0_original, rb_yield_0_fake, inst_size);
#endif

#ifdef __i386__
	rb_rescue( convention_detection_rescue_, Qnil, handling, Qnil);
#endif

}

#endif

