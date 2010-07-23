require 'mkmf'
dir_config('rallhook_base')
CONFIG['CC'] = 'gcc'

ruby_version = Config::CONFIG["ruby_version"]
ruby_version = ruby_version.split(".")[0..1].join(".")

def distorm

  distorm_names = {
      "/usr/lib/libdistorm3.so" => "distorm3",
      "/usr/local/lib/libdistorm3.so" => "distorm3",
      "/usr/lib/libdistorm64.so" => "distorm64",
      "/usr/local/lib/libdistorm64.so" => "distorm64"
    }

  distorm_names.each do |k,v|
    if File.exists? k then
      return v
    end
  end

  raise "Distorm library not found in the system"
end

$LIBS = $LIBS + " -l#{distorm()}"

if ruby_version == "1.8"
	$CFLAGS = $CFLAGS + " -DRUBY1_8"
elsif ruby_version == "1.9"
	$CFLAGS = $CFLAGS + " -DRUBY1_9"
else
	print "ERROR: unknown ruby version: #{ruby_version}\n"
	print "try passing the rubyversion by argument (1.8 or 1.9)\n"
end

create_makefile('rallhook_base')


