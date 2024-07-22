SUBDIRS += base/     
SUBDIRS += base/common/     
SUBDIRS += base/mempool/     
SUBDIRS += base/threadpool/     
SUBDIRS += core/
SUBDIRS += persistence/
SUBDIRS += engine/
SUBDIRS += engine/list/
SUBDIRS += engine/rbtree/
SUBDIRS += network/
SUBDIRS += protocol/

TOP_DIR = $(PWD)
OBJ = kvcache
SRCS := $(foreach subdir, $(SUBDIRS), $(wildcard $(TOP_DIR)/$(subdir)/*.c))
SRCS += $(wildcard *.c)
CUR_OBJS := $(patsubst %.c, %.o, $(wildcard *.c))
OBJS := $(patsubst %.c, %.o, $(SRCS))

CFLAGS += $(foreach subdir, $(SUBDIRS), "-I$(TOP_DIR)/$(subdir)")
CFLAGS += -I$(TOP_DIR) -g
LDFLAGS += -lpthread -lm -ldl -Wall -lrt -g --std=c11

export CFLAGS

$(OBJ): subdirs $(CUR_OBJS)
	@$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS)

%.o:%.c
	@$(CC) -c $(CFLAGS) $< -o $@

subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: clean subdirs $(SUBDIRS)
clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
		done
	@rm -f *.o $(OBJ)

