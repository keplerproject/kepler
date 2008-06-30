# $Id: Makefile,v 1.1 2008/06/30 14:29:59 carregal Exp $


install:
	chmod +x src/setup-kepler
	chmod +x src/cgilua.cgi
	chmod +x src/op.cgi
	find . -name "*.lp" -o -name "*.lua" -o -name "*.ws" -o -name "*.op" | xargs chmod +x
	cp -r web $(PREFIX)/
	cp -r doc $(PREFIX)/
	cp -r css $(PREFIX)/
	cp -r conf $(PREFIX)/
	cp src/setup-kepler  $(BIN_DIR)
	cp src/cgilua.cgi $(BIN_DIR)
	cp src/op.cgi $(BIN_DIR)
	test -e $(LUA_DIR)/kepler_init.lua || cp src/kepler_init.lua $(LUA_DIR)

install-fastcgi:
	chmod +x src/cgilua.fcgi
	chmod +x src/op.fcgi
	cp src/cgilua.fcgi $(BIN_DIR)
	cp src/op.fcgi $(BIN_DIR)

install-xavante:
	chmod +x src/xavante_start
	cp src/xavante_start $(BIN_DIR)

dist:
	darcs dist -d kepler-rocks-cvs
	mv kepler-rocks-cvs.tar.gz ..

gen_dist:
	darcs push 139.82.100.4:public_html/kepler-rocks/current
	ssh 139.82.100.4 "cd public_html/kepler-rocks/current && make dist"

