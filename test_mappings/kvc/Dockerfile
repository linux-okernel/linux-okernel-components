FROM gcc:4.9
COPY . /usr/src/kvc
WORKDIR /usr/src/kvc
RUN make
CMD ["./kernel_vuln_client"]
