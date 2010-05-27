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
#include <node_defs.h>
#endif

#include "ruby_symbols.h"
#include "distorm.h"

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>


VALUE rb_cNode;
ID intern_owner;
ID intern_name;
ID intern_sym;

///  from eval.c
#ifdef RUBY1_8
struct METHOD {
    VALUE klass, rklass;

    VALUE recv;
    ID id, oid;
    int safe_level;
    NODE *body;
};
unsigned char* base__;

#define nd_file(n) n->nd_file

#endif

#ifdef RUBY1_9

// from proc.c
struct METHOD {
    VALUE oclass;		/* class that holds the method */
    VALUE rclass;		/* class of the receiver */
    VALUE recv;
    ID id, oid;
    NODE *body;
};

typedef struct rb_iseq_struct__ {
    VALUE type;          // instruction sequence type
    VALUE name;	         // String: iseq name
    VALUE filename;      // file information where this sequence from
} rb_iseq_t__;


typedef VALUE (*MNEW)(VALUE klass, VALUE obj, ID id, VALUE mclass, int scope);
MNEW mnew_;
unsigned char* base__;

#endif

/*
	The node that acts as body of the method
*/
VALUE rb_method_body(VALUE self) {

//	VALUE name = rb_funcall(self, intern_name, 0);
//	VALUE sym = rb_funcall(name, intern_sym, 0);
//	VALUE owner = rb_funcall(self, intern_owner, 0);
//	NODE* body = rb_method_node(owner, SYM2ID(sym) );

	struct METHOD* method;
	Data_Get_Struct(self,struct METHOD,method);

	return Data_Wrap_Struct(rb_cNode, 0, 0, method->body);
}

/*
The number of the line  where the node are defined in the ruby source file
*/
VALUE rb_node_line(VALUE self) {
    NODE* _node;
    Data_Get_Struct(self,NODE,_node);

    #ifdef RUBY1_8
    return INT2FIX(nd_line(_node));
    #endif

    #ifdef RUBY1_9
    return INT2FIX(0);
    #endif
}

/*
The name of the ruby source file where the node are defined
*/
VALUE rb_node_file(VALUE self) {
    NODE* _node;
    Data_Get_Struct(self,NODE,_node);

	#ifdef RUBY1_8
    if (nd_file(_node)  == NULL ) {
	    return rb_str_new2("");
    }
    return rb_str_new2(nd_file(_node) );
    #endif

    #ifdef RUBY1_9

	if (nd_type(_node) == RUBY_VM_METHOD_NODE) {
		VALUE iseqval = (VALUE)(_node->nd_body);
		rb_iseq_t__* ptr;
		Data_Get_Struct(iseqval, rb_iseq_t__, ptr);

		return ptr->filename;
	}

	return rb_str_new2("");

	#endif

}

#ifdef RUBY1_9
// from proc.c
static void
rb_print_undef(klass, id, unused)
    VALUE klass;
    ID id;
    int unused;
{
    rb_name_error(id, "undefined method `%s' for %s `%s'",
		  rb_id2name(id),
		  (TYPE(klass) == T_MODULE) ? "module" : "class",
		  rb_class2name(klass));
}



static VALUE
my_mnew(VALUE klass, VALUE obj, ID id, VALUE mclass, int scope)
{
    VALUE method;
    NODE *body;
    struct METHOD *data;
    VALUE rclass = klass;
    ID oid = id;

  again:
    if ((body = rb_get_method_body(klass, id, 0)) == 0) {
	rb_print_undef(rclass, oid, 0);
    }
//    if (scope && (body->nd_noex & NOEX_MASK) != NOEX_PUBLIC) {
//	rb_print_undef(rclass, oid, (body->nd_noex & NOEX_MASK));
//    }

    klass = body->nd_clss;
    body = body->nd_body;

    if (nd_type(body) == NODE_ZSUPER) {
	klass = RCLASS_SUPER(klass);
	goto again;
    }

    while (rclass != klass &&
	   (FL_TEST(rclass, FL_SINGLETON) || TYPE(rclass) == T_ICLASS)) {
	rclass = RCLASS_SUPER(rclass);
    }
    if (TYPE(klass) == T_ICLASS)
	klass = RBASIC(klass)->klass;

	// FIXME: add valid bm_mark as in the source in proc.c
    method = Data_Make_Struct(mclass, struct METHOD, 0, -1, data);
    data->oclass = klass;
    data->recv = obj;

    data->id = id;
    data->body = body;
    data->rclass = rclass;
    data->oid = oid;
    OBJ_INFECT(method, klass);

    return method;
}

