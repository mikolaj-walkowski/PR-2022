build:
	mpicc newmain.c -o main -lm -lpthread
run: build
	mpirun --oversubscribe -np 3 --map-by node main
	rm main