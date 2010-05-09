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
#include "method_node.h"
#include <sys/mman.h>
#include <tag_container.h>

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

VALUE reunhook_reyield_ensure( VALUE arguments) {
	hook_enabled = 0;
	rb_yield(arguments);
}

VALUE restore_hook_status( VALUE unused) {
	hook_enabled = 1;
}

VALUE restore_unhook_status( VALUE unused) {
	hook_enabled = 0;
}


VALUE reunhook_reyield( VALUE arguments, VALUE args) {
	return rb_ensure(reunhook_reyield_ensure, arguments, restore_hook_status, Qnil);
}

VALUE ensured_recall( VALUE arguments ) {
	VALUE* vect = (VALUE*)arguments;
	VALUE klass = vect[0];
	VALUE recv = vect[1];
	ID mid = (ID)vect[2];
	int argc = (int)vect[3];
	VALUE* argv = (VALUE*)vect[4];

	if (rb_block_given_p() ) {
		return rb_block_call(recv, mid, argc, argv, reunhook_reyield, Qnil );
	} else {
		return rb_call_fake(klass,recv,mid,argc,argv,1,Qnil);
	}


}

static VALUE
rb_f_send_copy(argc, argv, recv)
    int argc;
    VALUE *argv;
    VALUE recv;
{
    VALUE vid;
    VALUE klass;

    if (argc < 1) rb_raise(rb_eArgError, "no method name given");



    klass = *argv++; argc--;
    vid = *argv++; argc--;

	ID mid;

	if (rb_obj_is_kind_of(vid,rb_cSymbol) ) {
		mid = rb_to_id(vid);
	} else {
		mid = FIX2LONG(vid);
	}

	recv = tag_container(recv,klass);

	hook_enabled = 1;
	hook_enable_left = 1;

	VALUE args[7] = {klass,recv,(VALUE)mid,(VALUE)argc,(VALUE)argv};

	return rb_ensure(ensured_recall, (VALUE)args, restore_unhook_status, Qnil);
}

VALUE rehook_reyield_ensure( VALUE arguments) {
	hook_enabled = 1;
	rb_yield(arguments);
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

	rb_define_method(rb_cObject, "hooked_send", (RBHOOK*)(rb_f_send_copy), -1);

	rb_call_fake_init();
	init_node();
	init_tag_container();

	id_call_ = rb_intern("call");
/*

*/
}