#endif

#ifdef RUBY1_8

// from eval.c
static void
_print_undef(klass, id)
    VALUE klass;
    ID id;
{
    rb_name_error(id, "undefined method `%s' for %s `%s'",
		  rb_id2name(id),
		  (TYPE(klass) == T_MODULE) ? "module" : "class",
		  rb_class2name(klass));
}


static VALUE
my_mnew(klass, obj, id, mklass)
    VALUE klass, obj, mklass;
    ID id;
{
    VALUE method;
    NODE *body;
    struct METHOD *data;
    VALUE rklass = klass;
    ID oid = id;

  again:
    if ((body = rb_method_node(klass, id)) == 0) {
	_print_undef(rklass, oid);
    }

    if (nd_type(body) == NODE_ZSUPER) {
	klass = RCLASS(klass)->super;
	goto again;
    }

    while (rklass != klass &&
	   (FL_TEST(rklass, FL_SINGLETON) || TYPE(rklass) == T_ICLASS)) {
	rklass = RCLASS(rklass)->super;
    }

	// FIXME :     method = Data_Make_Struct(mklass, struct METHOD, bm_mark, free, data);
    method = Data_Make_Struct(mklass, struct METHOD, 0, free, data);
    data->klass = klass;
    data->recv = obj;
    data->id = id;
    data->body = body;
    data->rklass = rklass;
    data->oid = oid;
    data->safe_level = 4;
    OBJ_INFECT(method, klass);

    return method;

}

#endif

/*
Overload of Object#method

With one argument, has the same behaviour as the original Object#method
With two argument adds the possibility of specify the class. example:

	class X
		def foo
		end
	end
	class X < Y
		def foo
		end
	end

	y = Y.new

	y.method(X, :foo) # returns the method foo implemented in X
	x.method(Y, :foo) # returns the method foo implemented in Y
*/
VALUE
rb_obj_method_(int argc,VALUE* argv, VALUE obj ) {
    ID mid;
    VALUE klass;
	VALUE method_id;

    if (argc <= 0) {
		rb_raise(rb_eArgError, "no method name/id given");
    }

    if (argc > 2) {
		rb_raise(rb_eArgError, "too many arguments given");
    }

    if (argc == 1) {

    	return rb_call_super(argc, argv);

//    	method_id = argv[0];
  //  	klass = CLASS_OF(obj);
    }

    if (argc == 2) {
    	klass = argv[0];
    	method_id = argv[1];
    }

	if (rb_obj_is_kind_of(method_id,rb_cSymbol) ) {
		mid = rb_to_id(method_id);
	} else {
		mid = FIX2LONG(method_id);
	}

#ifdef RUBY1_8
    return my_mnew(klass, obj, mid, rb_cMethod);
#endif
#ifdef RUBY1_9
    return my_mnew(klass, obj, mid, rb_cMethod,Qfalse);
#endif

}

void  init_node() {
	rb_define_method(rb_cMethod, "body", rb_method_body,0);

	/*
	The class Node represents the internal ruby node, a node is a piece of ruby code used
	for represent ruby methods in memory and other executable entities, many nodes come from
	the ruby source code and may be associated with a file and line where thats node are defined
	*/
	rb_cNode = rb_define_class("Node", rb_cObject);

	rb_define_method(rb_cNode, "line", rb_node_line, 0);
	rb_define_method(rb_cNode, "file", rb_node_file, 0);

	intern_owner = rb_intern("owner");
	intern_name = rb_intern("name");
	intern_sym = rb_intern("to_sym");

	void* handle = dlopen(current_libruby(),0x101);
	char* rb_funcall = (char*)dlsym(handle, "rb_funcall");
	Dl_info info;
	dladdr(rb_funcall, &info);

#ifdef RUBY1_9
	base__ = (unsigned char*)info.dli_fbase;
	mnew_ = ruby_resolv((unsigned char*)base__, "mnew");
#endif

	rb_define_method(rb_cObject, "method", rb_obj_method_, -1);
}
