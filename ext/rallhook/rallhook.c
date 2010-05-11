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
#include "hook.h"
#include "method_node.h"
#include <tag_container.h>

VALUE rb_cRallHook;
ID id_call_;

VALUE unhook(VALUE self) {
	hook_enabled = 0;
	return Qnil;
}

RBCALL rb_call_copy;


int code_changed = 0;

VALUE hook(VALUE self, VALUE hook_proc) {
	rb_hook_proc = hook_proc;
	hook_enabled = 1;

	if (!code_changed) {
		// insert inconditional jmp from rb_call to rb_call_copy
		int replaced = 0;
		if (memcmp(rb_call_original, "\x48\x89\x5c\x24\xd0\x4c\x89\x64\x24\xe0\x48\x89\xd3" ,13)==0) {
			replaced = 1;
			rb_call_copy = (RBCALL)put_jmp_hook(rb_call_original, rb_call_fake, 13);
		}

		if (!replaced) {
			rb_raise( rb_eFatal, "libruby incompatible with rallhook");
		}

		code_changed = 1;
	}

	if (rb_block_given_p() ) {
		return rb_ensure(rb_yield, Qnil, unhook, self);
	}

	return Qnil;
}


VALUE from(VALUE self, VALUE num) {
	hook_enable_left = FIX2INT(num)+1;
}

VALUE reunhook_reyield_ensure( VALUE arguments) {
	hook_enabled = 0;
	rb_yield_splat(arguments);
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

	if (rb_block_given_p() ) {
		recv = tag_container(recv,klass);
	}

	hook_enabled = 1;
	hook_enable_left = 1;

	VALUE args[7] = {klass,recv,(VALUE)mid,(VALUE)argc,(VALUE)argv};

	return rb_ensure(ensured_recall, (VALUE)args, restore_unhook_status, Qnil);
}


VALUE get_rb_yield_0_avalue() {
	__asm__("mov %r8, %rax");
}


VALUE rehook_reyield_ensure( VALUE arguments) {
	if (! (get_rb_yield_0_avalue()==Qtrue) ) {
		arguments = rb_ary_new3(1,arguments);
	}

	hook_enabled = 1;
	rb_yield_splat(arguments);
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

