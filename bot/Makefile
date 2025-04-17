# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pmagnero <pmagnero@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/11/04 11:30:16 by pmagnero          #+#    #+#              #
#    Updated: 2025/02/11 17:33:30 by pmagnero         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME				=	bot

FLAG				= -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS				=	srcs/main.cpp \
						srcs/Server.cpp \
						srcs/utils.cpp

INC					=	includes
INCS				= 	$(INC)/Server.hpp

OBJS_DIR			=	objs/
OBJS				=	$(SRCS:.cpp=.o)
OBJS_PREFIXED		=	$(addprefix $(OBJS_DIR), $(OBJS))

CC					=	c++

COLOR_TITLE	=	36
COLOR_KO	=	31
COLOR_OK	=	32

# debug
d	=	0
# no comment error in norme
c 	=	0

b	=	0

ifeq ($(d),1)
	DEBUG	=	-ggdb
#	PG		=	-pg
	PG		=	
else
    DEBUG	=	
    PG		=	
endif

$(OBJS_DIR)%.o : %.cpp $(INCS)
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(OBJS_DIR)/srcs
	$(CC) $(DEBUG) $(FLAG) -c $< -o $@


$(NAME): $(OBJS_PREFIXED)
	@echo "\n\033[$(COLOR_TITLE)m########################################"
	@echo "################  MAKE $(NAME)  ################"
	@echo "########################################\033[0m\n"
	$(CC) $(DEBUG) -o $(NAME) -I$(INC) $(FLAG) $(OBJS_PREFIXED)

all: $(NAME)

kek: $(NAME) extfunc valgrind

extfunc:
	@echo "\n\033[$(COLOR_TITLE)m########################################"
	@echo "#########  EXTERNAL FUNCTION  ##########"
	@echo "########################################\033[0m\n"
	@nm -gu $(NAME) | awk '!/malloc|exit|free|__|write|memcpy|X|pthread/ {++n;sub(/[ \t]+/, ""); printf "%s", $$0; if ($$0) printf "\t\033[$(COLOR_KO)mERROR!\033[0m\n\n"} END{if (n > 0) {printf "\033[$(COLOR_KO)mYou Fucking DONKEY, what are those forbidden functions !!!\033[0m\n\n";} else {printf "\033[$(COLOR_OK)mOK!, No forbidden functions in use !\033[0m\n\n"}}'

valgrind:
	@echo "\n\033[$(COLOR_TITLE)m########################################"
	@echo "##############  VALGRIND  ##############"
	@echo "########################################\033[0m\n"
	@valgrind $(VALGRIND_F) --log-file=$(LOGS)/valgrind.log ./$(NAME)

clean:
	@echo "\n\033[$(COLOR_TITLE)m########################################"
	@echo "##############  CLEANING  ##############"
	@echo "########################################\033[0m\n"
	@rm -rf $(OBJS_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all extfunc valgrind clean fclean re
