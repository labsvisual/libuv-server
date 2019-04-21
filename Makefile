server: http-parser/http_parser.o libuv/libuv.a
	$(CC) -o server \
		src/server.c libuv/build/Release/libuv.a http-parser/http_parser.o \
		-lpthread -I libuv/include

libuv/libuv.a:
	cd libuv && python gyp_uv.py -f xcode
	cd libuv && xcodebuild -ARCHS="x86_64" -project out/uv.xcodeproj \
		-configuration Release -alltargets

http-parser/http_parser.o:
	$(MAKE) -C http-parser http_parser.o

clean:
	$(MAKE) -C http-parser clean
	-$(RM) -r libuv/build
	-$(RM) -r libuv/uv.xcodeproj
	-$(RM) httpserver
