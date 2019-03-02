FOLDER = .

TARGETS =   pc_mutex_cond_pthread pc_sem_pthread smoke_pthread pc_spinlock pc_sem_uthread pc_mutex_cond_uthread pc_sem_uthread smoke_uthread

OBJS = $(FOLDER)/uthread.o \
		$(FOLDER)/uthread_mutex_cond.o \
		$(FOLDER)/uthread_sem.o
# ------------------------------------------------------------
JUNKF = $(FOLDER) *~
JUNKD = *.dSYM
CFLAGS  += -g -std=gnu11 -I$(FOLDER)
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

