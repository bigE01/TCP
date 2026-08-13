FROM gcc:latest

WORKDIR /app

COPY TCP.c .

CMD ["sh", "-c", "gcc TCP.c -o TCP -pthread && ./TCP"]
