#!/bin/bash

for i in $(seq 1 19); do
	rm media;
	gcc -x assembler-with-cpp -D TEST=$i media.s -no-pie -o media;
	printf "__TEST%02d__%35s\n" $i "" | tr " " "-" ; ./media;
done
