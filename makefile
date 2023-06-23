PROJECT = lazy-payload

CC = x86_64-w64-mingw32-gcc
CFLAGS = -std=c99 -O3 -fPIC -fno-function-sections

all: target dummy

target: bin
	$(CC) $(CFLAGS) lazy_payload.c -o ./bin/$(PROJECT).exe

dummy: bin
	$(CC) dummy.c -o ./bin/dummy.exe

.PHONY: docker-container
docker-container:
	docker build -f "Dockerfile" -t "$(PROJECT)-dev" .
.PHONY: docker-instance
docker-instance:
	docker run -itv "$(shell pwd):/var/$(PROJECT)-dev/$(PROJECT)" -u "$(shell id -u):$(shell id -g)" "$(PROJECT)-dev"
.PHONY: docker-build
docker-build:
	docker run -v "$(shell pwd):/var/$(PROJECT)-dev/$(PROJECT)" -u "$(shell id -u):$(shell id -g)" "$(PROJECT)-dev" make

bin:
	mkdir -p $@

clean:
	rm -fr ./bin
