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

VALUE rb_cRallHook;
ID id_call_;

void unprotect(void* ptr) {
	unsigned long int mask = 0xFFFFFFFFFFF00000;
	int ret = mprotect( (void*) ( ( (unsigned long int)ptr ) & mask ), 0x100000, PROT_READ | PROT_WRITE | PROT_EXEC);
}


VALUE unhook(VALUE self) {

//	rb_hook_proc = Qnil;
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


VALUE from(VALUE self, VALUE num) {
	hook_enable_left = FIX2INT(num)+1;
	return self;
}

VALUE reyield(VALUE argument, VALUE args ) {
	hook_enabled = 0;
	VALUE  ret = rb_yield(argument);
	hook_enabled = 1;
	return ret;
}

static VALUE
rb_f_send_copy(argc, argv, recv)
    int argc;
    VALUE *argv;
    VALUE recv;
{
    VALUE vid;

    if (argc == 0) rb_raise(rb_eArgError, "no method name given");

    vid = *argv++; argc--;

	ID mid;

	if (rb_obj_is_kind_of(vid,rb_cSymbol) ) {
		mid = rb_to_id(vid);
	} else {
		mid = FIX2LONG(vid);
	}


	VALUE ret =  rb_block_call(recv, mid, argc, argv, reyield, Qnil);

    return ret;
}

VALUE _hooked(VALUE self) {
	rb_ivar_set(self,"@_hooked", Qtrue);
	return self;
}

VALUE _hooked_p(VALUE self) {
	return rb_ivar_get(self,"@_hooked");
}

VALUE rehook_reyield_ensure( VALUE arguments) {
	hook_enabled = 1;
	rb_yield(arguments);
}

VALUE restore_unhook_status( VALUE unused) {
	hook_enabled = 0;
}

VALUE rehook_reyield( VALUE arguments, VALUE args) {
	return rb_ensure(rehook_reyield_ensure, arguments, restore_unhook_status, Qnil);
}

VALUE rallhook_call(VALUE self, VALUE klass, VALUE recv, VALUE sym, VALUE args, VALUE mid){
	if (rb_block_given_p() ) {
		VALUE argv[6] = {klass, recv, sym, args, mid};
		return rb_block_call(rb_hook_proc, id_call_, 5, argv, rehook_reyield, Qnil );
	} else {
		return rb_funcall(rb_hook_proc, id_call_, 5, klass, recv, sym, args, mid );
	}
}



void Init_rallhook() {

	const char* initcode = 	"require 'rubygems'\n"
						"require 'ruby-cymbol'\n";

	rb_eval_string(initcode);

	typedef VALUE (*RBHOOK)(VALUE self, ...);
	rb_cRallHook = rb_define_class("RallHook", rb_cObject);
	rb_define_method(rb_cRallHook, "hook", (RBHOOK*)(hook), 1);
	rb_define_method(rb_cRallHook, "unhook", (RBHOOK*)(unhook), 0);
	rb_define_method(rb_cRallHook, "from", (RBHOOK*)(from), 1);

	rb_define_singleton_method(rb_cRallHook, "call", (RBHOOK*)(rallhook_call), 5);


	rb_define_method(rb_cObject, "_hooked?", (RBHOOK*)(_hooked_p ), 0);
	rb_define_method(rb_cObject, "_hooked", (RBHOOK*)(_hooked), 0);

	rb_define_method(rb_cObject, "hooked_send", (RBHOOK*)(rb_f_send_copy), -1);

	rb_call_fake_init();

	id_call_ = rb_intern("call");
/*

*/
}

