#fisier folosit pentru compilarea serverului&clientului UDP

all:
	g++ -o servUdp.exe -I/C:\MinGW\lib servUdp.cpp -lws2_32 -lwininet
	g++ -o cliUdp.exe -I/C:\MinGW\lib cliUdp.cpp -lws2_32
	./servUdp.exe
	
clean:
	rm -f *~cliUdp.exe servUdp.exe
