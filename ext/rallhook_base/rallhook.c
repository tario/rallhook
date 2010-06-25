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
#include "restrict_def.h"

VALUE rb_cHook;
VALUE rb_mRallHook;
VALUE rb_mMethodRedirect;
VALUE rb_mMethodReturn;
ID id_call_;

ID id_call;
ID id_method_wrapper;
ID id_handle_method;
ID id_binding;
ID id_method_added;
ID id_hook_enabled;
ID id_hook_enable_left;
ID id_hook_proc;

ID id_return_value_var, id_klass_var, id_recv_var, id_method_var, id_unhook_var;

void *rb_get_method_body(VALUE klass, ID id, ID *idp);

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

typedef struct AttachedThreadInfo_ {
	int hook_enabled;
	int hook_enable_left;
	VALUE hook_proc;
	int handle_method_arity;
} AttachedThreadInfo;

AttachedThreadInfo* tinfo_from_thread(VALUE thread) {
	VALUE tmp = rb_ivar_get( thread, rb_intern("__tinfo") );

	if (tmp == Qnil) {
		AttachedThreadInfo* tinfo = malloc(sizeof(AttachedThreadInfo));
		tinfo->hook_enabled = 0;
		tinfo->hook_enable_left = 0;
		tinfo->hook_proc = Qnil;

		VALUE tinfo_obj = Data_Wrap_Struct(rb_cObject, 0, 0,tinfo);

		rb_ivar_set( thread, rb_intern("__tinfo"), tinfo_obj);

		return tinfo;
	} else {

		AttachedThreadInfo* tinfo;
		Data_Get_Struct(tmp, struct AttachedThreadInfo, tinfo);

		return tinfo;
	}
}

void redirect_left(AttachedThreadInfo* tinfo, int left) {
	tinfo->hook_enable_left = left;
}

void enable_redirect(AttachedThreadInfo* tinfo) {
	tinfo->hook_enabled = 1;
}

void disable_redirect(AttachedThreadInfo* tinfo) {
	tinfo->hook_enabled = 0;
}

int get_hook_enable_left(AttachedThreadInfo* tinfo) {
	return tinfo->hook_enable_left;
}

int get_hook_enabled(AttachedThreadInfo* tinfo) {
	return tinfo->hook_enabled;
}

VALUE get_hook_proc() {
	return tinfo_from_thread( rb_thread_current() )->hook_proc ;
}

/*
Disable the hook. Is not usually necesary because of the RAII feature of Hook#hook
*/
VALUE unhook(VALUE self) {
	disable_redirect(tinfo_from_thread( rb_thread_current() ) );
	return Qnil;
}

VALUE ensured_handle_method( VALUE params ) {
	int argc = (int)((VALUE*)params)[0];
	VALUE* argv = ((VALUE*)params)+1;
	return rb_funcall2( get_hook_proc(), id_handle_method, argc, argv);
}

VALUE restore_hook_status(AttachedThreadInfo* tinfo) {
	enable_redirect(tinfo);
	return Qnil;
}

void set_handle_method_arity(int value) {
	tinfo_from_thread(rb_thread_current() )->handle_method_arity = value;
}

void rallhook_redirect_handler ( VALUE* klass, VALUE* recv, ID* mid ) {

	VALUE current_thread = rb_thread_current();
	AttachedThreadInfo* tinfo = tinfo_from_thread(current_thread);
	int hook_enable_left  = get_hook_enable_left(tinfo);

	if (get_hook_enabled(tinfo) == 0 || hook_enable_left > 0){
		if(hook_enable_left>0) hook_enable_left --;
		redirect_left( tinfo, hook_enable_left );
		return;
	}

	VALUE argv_[6];
	if(*mid == id_method_added) {
		*recv = unshadow(*recv); // recv is THE class
		*klass = CLASS_OF(*recv);
	}

	argv_[0] = tinfo->handle_method_arity;
	if (tinfo->handle_method_arity == 4) {

		VALUE sym;
	    if (rb_id2name(*mid)) {
	    	sym = ID2SYM(*mid);
	    } else {
	    	sym =Qnil;
	    }


		argv_[1] = unshadow(*klass);
		argv_[2] = *recv;
		argv_[3] = sym;
		argv_[4] = LONG2FIX(*mid);
	} else {
		argv_[1] = unshadow(*klass);
		argv_[2] = *recv;
		argv_[3] = LONG2FIX(*mid);
	}

	ID original_id = *mid;

	disable_redirect(tinfo);

	rb_thread_t__* th;
	Data_Get_Struct( current_thread, rb_thread_t__, th );

	void* blockptr = th->passed_block;
	VALUE result = rb_ensure(ensured_handle_method,(VALUE)argv_,restore_hook_status, (VALUE)tinfo);

	th->passed_block = blockptr;

	if (original_id != id_binding ) {

		// method named "binding" cannot be redirected
		if (rb_obj_is_kind_of(result,rb_mMethodRedirect) == Qtrue ) {
			*klass = rb_ivar_get(result,id_klass_var );
			*recv = rb_ivar_get(result,id_recv_var );
			*mid = rb_to_id( rb_ivar_get(result,id_method_var) );

			if (rb_ivar_get(result,id_unhook_var) != Qnil ) {
				disable_redirect(tinfo);
			}

		} else {
			shadow_redirect(klass, recv, mid);
		}
	}

	// methods over class hook are illegal, may change the state of hook
	if (*recv == rb_cHook ) {
		rb_raise(rb_eSecurityError, "Illegal method call: Hook.%s", rb_id2name(*mid) );
	}


}

