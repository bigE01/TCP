FROM gcc:latest

WORKDIR /app

COPY tcp.c .

CMD ["sh", "-c", "gcc tcp.c -o TCP -pthread && ./TCP"]
