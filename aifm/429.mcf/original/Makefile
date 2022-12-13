# Include standard template for this suite
CFLAGS += -O3 

obj_dir = obj
_obj = implicit.o mcf.o mcfutil.o output.o pbeampp.o pbla.o pflowup.o psimplex.o pstart.o readmin.o treeup.o internal_utils.o
OBJ = $(addprefix $(obj_dir)/, $(_obj))

$(obj_dir):
	mkdir -p $@

$(obj_dir)/%.o : %.c *.h
	$(CC) $(CFLAGS) $< -c -o $@;
	
.PHONY: all
all: $(obj_dir) mcf

mcf: $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(addsuffix .ll, $(OBJ)) mcf
