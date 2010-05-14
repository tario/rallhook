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
#include "method_wrapper.h"

VALUE rb_cMethodWrapper;
static ID id_new;

VALUE new_method_wrapper() {
	return rb_funcall(rb_cMethodWrapper, id_new, 0);
}

VALUE method_wrapper_get_klass(VALUE methodw) {
	return rb_ivar_get(methodw,"@klass");
}
VALUE method_wrapper_get_recv(VALUE methodw) {
	return rb_ivar_get(methodw,"@recv");
}

VALUE method_wrapper_get_method_id(VALUE methodw) {
	return rb_ivar_get(methodw,"@recv");
}

VALUE method_wrapper_set_klass(VALUE methodw, VALUE klass) {
	rb_ivar_set(methodw,"@klass", klass);
	return Qnil;
}
VALUE method_wrapper_set_recv(VALUE methodw, VALUE recv) {
	rb_ivar_set(methodw,"@recv", recv);
	return Qnil;
}
void method_wrapper_set_method_id(VALUE methodw, ID method_id) {
	rb_ivar_set(methodw,"@method_id", LONG2FIX(method_id));
}

VALUE method_wrapper_set_method_id_(VALUE methodw, VALUE method_id) {
	rb_ivar_set(methodw,"@method_id", method_id);
	return Qnil;
}

void init_method_wrapper() {
	id_new = rb_intern("new");

	rb_cMethodWrapper = rb_define_class("MethodWrapper", rb_cObject);
	rb_define_method(rb_cMethodWrapper, "klass", method_wrapper_get_klass, 0);
	rb_define_method(rb_cMethodWrapper, "recv", method_wrapper_get_recv, 0);
	rb_define_method(rb_cMethodWrapper, "method_id", method_wrapper_get_method_id, 0);
	rb_define_method(rb_cMethodWrapper, "klass=", method_wrapper_set_klass, 1);
	rb_define_method(rb_cMethodWrapper, "recv=", method_wrapper_set_recv, 1);
	rb_define_method(rb_cMethodWrapper, "method_id=", method_wrapper_set_method_id_, 1);
}
