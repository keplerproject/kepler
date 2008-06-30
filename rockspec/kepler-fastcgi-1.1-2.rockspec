package = "Kepler-Fastcgi"

version = "1.1-2"

source = {
  url = "http://luaforge.net/frs/download.php/3461/kepler-1.1.tar.gz",
}

description = {
  summary = "The Lua Web Development Platform",
  detailed = [[
Kepler is a Lua web development platform that offers the same advantages of Lua. Kepler is simple, portable, light, embeddable and extensible. This is the
FastCGI launcher. ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/kepler"
}

dependencies = {
	"kepler >= 1.1",
	"wsapi-fcgi >= 1.0",
	"coxpcall >= 1.13",
}

build = {
   type = "make",
   build_pass = false,
   install_target = "install-fastcgi",
   install_variables = {
     PREFIX  = "$(PREFIX)",
     LUA_DIR = "$(LUADIR)",
     BIN_DIR = "$(BINDIR)"
   },
   platforms = {
     win32 = {
       build_pass = true,
       build_target = "fcgi",
       build_variables = {
         CFLAGS = "$(CFLAGS)",
	 LUA_INCDIR = "$(LUA_INCDIR)",
	 LUA_LIB = "$(LUA_LIBDIR)\\lua5.1.lib"
       }
     }
   }
}
