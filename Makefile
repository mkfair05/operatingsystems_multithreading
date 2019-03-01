UTHREAD = .
TARGETS = pc_mutex_cond smoke_ smoke_uthread

OBJS = $(UTHREAD)/uthread.o $(UTHREAD)/uthread_mutex_cond.o $(UTHREAD)/uthread_sem.o $(UTHREAD)/smoke_uthread.o
JUNKF = $(OBJS) *~
JUNKD = *.dSYM
CFLAGS  += -g -std=gnu11 -D VERBOSE -I$(UTHREAD)
UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
LDFLAGS += -pthread
endif
all: $(TARGETS)
$(TARGETS): $(OBJS)
tidy:
	rm -f $(JUNKF); rm -rf $(JUNKD)
clean:
	rm -f $(JUNKF) $(TARGETS); rm -rf $(JUNKD)

