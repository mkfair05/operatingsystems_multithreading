UTHREAD = .
#-------------------------------------------------------------
# Uthread compiling
# TARGETS = pc_spinlock pc_mutex_cond smoke pc_sem smoke_sem 
# OBJS = $(UTHREAD)/uthread.o $(UTHREAD)/uthread_mutex_cond.o $(UTHREAD)/uthread_sem.o
#---------------------------------------------------------------
# Pthread compiling
# TARGETS = smoke_pthread
# OBJS = $(UTHREAD)/smoke_pthread.o

TARGETS = pc_sem_pthread 
OBJS = $(UTHREAD)/pc_sem_pthread.o 

# TARGETS = pc_mutex_cond_pthread
# OBJS = $(UTHREAD)/pc_mutex_cond_pthread.o
# ------------------------------------------------------------
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

