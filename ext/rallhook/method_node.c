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
#include <ruby_version.h>

#ifdef RUBY1_8
#include <node.h>
#endif

#ifdef RUBY1_9
#include <ruby/node.h>
#endif

VALUE rb_cNode;
ID intern_owner;
ID intern_name;
ID intern_sym;

///  from eval.c

struct METHOD {
    VALUE klass, rklass;

    VALUE recv;
    ID id, oid;
    int safe_level;
    NODE *body;
};


VALUE rb_method_body(VALUE self) {

//	VALUE name = rb_funcall(self, intern_name, 0);
//	VALUE sym = rb_funcall(name, intern_sym, 0);
//	VALUE owner = rb_funcall(self, intern_owner, 0);
//	NODE* body = rb_method_node(owner, SYM2ID(sym) );

	struct METHOD* method;
	Data_Get_Struct(self,struct METHOD,method);

	return Data_Wrap_Struct(rb_cNode, 0, 0, method->body);
}

VALUE rb_node_line(VALUE self) {
    NODE* _node;
    Data_Get_Struct(self,NODE,_node);
    return INT2FIX(nd_line(_node));
}

VALUE rb_node_file(VALUE self) {
    NODE* _node;
    Data_Get_Struct(self,NODE,_node);

    if (_node->nd_file  == NULL ) {
	    return rb_str_new2("");
    }

    return rb_str_new2(_node->nd_file );
}

VALUE rb_node_method_body(VALUE self, VALUE klass, VALUE method_id) {

    NODE* _node;
    ID mid;

	if (rb_obj_is_kind_of(method_id,rb_cSymbol) ) {
		mid = rb_to_id(method_id);
	} else {
		mid = FIX2LONG(method_id);
	}
	_node = rb_method_node(klass, mid);

	if (_node == NULL) {
		return Qnil;
	}

	return Data_Wrap_Struct(rb_cNode, 0, 0, _node);

}

VALUE rb_class_method_body(VALUE self, VALUE method_id) {

    NODE* _node;
    ID mid;

	if (rb_obj_is_kind_of(method_id,rb_cSymbol) ) {
		mid = rb_to_id(method_id);
	} else {
		mid = FIX2LONG(method_id);
	}

	_node = rb_method_node(self, mid);

	if (_node == NULL) {
		return Qnil;
	}

	return Data_Wrap_Struct(rb_cNode, 0, 0, _node);
}


void  init_node() {
	rb_define_method(rb_cMethod, "body", rb_method_body,0);

	rb_cNode = rb_define_class("Node", rb_cObject);

	rb_define_method(rb_cNode, "line", rb_node_line, 0);
	rb_define_method(rb_cNode, "file", rb_node_file, 0);

	rb_define_singleton_method(rb_cNode, "method_node", rb_node_method_body, 2);
	rb_define_singleton_method(rb_cClass, "method_node", rb_class_method_body, 1);

	intern_owner = rb_intern("owner");
	intern_name = rb_intern("name");
	intern_sym = rb_intern("to_sym");

}
