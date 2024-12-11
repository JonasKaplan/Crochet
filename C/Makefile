CC = clang
CFLAGS = -g -Wall -Wextra -Wpedantic -Werror
OUT = ./out
LIBRARY = libcrochet.a

all: $(OUT)/$(LIBRARY)

$(OUT)/$(LIBRARY): $(OUT)/rules.o $(OUT)/parser.o $(OUT)/nodes.o $(OUT)/lines.o $(OUT)/actions.o $(OUT)/interpreter.o $(OUT)/base.o
	ar rcs $@ $^

$(OUT)/rules.o: Parser/rules.c output_dir
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/parser.o: Parser/parser.c output_dir
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/nodes.o: Parser/nodes.c output_dir
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/lines.o: Parser/lines.c output_dir
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/actions.o: Parser/actions.c output_dir
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/interpreter.o: interpreter.c output_dir
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT)/base.o: base.c output_dir
	$(CC) $(CFLAGS) -c -o $@ $<

clean: output_dir
	rm -rf $(OUT)

output_dir:
	@mkdir -p $(OUT)
