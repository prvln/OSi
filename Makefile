all:
	gcc ./Laba1/main.c -o ./Laba1/main.out
	gcc ./Laba2/main.c -o ./Laba1/main.out

check: 
	./Laba1/main.out
	./Laba2/main.out