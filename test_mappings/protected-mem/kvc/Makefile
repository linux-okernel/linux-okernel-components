.PHONY: clean 

all: kernel_vuln_client
	

kernel_vuln_client:
	$(CC) $(CFLAGS) -o kernel_vuln_client kernel_vuln_client.c
	
clean:
	rm -rf *.o *~ core kernel_vuln_client
