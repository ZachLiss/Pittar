all: pittar compress

pittar:
	gcc -o pittar pittar.c -fnested-functions

compress:
	gcc -o compress compress.c -fnested-functions