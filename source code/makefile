myFileSys : global.o basic_class.o file_sys.o main.o
	g++ global.o basic_class.o file_sys.o main.o -o myFileSys -g

global.o : global.cpp
	g++ -c global.cpp

basic_class.o : basic_class.cpp
	g++ -c basic_class.cpp

file_sys.o:  file_sys.cpp 
	g++ -c file_sys.cpp

main.o: main.cpp 
	g++ -c main.cpp

clean:
	-rm -f *.o

