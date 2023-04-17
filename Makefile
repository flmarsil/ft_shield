CC					= gcc
CFLAGS				= -Wall -Wextra -Werror
SSLFLAGS			= -lssl -lcrypto
RM 					= rm -rf

DIR_FT_NM 			= .
SRCS_FT_NM 			= $(DIR_FT_NM)/srcs/
INCS_FT_NM 			= $(DIR_FT_NM)/includes/
OBJS_FT_SHIELD		= $(DIR_FT_NM)/objs/

SOURCES				= 	main.c \
						server.c \
						sha256.c \
						handlers.c \
						daemon.c \
						cmd.c 

OBJS 				= $(SOURCES:%.c=$(OBJS_FT_SHIELD)%.o)
TARGET 				= ft_shield

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(SSLFLAGS) -o $(TARGET)

# compiled sources :
$(OBJS): | $(OBJS_FT_SHIELD)

$(OBJS_FT_SHIELD)%.o: $(SRCS_FT_NM)%.c
	$(CC) $(CFLAGS) -I $(INCS_FT_NM) -c $< -o $@

$(OBJS_FT_SHIELD):
	@mkdir -p $(OBJS_FT_SHIELD)

clean: 
	@$(RM) $(OBJS_FT_SHIELD)

fclean: clean
	@$(RM) $(TARGET)


re: fclean all

.PHONY: all clean fclean re

