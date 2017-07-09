# introspection-result-checking

# To build the tool:
------------------------------------
	mkdir build
	cd build
	cmake ../
	make
  
  
# Setting up file to mark functions as sensitive:
-------------------------------------------------------------
Add all sensitive function names to a file, with one function name per line.

# To run the tool:
---------------------------------------------
- dump llvm bitcode for further instrumentation
  
    `clang source.c -c -emit-llvm`
- Dump llvm bitcode of response function

    `clang ../response_lib.c -c -emit-llvm`
- Run llvm transformation pass to add guards into each sensitive function

    `opt -load ../build/lib/libresult-checking.so source.bc -insert-guards -sensitive-functions sensitive.txt -connectivity-level <con_level> -o protected.bc`
- link bitcode from the previous step with the bitcode of external library containing an integrity checker function.

    `llvm-link protected.bc response_lib.bc -o protected.bc`
- Compile resulting bitcode into a binary

    `clang -rdynamic protected.bc -o out`
