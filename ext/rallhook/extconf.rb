require 'mkmf'
dir_config('rallhook')
CONFIG['CC'] = 'gcc'

ruby_version = ARGV[0]
unless ruby_version
ruby_version = Config::CONFIG["ruby_version"]
end

if ruby_version == "1.8"
	$CFLAGS = $CFLAGS + " -DRUBY1_8"
elsif ruby_version == "1.9"
	$CFLAGS = $CFLAGS + " -DRUBY1_9"
else
	print "ERROR: unknown ruby version: #{ruby_version}\n"
	print "try passing the rubyversion by argument (1.8 or 1.9)\n"
end

create_makefile('rallhook')

    
