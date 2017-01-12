CFLAGS = -g -W \
-O3 \
-Wall \
-Wpointer-arith \
-Wshadow \
-Wno-long-long \
-Wformat \
-Winvalid-pch \
-std=c++1z \
-I ./../include \
-l pthread
server:rm_server
	g++ unit_test.cpp $(CFLAGS) -o unit_test
rm_server:
	$(shell if [ -f server ]; then rm server; fi;)

