CC=gcc
CFLAGS= -O3 -march=native  -mtune=generic -Wall  -fopenmp -I. -L.
LIBS=-lm -lpng 

all: st_naive_mask st_naive_up st_naive_down st_mask st_up st_down

clean:
	rm -f main.o mypng.o st_naive_mask.o st_naive_up.o st_naive_down.o st_mask.o st_up.o st_down.o cmp_png.o
	rm -f st_naive_mask st_naive_up st_naive_down st_mask st_up st_down cmp_png
	rm -f out_mask.png out_up.png out_down.png out_naive_mask.png out_naive_up.png out_naive_down.png
	rm -f *_gray_???_UP.png  *_gray_???_DOWN.png *_gray_???_MASK.png 

st_naive_mask: main.o mypng.o st_naive_mask.o
	$(CC) $(CFLAGS) -o st_naive_mask  main.c mypng.o st_naive_mask.o $(LIBS)

st_naive_up: main.o mypng.o st_naive_up.o
	$(CC) $(CFLAGS) -o st_naive_up  main.c mypng.o st_naive_up.o $(LIBS)

st_naive_down: main.o mypng.o st_naive_down.o
	$(CC) $(CFLAGS) -o st_naive_down  main.c mypng.o st_naive_down.o $(LIBS)

st_mask: main.o mypng.o st_mask.o
	$(CC) $(CFLAGS) -o st_mask  main.c mypng.o st_mask.o $(LIBS)

st_up: main.o mypng.o st_up.o
	$(CC) $(CFLAGS) -o st_up  main.c mypng.o st_up.o $(LIBS)

st_down: main.o mypng.o st_down.o
	$(CC) $(CFLAGS) -o st_down  main.c mypng.o st_down.o $(LIBS)

dbg_down: main.o mypng.o dbg_down.o
	$(CC) $(CFLAGS) -o dbg_down  main.c mypng.o dbg_down.o $(LIBS)

cmp_png: cmp_png.c mypng.o
	$(CC) $(CFLAGS) -o cmp_png cmp_png.c mypng.o $(LIBS)	

timings: st_up st_down st_mask \
         lena_gray_512.png       lena_gray_256.png       lena_gray_128.png \
         cameraman_gray_512.png  cameraman_gray_256.png  cameraman_gray_128.png \
         livingroom_gray_512.png livingroom_gray_256.png livingroom_gray_128.png \
         walkbridge_gray_512.png walkbridge_gray_256.png walkbridge_gray_128.png
	./st_up   lena_gray_512.png lena_gray_512_UP.png
	./st_down lena_gray_512.png lena_gray_512_DOWN.png
	./st_mask lena_gray_512.png lena_gray_512_MASK.png
	./st_up   cameraman_gray_512.png cameraman_gray_512_UP.png
	./st_down cameraman_gray_512.png cameraman_gray_512_DOWN.png
	./st_mask cameraman_gray_512.png cameraman_gray_512_MASK.png
	./st_up   livingroom_gray_512.png livingroom_gray_512_UP.png
	./st_down livingroom_gray_512.png livingroom_gray_512_DOWN.png
	./st_mask livingroom_gray_512.png livingroom_gray_512_MASK.png
	./st_up   walkbridge_gray_512.png walkbridge_gray_512_UP.png
	./st_down walkbridge_gray_512.png walkbridge_gray_512_DOWN.png
	./st_mask walkbridge_gray_512.png walkbridge_gray_512_MASK.png
	./st_up   lena_gray_256.png lena_gray_256_UP.png
	./st_down lena_gray_256.png lena_gray_256_DOWN.png
	./st_mask lena_gray_256.png lena_gray_256_MASK.png
	./st_up   cameraman_gray_256.png cameraman_gray_256_UP.png
	./st_down cameraman_gray_256.png cameraman_gray_256_DOWN.png
	./st_mask cameraman_gray_256.png cameraman_gray_256_MASK.png
	./st_up   livingroom_gray_256.png livingroom_gray_256_UP.png
	./st_down livingroom_gray_256.png livingroom_gray_256_DOWN.png
	./st_mask livingroom_gray_256.png livingroom_gray_256_MASK.png
	./st_up   walkbridge_gray_256.png walkbridge_gray_256_UP.png
	./st_down walkbridge_gray_256.png walkbridge_gray_256_DOWN.png
	./st_mask walkbridge_gray_256.png walkbridge_gray_256_MASK.png
	./st_up   lena_gray_128.png lena_gray_128_UP.png
	./st_down lena_gray_128.png lena_gray_128_DOWN.png
	./st_mask lena_gray_128.png lena_gray_128_MASK.png
	./st_up   cameraman_gray_128.png cameraman_gray_128_UP.png
	./st_down cameraman_gray_128.png cameraman_gray_128_DOWN.png
	./st_mask cameraman_gray_128.png cameraman_gray_128_MASK.png
	./st_up   livingroom_gray_128.png livingroom_gray_128_UP.png
	./st_down livingroom_gray_128.png livingroom_gray_128_DOWN.png
	./st_mask livingroom_gray_128.png livingroom_gray_128_MASK.png
	./st_up   walkbridge_gray_128.png walkbridge_gray_128_UP.png
	./st_down walkbridge_gray_128.png walkbridge_gray_128_DOWN.png
	./st_mask walkbridge_gray_128.png walkbridge_gray_128_MASK.png


test:	cmp_png st_naive_up st_naive_down st_naive_mask st_up st_down st_mask lena_gray_256.png
	./st_naive_mask  lena_gray_256.png out_naive_mask.png
	./st_mask        lena_gray_256.png out_mask.png
	./st_naive_up    lena_gray_256.png out_naive_up.png
	./st_up          lena_gray_256.png out_up.png
	./st_naive_down  lena_gray_256.png out_naive_down.png
	./st_down        lena_gray_256.png out_down.png
	./cmp_png out_naive_up.png   out_up.png
	./cmp_png out_naive_mask.png out_mask.png
	./cmp_png out_naive_down.png out_down.png
