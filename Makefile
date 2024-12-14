NAME = ft_traceroute

OBJ_DIR = $(PWD)/obj
SRC_DIR = $(PWD)/src
INC_DIR = $(PWD)/inc
LIBFT_DIR = $(PWD)/libft

BLOCK_SIZE = $(shell stat -fc %s .)

SRCS = traceroute.c

OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

LIBFT = $(LIBFT_DIR)/libft.a
LIBFT_FLAGS = -L$(LIBFT_DIR) -lft

CC = cc
CFLAGS = 	-DFS_BLOCK_SIZE=${BLOCK_SIZE} \
			-Wall -Wextra -Werror \
			-I$(INC_DIR) \
			-I$(LIBFT_DIR)/src/alloc \
			-I$(LIBFT_DIR)/src/mem \
			-I$(LIBFT_DIR)/src/str \
			-I$(LIBFT_DIR)/src/print \
			-I$(LIBFT_DIR)/src/bit \

LDFLAGS = $(LIBFT_FLAGS)

all: $(LIBFT) $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)
	@echo "$(NAME) built successfully."

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $< into $@"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
	$(MAKE) -C $(LIBFT_DIR) clean

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C $(LIBFT_DIR) fclean

re: fclean all

prof:
	FILE_NAME="/tmp/.out" ./tests/generate_random_file.sh
	make fclean
	make CFLAGS="${CFLAGS} -pg -g"
	valgrind --tool=callgrind ./ft_ssl ${ALGO} /tmp/.out
	callgrind_annotate callgrind.out.* > ${OUTFILE}
	rm callgrind.out.* gmon.out

debug: CFLAGS += -g
debug: all

$(LIBFT):
	$(MAKE) -C $(LIBFT_DIR)

.PHONY: all clean fclean re