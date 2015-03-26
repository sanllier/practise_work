all: pwork

#-----------------------------------------------------------------------------

OBJECTS   = obj/main.o obj/parparser.o
CC        = mpicxx  
GPU       = 
CGEN      = 

#-----------------------------------------------------------------------------

CFLAGS  = -Iinclude/ -Iparparser/ -IQGen/include/ -O3
ifneq ($(GPU), )	
	LFLAGS   = -L/opt/cuda/cuda-6.5/lib64 -lcudart -lcurand
	LIBS     = QGen/lib/qgen_gpu.a
	OUTFILE  = pwork_gpu
	DFLAGS   = -DGPU
else
	LFLAGS   = 
	LIBS     = QGen/lib/qgen_cpu.a
	OUTFILE  = pwork_cpu
	DFLAGS   = 
endif

ifneq ($(GPU), )	
	TARGETMAKE = -C QGen/ -f Makefile all GPU=Y
else
	TARGETMAKE = -C QGen/ -f Makefile all
endif

#-----------------------------------------------------------------------------

pwork: $(OBJECTS)
	@mkdir -p bin
	@echo "\033[30;1;41m "bin" dir created \033[0m"
	@make $(TARGETMAKE)
	@$(CC) $(LFLAGS) $(OBJECTS) $(LIBS) -o bin/$(OUTFILE)
	@echo "\033[30;1;41m ALL DONE \033[0m"

#-----------------------------------------------------------------------------

obj/main.o: src/main.cpp
	@mkdir -p obj
	@$(CC) -c $(DFLAGS) $(CFLAGS) $^ -o $@
	@echo "\033[30;1;46m $@ - done \033[0m\n"

#-----------------------------------------------------------------------------

obj/parparser.o: parparser/parparser.cpp
	@mkdir -p obj
	@$(CC) -c $(CFLAGS) $^ -o $@
	@echo "\033[30;1;46m $@ - done \033[0m\n"

#-----------------------------------------------------------------------------

clean:
	rm -r -f bin
	rm -r -f obj

#-----------------------------------------------------------------------------

clean_lib:
	@make -C QGen/ -f Makefile clean

#-----------------------------------------------------------------------------

clean_all: clean clean_lib

#-----------------------------------------------------------------------------