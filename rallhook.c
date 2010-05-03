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

VALUE rb_mRallHook;
VALUE rb_hook_proc;

VALUE hook(VALUE self, VALUE hook_proc) {
	rb_hook_proc = hook_proc;
	return Qnil;
}

VALUE unhook(VALUE self) {
	rb_hook_proc = Qnil;
	return Qnil;
}

void Init_rallhook() {

	const char* initcode = 	"require 'rubygems'\n"
						"require 'ruby-cymbol'\n";

	rb_eval_string(initcode);

	typedef VALUE (*RBHOOK)(VALUE self, ...);

	VALUE rb_mRallHook = rb_define_module("RallHook");
	rb_define_singleton_method(rb_mRallHook, "hook", (RBHOOK*)(hook), 1);
	rb_define_singleton_method(rb_mRallHook, "unhook", (RBHOOK*)(unhook), 0);


}

