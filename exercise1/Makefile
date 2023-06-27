CC     = mpicc
INCDIR = headers
OBJDIR = objects
SRCDIR = source

CFLAGS = -c -I$(INCDIR)
OFLAGS = -fopenmp -Wextra -Wall -O3 -march=native

OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/run.o $(OBJDIR)/initialize.o $(OBJDIR)/read_write.o

main.x: $(OBJECTS)
	$(CC) $(OFLAGS) $^ -o $@
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(OFLAGS) $^ -o $@
clean:
	rm -rf *.o *.x $(OBJDIR)/*.o
image:
	rm snap* init*
