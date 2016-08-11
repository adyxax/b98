# Common
prefix= /usr/local

CC=clang
CFLAGS= -Wall -Werror -Wextra
#CFLAGS+= -std=c99
CFLAGS+= -std=gnu99
#CFLAGS+= -Os
CFLAGS+= -g

LDFLAGS= -lgnutls -lpthread -ldl -rdynamic
LDFLAGS+= -g
#LDFLAGS+= -Os

# Target: cbot
cbot_BIN= cbot
cbot_OBJ= $(subst .c,.o,$(wildcard src/*.c))

# Targets: plugins
plugins_SO= $(subst .c,.so,$(wildcard src/plugin/*.c))

# Rules
all: $(cbot_BIN) $(plugins_SO)

$(cbot_BIN): $(cbot_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(cbot_BIN) $(cbot_OBJ)

%.so: %.c
	$(CC) -shared -nostartfiles -fPIC $(CFLAGS) $*.c -o $*.so
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

-include $(cbot_OBJ:.o=.d)

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

clean:
	$(RM) */*.o */*.d */*/*.o */*/*.d */*/*.so
	$(RM) $(cbot_BIN) AllTests
	$(RM) -rf clang-analyzer

check:
	scan-build -o clang-analyzer make
	clang -o AllTests $(wildcard tests/*.c)
	./AllTests

tags:
	ctags -o .tags -a $(wildcard src/*.[hc])

.PHONY: all clean install uninstall tags

start:
	./cbot
