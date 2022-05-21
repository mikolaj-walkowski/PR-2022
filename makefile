build:
	mpicc main.c -o main -lm
run: build
	mpirun --oversubscribe -np 10 --map-by node main
	rm main