/*
Activate the hook, it is desirable to use the RAII call to make the hook block exception safe:

	rallhook.hook method_hadler do
		print "hello world\n" # calls to print, write or whatever are intercepted by method_handler#method_handle
	end # in the finish of the block, the hook are disabled

*/
VALUE hook(VALUE self, VALUE hook_proc) {

	tinfo_from_thread( rb_thread_current() )->hook_proc = hook_proc;

	VALUE handle_method_method =rb_obj_method(hook_proc, ID2SYM(id_handle_method) );
	VALUE handle_method_method_arity = rb_funcall( handle_method_method, rb_intern("arity"), 0 );
	tinfo_from_thread( rb_thread_current() )->handle_method_arity = FIX2INT( handle_method_method_arity );

	put_redirect_handler( rallhook_redirect_handler );

	enable_redirect(tinfo_from_thread(rb_thread_current()));

	if (rb_block_given_p() ) {
		return rb_ensure(rb_yield, Qnil, unhook, self);
	}

	return Qnil;
}


/*
Disable the hook in the next N calls and reenable them. Useful to avoid infinite recursion and used
in RallHook::Helper::MethodWrapper
*/
VALUE from(VALUE self, VALUE num) {
	redirect_left(tinfo_from_thread( rb_thread_current() ), FIX2INT(num)+1);
	return self;
}


/*
Re-enable the hook if the hook was disabled and was activated previously with Hook#hook
If no call to Hook#hook has made, rehook does nothing
*/
VALUE rehook(VALUE unused) {
	enable_redirect(tinfo_from_thread( rb_thread_current() ));
	if (rb_block_given_p() ) {
		return rb_ensure(rb_yield, Qnil, unhook, Qnil);
	}
	return Qnil;
}



VALUE thread_restore_attributes(AttachedThreadInfo* tinfo) {

	AttachedThreadInfo* current_tinfo = tinfo_from_thread(rb_thread_current());

	current_tinfo->hook_enabled = tinfo->hook_enabled;
	current_tinfo->hook_enable_left = tinfo->hook_enable_left;
	current_tinfo->hook_proc = tinfo->hook_proc;
	current_tinfo->handle_method_arity = tinfo->handle_method_arity;

	return Qnil;
}

VALUE rb_thread_acquire_attributes( VALUE thread ) {

	AttachedThreadInfo* orig = tinfo_from_thread(thread);
	AttachedThreadInfo* dest = tinfo_from_thread(rb_thread_current() );

	AttachedThreadInfo oldattr;

	oldattr.hook_enabled = dest->hook_enabled;
	oldattr.hook_enable_left = dest->hook_enable_left;
	oldattr.hook_proc = dest->hook_proc;
	oldattr.handle_method_arity = dest->handle_method_arity;

	dest->hook_enabled = orig->hook_enabled;
	dest->hook_enable_left = orig->hook_enable_left;
	dest->hook_proc = orig->hook_proc;
	dest->handle_method_arity = orig->handle_method_arity;

	if (rb_block_given_p() ) {
		return rb_ensure(rb_yield, Qnil, thread_restore_attributes, (VALUE)&oldattr);
	}

	return Qnil;
}


extern void Init_rallhook_base() {

	const char* initcode = 	"require 'rubygems'\n"
						"require 'ruby-cymbol'\n";

	rb_eval_string(initcode);

/*
This module brings together all classes and methods belonging to rallhook with the exception of Node
*/
	rb_mRallHook = rb_define_module("RallHook");
/*
This class handles the hook, enabling and disable it.

Example:

	# ... instanciate method_handler ... (see README and examples)

	RallHook::Hook.hook method_hadler do
		print "hello world\n" # calls to print, write or whatever are intercepted by method_handler#method_handle
	end # in the finish of the block, the hook are disabled

*/

	rb_cHook = rb_define_class_under(rb_mRallHook, "Hook", rb_cObject);

	rb_define_singleton_method(rb_cHook, "rehook", rehook, 0 );
	rb_define_singleton_method(rb_cHook, "hook", hook, 1);
	rb_define_singleton_method(rb_cHook, "unhook", unhook, 0);
	rb_define_singleton_method(rb_cHook, "from", from, 1);

/*
Marker module to indicate messages of method redirection

Example:

	class MyMessage
		include MethodRedirect # indicates that this message is about method redirection

		def initialize(klass, recv, m)
			@klass = klass
			@recv = recv
			@method = m
		end
	end
	class MethodHandler
		class X
			def foo
			end
		end
		def method_handle( ... )
			MyMessage.new(X, x.new, :foo)
		end
	end

Don't use this module, use x.redirect(:foo) instead.

Example:
	class MethodHandler
		class X
			def foo
			end
		end
		def method_handle( ... )
			x.redirect(:foo)
		end
	end
*/
	rb_mMethodRedirect = rb_define_module_under(rb_mRallHook, "MethodRedirect");

	init_node();
	init_restrict_def();
	init_redirect();

	id_call_ = rb_intern("call");

	id_call = rb_intern("call");
	id_method_wrapper = rb_intern("method_wrapper");
	id_handle_method = rb_intern("handle_method");
	id_return_value_var = rb_intern("@return_value");
	id_klass_var = rb_intern("@klass");
	id_recv_var = rb_intern("@recv");
	id_method_var = rb_intern("@method");
	id_unhook_var = rb_intern("@unhook");
	id_binding = rb_intern("binding");
	id_method_added = rb_intern("method_added");
	id_hook_enabled = rb_intern("__hook_enabled");
	id_hook_enable_left = rb_intern("__hook_enable_left");
	id_hook_proc = rb_intern("__hook_proc");

	rb_define_method(rb_cThread, "acquire_attributes", rb_thread_acquire_attributes,0);

}
