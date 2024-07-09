all: 
	gcc a.c \
	-I/home/jiao/Software/OpenSSL-3.3.1/include \
	-L/home/jiao/Software/OpenSSL-3.3.1/lib64 \
	-Wl,-rpath=/home/jiao/Software/OpenSSL-3.3.1/lib64 -lssl -lcrypto \
	-o a && \
	gcc b.c \
	-I/home/jiao/Software/OpenSSL-3.3.1/include \
	-L/home/jiao/Software/OpenSSL-3.3.1/lib64 \
	-Wl,-rpath=/home/jiao/Software/OpenSSL-3.3.1/lib64 -lssl -lcrypto \
	-o b