require 'mkmf'
dir_config('rallhook_base')
CONFIG['CC'] = 'gcc'

ruby_version = Config::CONFIG["ruby_version"]
ruby_version = ruby_version.split(".")[0..1].join(".")

if ruby_version == "1.8"
	$CFLAGS = $CFLAGS + " -DRUBY1_8"
elsif ruby_version == "1.9"
	$CFLAGS = $CFLAGS + " -DRUBY1_9"
else
	print "ERROR: unknown ruby version: #{ruby_version}\n"
	print "try passing the rubyversion by argument (1.8 or 1.9)\n"
end

$CFLAGS = $CFLAGS  + " -o $@"

srcdir = '.'

$objs = []
srcs = Dir[File.join(srcdir, "*.{#{SRC_EXT.join(%q{,})}}")]
srcs += Dir[File.join(srcdir, "deps/distorm/*.c")]
srcs += Dir[File.join(srcdir, "deps/distorm/src/*.c")]

for f in srcs
obj = f[2..-1].gsub(/\.c$/, ".o")
$objs.push(obj) unless $objs.index(obj)
end

create_makefile('rallhook_base')



