NAME :=	Learn2Slither

CXX :=	c++
CXXFLAGS :=	-g -MP -MMD -std=c++17 # -Wall -Wextra -Werror -g -MP -MMD # -fsanitize=address -fno-omit-frame-pointer
LFLAGS := -lSDL2
EXTERNAL_DIR := external

###

INCLUDE_DIRS :=	inc/\
				external/imgui/\
				/usr/include/SDL2/\

SRCS :=	main\
		Window\

###

INCLUDE_DIRS :=	$(addprefix -I, $(INCLUDE_DIRS))

SRCS :=	$(addprefix src/, $(SRCS))
SRCS :=	$(addsuffix .cpp, $(SRCS))

IMGUI := $(EXTERNAL_DIR)/imgui
IMGUI_SRCS_RAW =	imgui.cpp\
					imgui_draw.cpp\
					imgui_widgets.cpp\
					imgui_tables.cpp\
					imgui_demo.cpp\
					backends/imgui_impl_sdl2.cpp\
					backends/imgui_impl_sdlrenderer2.cpp
IMGUI_SRCS = $(addprefix $(IMGUI)/, $(IMGUI_SRCS_RAW))

SRCS +=	$(IMGUI_SRCS)

###

OBJ_DIR :=	obj

OBJS =	$(SRCS:%.cpp=$(OBJ_DIR)/%.o)
DEPS =	$(SRCS:%.cpp=$(OBJ_DIR)/%.d)

###

compile: imgui
	@make -j all --no-print-directory

all: $(NAME)

$(EXTERNAL_DIR):
	@mkdir -p external

imgui: $(EXTERNAL_DIR)
	@if ls external | grep -q "imgui"; then \
		printf ""; \
	else \
		echo "\033[31;1mDownloading imgui config\033[0m";\
		git clone https://github.com/ocornut/imgui.git $(IMGUI) -q;\
		echo "\033[31;1mDownloaded imgui config\033[0m";\
	fi

$(NAME): $(OBJS)
	@echo Compiling $(NAME)
	@$(CXX) $(CXXFLAGS) $(LFLAGS) $(INCLUDE_DIRS) -o $@ $^

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo Compiling $@
	@$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

re: fclean compile

fclean: clean
	@echo Removed $(NAME)
	@rm -rf $(NAME)

clean:
	@echo Removed $(OBJ_DIR)
	@rm -rf $(OBJ_DIR)

.PHONY: all clean fclean re compile

-include $(DEPS)
