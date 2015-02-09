all: pwork

OBJECTS = obj/main.o obj/parparser.o
DFLAG =
CFLAG = -Iinclude/ -Iparparser/ -IQGen/include/ -O3

pwork: $(OBJECTS)
	mkdir -p lib
	@make -C QGen/ all
	@mpicxx -lQGen/lib/QGen.a $OBJECTS -o pwork
	@echo -en "\033[30;1;41m pwork builded successfully! \033[0m\n"

obj/main.o: src/main.cpp
	@mkdir -p obj
	@mpicxx -c $(CFLAG) $^ -o $@
	@echo -en "\033[30;1;46m $@ - done \033[0m\n"

obj/parparser.o: parparser/parparser.cpp
	@mkdir -p obj
	@mpicxx -c $(CFLAG) $^ -o $@
	@echo -en "\033[30;1;46m $@ - done \033[0m\n"

clean:
	rm -r -f lib
	rm -r -f obj