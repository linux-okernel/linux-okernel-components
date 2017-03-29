FROM gcc:4.9
COPY . /usr/src/pmc
WORKDIR /usr/src/pmc
RUN make
CMD ["./protected_memory_client"]
