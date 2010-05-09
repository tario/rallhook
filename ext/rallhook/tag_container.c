/*

This file is part of the rallhook project, http//github.com/tario/rallhook

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
#include "tag_container.h"

VALUE rb_cTagContainer;
ID internal_new;

VALUE tag_container(VALUE self, VALUE tag) {
	VALUE ret = rb_funcall(rb_cTagContainer, internal_new, 0);

	rb_ivar_set( ret, "@self_", self);
	rb_ivar_set( ret, "@tag", tag);

	return ret;
}
VALUE tag_container_get_self(VALUE tag) {
	return rb_ivar_get( tag, "@self_");
}
VALUE tag_container_get_tag(VALUE tag) {
	return rb_ivar_get( tag, "@tag");
}

int is_tag(VALUE tag) {
	return rb_obj_is_kind_of(tag,rb_cTagContainer );
}



void init_tag_container() {

	rb_cTagContainer = rb_define_class("TagContainer", rb_cObject);

	rb_define_method( rb_cTagContainer, "self_", tag_container_get_self, 0);
	rb_define_method( rb_cTagContainer, "tag", tag_container_get_tag, 0);

	internal_new = rb_intern("new");

}