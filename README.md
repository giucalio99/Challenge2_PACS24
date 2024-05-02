# Challenge2_PACS24_10582530
## Configuration
The makefile is already working and no changes are require.
## Building
In order to compile type:
```
make
```
or
make all

To clean the directory type
make clean
## Execution
A lot of example of the usage of the class Matrix has been shown in the main program. 
You will learn how:
1. Istantiate an object Matrix<T, Order> and fill it;
2. Read the values and performed the Matrix-vector operation in uncompressed state;
3. Compress, uncompress the matrix and interrogate the state;
4. Read the valuee and performed the Matrix-vector operation in the compressed state;
5. Read a matrix in Matrix Market format;
6. Evaluate and compare the performance of operation on the matrix while in different states (compressed or not);
7. Play with a matrix of complex numbers;


## Documetation
In the doc folder a doxyfile is present. Type
doxygen
to have the doxygenated documantation