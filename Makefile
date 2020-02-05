NAME    :=  jpegtofb
VERSION :=  0.1a
CC      :=  gcc 
#LIBS    := -l:libjpeg.so.62 ${EXTRA_LIBS} 
LIBS    := ${EXTRA_LIBS} 
TARGET	:= $(NAME)
SOURCES := $(shell find src/ -type f -name *.c)
OBJECTS := $(patsubst src/%,build/%,$(SOURCES:.c=.o))
DEPS	:= $(OBJECTS:.o=.deps)
DESTDIR := /
PREFIX  := /usr
MANDIR  := $(DESTDIR)/$(PREFIX)/share/man
BINDIR  := $(DESTDIR)/$(PREFIX)/bin
SHARE   := $(DESTDIR)/$(PREFIX)/share/$(TARGET)
CFLAGS  := -g -O3 -fpie -fpic -Wall -DNAME=\"$(NAME)\" -DVERSION=\"$(VERSION)\" -DSHARE=\"$(SHARE)\" -DPREFIX=\"$(PREFIX)\" ${EXTRA_CFLAGS}
LDFLAGS := -pie ${EXTRA_LDFLAGS}

all: $(TARGET)
debug: CFLAGS += -g
debug: $(TARGET) 

$(TARGET): $(OBJECTS) 
	@$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) 

build/%.o: src/%.c
	@mkdir -p build/
	$(CC) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	@echo "  Cleaning..."; $(RM) -r build/ $(TARGET) 

install: $(TARGET)
	mkdir -p $(DESTDIR)/$(PREFIX) $(DESTDIR)/$(BINDIR) $(DESTDIR)/$(MANDIR)
	strip $(TARGET)
	install -m 755 $(TARGET) $(DESTDIR)/${BINDIR}
	mkdir -p $(DESTDIR)/$(MANDIR)/man1
	cp -p man1/* $(DESTDIR)/${MANDIR}/man1/

-include $(DEPS)

.PHONY: clean

