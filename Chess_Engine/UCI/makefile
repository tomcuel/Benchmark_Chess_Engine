# Build of the chess engine : the UCI program, the perft check and the self-play check
# the objects are put in build/
CC=gcc
# -flto lets the compiler optimize across the files (the small functions called everywhere get inlined)
CGFLAGS=-O2 -flto -Wall -Wfatal-errors

ENGINE_SRC = $(wildcard ../Alpha_Beta/*.c) $(wildcard ../Game_Treatment/*.c) ../Useful/Utility.c
ENGINE_OBJ = $(patsubst ../%.c,build/%.o,$(ENGINE_SRC))

# Engine_Check.x : makes the engine play against itself, to check that it works
# Perft.x : checks the move generation on reference positions
# UCI.x : the engine with the UCI protocol, to make it play against other engines
all : Engine_Check.x Perft.x UCI.x

%.x : build/UCI/%.o $(ENGINE_OBJ)
	$(CC) $(CGFLAGS) -o $@ $^ -lm -lpthread

# to keep the objects between two builds
.PRECIOUS : build/UCI/%.o build/%.o

build/UCI/%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(CGFLAGS) -MMD -MP -o $@ -c $<

build/%.o : ../%.c
	@mkdir -p $(dir $@)
	$(CC) $(CGFLAGS) -MMD -MP -o $@ -c $<

# the headers each object depends on (written by the compiler with -MMD), so changing a .h recompiles what includes it
-include $(wildcard build/*/*.d)

clean:
	rm -rf build

realclean: clean
	rm -f *.x
