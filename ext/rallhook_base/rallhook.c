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
#include "method_node.h"
#include "ruby_redirect.h"

VALUE rb_cHook;
VALUE rb_mRallHook;
VALUE rb_mMethodRedirect;
VALUE rb_mMethodReturn;
ID id_call_;

VALUE rb_hook_proc = Qnil;

VALUE unhook(VALUE self) {
	disable_redirect();
	return Qnil;
}

ID id_call;
ID id_method_wrapper;
ID id_handle_method;

ID id_return_value_var, id_klass_var, id_recv_var, id_method_var, id_unhook_var;


typedef struct rb_thread_struct
{
    VALUE self;
	void *vm; // rb_vm_t_

    /* execution information */
    VALUE *stack;		/* must free, must mark */
    unsigned long stack_size;
    void *cfp; // rb_control_frame_t_
    int safe_level;
    int raised_flag;
    VALUE last_status; /* $? */

    /* passing state */
    int state;

    /* for rb_iterate */
    void *passed_block; // rb_block_t_

    // ...
} rb_thread_t__;

VALUE ensured_handle_method( VALUE params ) {
	return rb_funcall2( rb_hook_proc, id_handle_method, 5, (VALUE*)params );
}

VALUE restore_hook_status(VALUE unused) {
	enable_redirect();
	return Qnil;
}
extern int hook_enabled;
extern int hook_enable_left;
extern REDIRECTHANDLER current_redirect_handler;

void rallhook_redirect_handler ( CallData* call_data ) {

	VALUE sym;

	if (hook_enabled == 0 || hook_enable_left > 0){
		if(hook_enable_left>0) hook_enable_left --;
		return;
	}

	// avoid to send symbols without name (crash the interpreter)
	if (rb_id2name(call_data->mid) == NULL){
		sym = Qnil;
	} else {
		sym = ID2SYM(call_data->mid);
	}


	VALUE argv_[6];
	argv_[0] = call_data->klass;
	argv_[1] = call_data->recv;
	argv_[2] = sym;
	argv_[3] = call_data->args;
	argv_[4] = LONG2FIX(call_data->mid);

	disable_redirect();

	rb_thread_t__* th;
	VALUE current_thread = rb_thread_current();
	Data_Get_Struct( current_thread, rb_thread_t__, th );

	void* blockptr = th->passed_block;
	VALUE result = rb_ensure(ensured_handle_method,(VALUE)argv_,restore_hook_status,Qnil);

	th->passed_block = blockptr;

	if (rb_obj_is_kind_of(result,rb_mMethodRedirect) == Qtrue ) {

		call_data->klass = rb_ivar_get(result,id_klass_var );
		call_data->recv = rb_ivar_get(result,id_recv_var );
		call_data->mid = rb_to_id( rb_ivar_get(result,id_method_var) );

		if (rb_ivar_get(result,id_unhook_var) != Qnil ) {
			disable_redirect();
		}

	}

}

VALUE hook(VALUE self, VALUE hook_proc) {
	rb_hook_proc = hook_proc;
	put_redirect_handler( rallhook_redirect_handler );

	enable_redirect();

	if (rb_block_given_p() ) {
		return rb_ensure(rb_yield, Qnil, unhook, self);
	}

	return Qnil;
}


VALUE from(VALUE self, VALUE num) {
	redirect_left(FIX2INT(num)+1);
	return self;
}

VALUE rehook(VALUE unused) {
	enable_redirect();
	if (rb_block_given_p() ) {
		return rb_ensure(rb_yield, Qnil, unhook, Qnil);
	}
	return Qnil;
}

extern void Init_rallhook_base() {

	const char* initcode = 	"require 'rubygems'\n"
						"require 'ruby-cymbol'\n";

	rb_eval_string(initcode);
	rb_mRallHook = rb_define_module("RallHook");
	rb_cHook = rb_define_class_under(rb_mRallHook, "Hook", rb_cObject);

	rb_define_singleton_method( rb_cHook, "rehook", rehook, 0 );

	rb_define_method(rb_cHook, "hook", hook, 1);
	rb_define_method(rb_cHook, "unhook", unhook, 0);
	rb_define_method(rb_cHook, "from", from, 1);

	rb_mMethodRedirect = rb_define_module_under(rb_mRallHook, "MethodRedirect");

	init_node();

	id_call_ = rb_intern("call");

	id_call = rb_intern("call");
	id_method_wrapper = rb_intern("method_wrapper");
	id_handle_method = rb_intern("handle_method");
	id_return_value_var = rb_intern("@return_value");
	id_klass_var = rb_intern("@klass");
	id_recv_var = rb_intern("@recv");
	id_method_var = rb_intern("@method");
	id_unhook_var = rb_intern("@unhook");
/*

*/
}

