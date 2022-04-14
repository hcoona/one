# cpp-deps

Ported from https://github.com/nafur/cpp-deps

Visualize C++ include dependencies

## usage

```
	Options:
		--help                show help
		--commands arg        path to compile_commands.json
		--output arg          output file for graphviz file
		--exclude arg (=[/usr/include/, /usr/lib/, /build/])
		                      exclude patterns for files
```

## output

Produces a graph representation of the include dependencies.
