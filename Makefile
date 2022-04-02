APP=ProxyServer
CLASSES=srcs/ServerProxy
MAIN=main

CXX			:= clang++
CXXFLAGS	= -g3 -D_GLIBCXX_DEBUG -fdiagnostics-color=always

CPP_FILES=$(addsuffix .cpp, $(CLASSES) $(MAIN))
H_FILES=$(addsuffix .h, $(CLASSES))
OBJ=$(addsuffix .o, $(CLASSES) $(MAIN))

all: $(APP)

$(APP): $(OBJ)
	$(CXX) $(CXXFLAGC) $^ -o $(APP)
%.o: %.cpp
	$(CXX) $(CXXFLAGC) $(addprefix -I., $(H_FILES)) $< -c -o $@

clean:
	rm -rf $(OBJ)
fclean:
	rm -rf $(OBJ) $(APP)

