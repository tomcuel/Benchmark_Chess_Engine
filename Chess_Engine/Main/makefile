CC=gcc
CGFLAGS=-Wall -Wfatal-errors -I/opt/homebrew/include/SDL2
LDFLAGS=-Iinclude -lSDL2main -lSDL2 -L/opt/homebrew/lib -lSDL2_ttf -lSDL2_image

all : Chess_Game.x

Chess_Game.x : Chess_Game.o AI.o ../Alpha_Beta/Evaluation.o ../Alpha_Beta/Move_Ordering.o ../Alpha_Beta/Openings.o ../Alpha_Beta/Pieces_Squares_Tables.o ../Alpha_Beta/Pre_Computed_Data.o ../Alpha_Beta/Search.o ../Alpha_Beta/Transposition_Table.o ../Game_Treatment/Board.o ../Game_Treatment/Log.o ../Game_Treatment/Move_Generator.o ../Game_Treatment/Moves_Treatment.o ../Game_Treatment/Settings.o Download_Log.o Graphics.o ../Useful/Utility.o
	$(CC) $(CGFLAGS) -o $@ $^ $(LDFLAGS)

%.o : %.c 
	$(CC) $(CGFLAGS) -o $@ -c $< $(LDFLAGS)

clean:
	rm -f *.o

realclean: clean
	rm -f *.x