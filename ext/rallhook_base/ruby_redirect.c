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
#include "ruby_redirect.h"

REDIRECTHANDLER* current_redirect_handler;

int put_redirect_handler( REDIRECTHANDLER* redirect_handler) {
	current_redirect_handler = redirect_handler;
	return 0;
}

void disable_redirect() {

}

void enable_redirect() {

}

REDIRECTHANDLER* get_current_redirect_handler() {
	return current_redirect_handler;
}


