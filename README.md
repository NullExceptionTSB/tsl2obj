# tsl2obj
Tool to convert data to linkable ELF symbol files<br>
Built for Tesla project, I'll be putting it in a seperate repository

Made completely in C99

Usage: `tsl2obj [binary file] [object file] [data symbol name] [size symbol name]`

How to compile:<br>
It's one source file, I think you can figure this one out<br>
Tested on MinGW64 GCC

How to use generated symbols:<br>
Simply reference the symbol names via `extern`

Example:<br>
  * Command: `./tsl2obj example.txt example.o ExampleData ExampleSize`
  * Externs:
  ```
  extern const char ExampleData;
  extern const int ExampleSize;
  ```
  

**NOTE: ExampleData is declared as `const char` and not `const char*`**

Using the externs:<br>
There is one specific thing to note, and that is the externals must be used in a specific way.
I'll explain this via an example usage in fwrite
`fwrite(&ExampleData, 1, ExampleSize, OutputFile);`

As you can see, ExampleData is declared as a char, and the buffer is actually `&ExampleData`<br>
This is because I can't figure out how to actually declare relocatable pointers in an ELF symbol.
