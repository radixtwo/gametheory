
SRCDIR	=	src
LIBDIR	=	lib
OBJDIR	=	obj
BINDIR	=	bin

OBJECT	:=	$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(shell find $(SRCDIR) -name *.c))
OEUVRE	:=	$(patsubst $(SRCDIR)/%.c,$(BINDIR)/%,$(shell grep -rnwl $(SRCDIR) -e \
			"^\s*\(int\|void\)\s\+main\s*(\s*\(int\s\+argc\s*\(,\s*char\s*\*\s*\(argv\[\]\|\*\s*argv\)\)\?\)\?\s*)\s*{\?\s*$$"))
    
CC		=	gcc
CFLAGS	=	-g -std=gnu11 -Og -Wall -Wextra $$etcetera
#CFLAGS	=	-g -std=gnu11 -O3 -Wall -Wextra $$etcetera
#CFLAGS	=	-g -std=gnu11 -Ofast -Wall -Wextra $$etcetera
LDFLAGS	=	
LDLIBS	=	

export etcetera	=	-Wfloat-equal -Wtype-limits -Wpointer-arith -Wlogical-op -Wshadow

all:: $(OEUVRE)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(COMPILE.c) -I$(LIBDIR) $< -o $@

$(OEUVRE): %:$(OBJECT)
	$(LINK.o) $(filter %.o,$^) $(LDLIBS) -o $@
	chmod 755 $@
	ln -sf $(OEUVRE) run

clean::
	rm -f $(OEUVRE) $(OBJECT) run

.PHONY: clean all

