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

