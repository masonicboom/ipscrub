nginx/:
	-git clone --branch release-1.13.9 --depth 1 https://github.com/nginx/nginx.git

nginx/objs/nginx: nginx/Makefile ipscrub/src/ngx_ipscrub_module.c
	cd nginx && make

nginx/Makefile: nginx/
	cd nginx && ./auto/configure --prefix=$(PWD)/dest --add-module=../ipscrub --add-dynamic-module=../ipscrub --with-compat

nginx/objs/ngx_ipscrub_module.so: nginx/Makefile
	cd nginx && make modules

dest/conf/nginx.conf: ipscrub/nginx.conf dest/
	cp $< $@

restart: nginx/objs/nginx dest/conf/nginx.conf
	pkill nginx && $<

start: nginx/objs/nginx dest/conf/nginx.conf
	@mkdir -p dest/logs/
	$<

dest/:
	mkdir -p dest/conf/
	mkdir -p dest/logs/

test: clean test-sha1hash test-saltedhash test-ipv46hash test-dynmodule

test-sha1hash: ipscrub/test/sha1hash/test.sh ipscrub/test/sha1hash/nginx.conf dest/ nginx/objs/nginx
	cp ipscrub/test/sha1hash/nginx.conf dest/conf/nginx.conf
	-pkill nginx
	nginx/objs/nginx
	bash $<

test-saltedhash: ipscrub/test/saltedsha/test.sh ipscrub/test/saltedsha/nginx.conf dest/ nginx/objs/nginx
	cp ipscrub/test/saltedsha/nginx.conf dest/conf/nginx.conf
	-pkill nginx
	nginx/objs/nginx
	bash $<

test-ipv46hash: ipscrub/test/ipv46hash/test.sh ipscrub/test/ipv46hash/nginx.conf dest/ nginx/objs/nginx
	cp ipscrub/test/ipv46hash/nginx.conf dest/conf/nginx.conf
	-pkill nginx
	nginx/objs/nginx
	bash $<

test-dynmodule: ipscrub/test/dynmodule/test.sh nginx/objs/ngx_ipscrub_module.so
	cd nginx && ./auto/configure --prefix=$(PWD)/dest --with-compat && make
	cp nginx/objs/ngx_ipscrub_module.so dest/
	cp ipscrub/test/dynmodule/nginx.conf dest/conf/nginx.conf
	-pkill nginx
	nginx/objs/nginx
	bash $<

clean:
	-rm -f nginx/Makefile
	-rm -f nginx/objs/nginx
	-rm -rf dest/

run-demo:
	docker build -t ipscrub .
	docker build -t ipscrub-demo-client demo/
	docker-compose up --abort-on-container-exit

check-up-to-date: script/check-up-to-date.sh
	$<
