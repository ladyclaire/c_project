SRCDIR = src
INCDIR = include
OBJDIR = .
BINDIR = .
TARGET = rep

CFLAGS += -Wall -g -I$(INCDIR)
 
LIBS = -lpcre
CFLAGS += -DVERSION=\"$(VER)\" -DREVISION=\"$(REV)\" -DPLATFORM=\"$(PLATFORM)\"
LDFLAGS += -L/usr/lib
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(OBJS:%.o=%.d)

all:compile
compile:$(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET):$(OBJS) $(DEPS)
	@mkdir -p $(BINDIR)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(OBJS):$(OBJDIR)/%.o:$(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
	
$(DEPS):$(OBJDIR)/%.d:$(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) -MM -MT"$(@:%.d=%.o)" $(CFLAGS) $< > $@

.PHONY : clean 
clean :
	rm -rf *.o *.d $(TARGET)

.PHONY : install 
install: compile

ifeq ($(OBJDIR), $(wildcard $(OBJDIR)))
-include $(DEPS)
endif

