all:
	gcc ./Laba1/main.c -o ./Laba1/main.out
	gcc ./Laba2/main.c -o ./Laba2/main.out
	gcc ./Laba2.1/main.c -o ./Laba2.1/main.out
	gcc ./Laba3/broker.c -o ./Laba3/broker.out -lrt
	gcc ./Laba3/subscriber.c -o ./Laba3/subscriber.out -lrt

check: 
	./Laba1/main.out
	./Laba2/main.out
	./Laba2.1/main.out
	echo "Laba3"