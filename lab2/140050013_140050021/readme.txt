Experimental setup:
Connected  Our 2 Laptop with CAT 6 Lan cable
max bandwidth achieved (calculated via iperf)= 11.775 MiB/s

Instruction to Run the code:

1)create the executables:
	a) for server:
		gcc server-mp.c -o server-mp

	b) for client:
		gcc multi-client.c -o multi-client -lpthread

2) running the executables:
	a) for server:
		./server-mp [port]

	b) for client:
		./multi-client [server ip] [server port] [number of threads] [time] [sleep time] [random/fixed]