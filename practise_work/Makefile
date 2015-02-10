all: pwork

OBJECTS   = obj/main.o obj/parparser.o
DFLAG     =
CFLAG     = -Iinclude/ -Iparparser/ -IQGen/include/ -ICGen/include/ -O3
LFLAGS    = -openmp
GPULFLAGS = -openmp -L$(LD_LIBRARY_PATH)
CC        = mpicxx  
GPU = 

pwork: $(OBJECTS)
	@mkdir -p bin
	@echo "\033[30;1;41m "bin" dir created \033[0m"
	@make -C CGen/ -f Makefile all
ifneq ($(GPU), )
	@make -C QGen/ -f Makefile_GPU all
	@$(CC) $(GPULFLAGS) $(OBJECTS) QGen/lib/QGen_gpu.a CGen/lib/CGen.a -o bin/pwork_gpu
else
	@make -C QGen/ -f Makefile_CPU all	
	@$(CC) $(LFLAGS) $(OBJECTS) QGen/lib/QGen_cpu.a CGen/lib/CGen.a -o bin/pwork
endif

obj/main.o: src/main.cpp
	@mkdir -p obj
ifneq ($(GPU), )
	@$(CC) -c -DGPU $(CFLAG) $^ -o $@
else
	@$(CC) -c $(CFLAG) $^ -o $@
endif
	@echo "\033[30;1;46m $@ - done \033[0m\n"

obj/parparser.o: parparser/parparser.cpp
	@mkdir -p obj
	@$(CC) -c $(CFLAG) $^ -o $@
	@echo "\033[30;1;46m $@ - done \033[0m\n"

clean:
	rm -r -f bin
	rm -r -f obj

clean_lib:
	@make -C CGen/ -f Makefile clean
	@make -C QGen/ -f Makefile_CPU clean

clean_all: clean clean_lib