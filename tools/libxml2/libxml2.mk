# libxml2 compiled from libs/libxml2 with the source list in sources.txt. Shared by
# the root Makefile and tools/Makefile. Set LIBXML2_ROOT to the repository prefix
# ("" or "../") and LIBXML2_OBJDIR for the object directory before including it.
# LIBXML2_CC defaults to the caller's CC.
LIBXML2_SRCDIR = $(LIBXML2_ROOT)libs/libxml2
LIBXML2_GENDIR = $(LIBXML2_ROOT)tools/libxml2
LIBXML2_SRCS = $(shell cat $(LIBXML2_GENDIR)/sources.txt)
LIBXML2_OBJS = $(addprefix $(LIBXML2_OBJDIR)/,$(LIBXML2_SRCS:.c=.o))
LIBXML2_CC ?= $(CC)
LIBXML2_CFLAGS = -O2 -fpic -I$(LIBXML2_GENDIR) -I$(LIBXML2_GENDIR)/include -I$(LIBXML2_SRCDIR)/include

$(LIBXML2_OBJDIR)/%.o: $(LIBXML2_SRCDIR)/%.c $(LIBXML2_GENDIR)/config.h $(LIBXML2_GENDIR)/include/libxml/xmlversion.h
	@mkdir -p $(dir $@)
	$(LIBXML2_CC) $(LIBXML2_CFLAGS) $(LIBXML2_EXTRA_CFLAGS) -c $< -o $@
