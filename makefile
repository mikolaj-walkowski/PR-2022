build:
	mpicc main.c -o main -lm
run: build
	mpirun --oversubscribe -np 32 --map-by node main
	rm main