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
#include "ruby_symbols.h"
#include "rb_call_fake.h"
#include <sys/mman.h>

VALUE rb_mRallHook;
ID id_send;

void unprotect(void* ptr) {
	unsigned long int mask = 0xFFFFFFFFFFF00000;
	int ret = mprotect( (void*) ( ( (unsigned long int)ptr ) & mask ), 0x100000, PROT_READ | PROT_WRITE | PROT_EXEC);
}


VALUE unhook(VALUE self) {

	rb_hook_proc = Qnil;
	hook_enabled = 0;

	return Qnil;
}


VALUE hook(VALUE self, VALUE hook_proc) {
	rb_hook_proc = hook_proc;
	hook_enabled = 1;

	// insert inconditional jmp from rb_call to rb_call_copy
	typedef unsigned char uchar;

	uchar* p = (uchar*)rb_call_original;
	//x86_64 inconditional jump
	unprotect(p);

	p[0] = 0x48; // movl XXX, %rax
	p[1] = 0xb8;

	void** address = (void**)(p+2);

	p[10] = 0xff; // jmp %rax
	p[11] = 0xe0;

	*address = &rb_call_fake;

	if (rb_block_given_p() ) {
		rb_yield(Qnil);

		unhook(self);
	}

	return Qnil;
}


VALUE restore_unhook_status_ensure(VALUE unused) {
	hook_enabled = 1;
}

VALUE rb_call_copy_not_ensure(VALUE ary) {

    int argc;
    VALUE *argv;
    VALUE recv;

	argc = (int)rb_ary_entry(ary,0);
	argv = (VALUE*)rb_ary_entry(ary,1);
	recv = rb_ary_entry(ary,2);

    VALUE vid;
    if (argc == 0) rb_raise(rb_eArgError, "no method name given");

    vid = *argv++; argc--;

	ID mid;

	if (rb_obj_is_kind_of(vid,rb_cSymbol) ) {
		mid = rb_to_id(vid);
	} else {
		mid = FIX2LONG(vid);
	}

    return rb_call_copy(CLASS_OF(recv), recv, mid, argc, argv, 1, Qundef);

}

static VALUE
rb_f_send_copy(argc, argv, recv)
    int argc;
    VALUE *argv;
    VALUE recv;
{

//    PUSH_ITER(rb_block_given_p()?ITER_PRE:ITER_NOT);

	VALUE ary = rb_ary_new2(3);
	rb_ary_store(ary,0,(VALUE)argc);
	rb_ary_store(ary,1,(VALUE)argv);
	rb_ary_store(ary,1,(VALUE)recv);

	hook_enabled = 1;
	return rb_ensure( rb_call_copy_not_ensure, ary, restore_unhook_status_ensure, Qnil);

//    POP_ITER();

}

void Init_rallhook() {

	const char* initcode = 	"require 'rubygems'\n"
						"require 'ruby-cymbol'\n";

	rb_eval_string(initcode);

	typedef VALUE (*RBHOOK)(VALUE self, ...);
	VALUE rb_mRallHook = rb_define_module("RallHook");
	rb_define_singleton_method(rb_mRallHook, "hook", (RBHOOK*)(hook), 1);
	rb_define_singleton_method(rb_mRallHook, "unhook", (RBHOOK*)(unhook), 0);

	rb_define_method(rb_cObject, "hooked_send", (RBHOOK*)(rb_f_send_copy), -1);

	rb_call_fake_init();

	id_send = rb_intern("send");
/*

*/
}

