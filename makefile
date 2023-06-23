PROJECT = lazy-payload

CC = x86_64-w64-mingw32-gcc
CFLAGS = -O3 -fno-function-sections -fPIC

all: target dummy

target:
	$(CC) $(CFLAGS) lazy_payload.c -o $(PROJECT).exe

dummy:
	$(CC) dummy.c -o dummy.exe

.PHONY: docker-container
docker-container:
	docker build -f "Dockerfile" -t "$(PROJECT)-dev" .
.PHONY: docker-instance
docker-instance:
	docker run -itv "$(shell pwd):/var/$(PROJECT)-dev/$(PROJECT)" -u "$(shell id -u):$(shell id -g)" "$(PROJECT)-dev"
.PHONY: docker-build
docker-build:
	docker run -v "$(shell pwd):/var/$(PROJECT)-dev/$(PROJECT)" -u "$(shell id -u):$(shell id -g)" "$(PROJECT)-dev" make

clean:
	rm -f *.exe
