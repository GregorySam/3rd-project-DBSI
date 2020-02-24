all: sr_main1 sr_main2 sr_main3

sr_main1:
	@echo " Compile sr_main1 ...";
	gcc -Wall -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sr_main1.c ./src/sort_file.c ./src/Sort_Methods.c -lbf -O2 -o ./build/sr_main1

sr_main2:
	@echo " Compile sr_main2 ...";
	gcc -Wall -g -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sr_main2.c ./src/sort_file.c ./src/Sort_Methods.c -lbf -O2 -o ./build/sr_main2

sr_main3:
	@echo " Compile sr_main3 ...";
	gcc -Wall -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/sr_main3.c ./src/sort_file.c ./src/Sort_Methods.c -lbf -O2 -o ./build/sr_main3 


bf:
	@echo " Compile bf_main ...";
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c -lbf -o ./build/runner -O2
