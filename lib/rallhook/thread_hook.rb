=begin

This file is part of the rallhook project, http://github.com/tario/

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

=end
class Thread

  class << self
    alias :original_new :new

    private :original_new

    def new
      parent = current
      original_new do
        parent.acquire_attributes do
          yield
        end
      end
    end
  end
end