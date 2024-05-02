/**
 * @file main.cpp
 * @author Giuseppe Caliò (you@domain.com)
 * @brief This file is the actual program, with tests and computations
 * @version 0.1
 * @date 2024-04-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <iostream>
#include "Matrix.hpp"
#include "chrono.hpp"
#include <map>
#include <array>
#include <vector>
#include <utility>
int main()
{
    using namespace algebra;
    //////////////////////////////////////////////////////
    //****************** TEST 1*************************//
    //////////////////////////////////////////////////////
    // in this test I will fill a sparse matrix A considering a row-major ordering
    const StorageOrder Order=StorageOrder::RowWise;
    // A can be built without specifying the size
    Matrix<double, Order> A;
    // a resize is possible, but not compulsory, except for the matrix-vector product
    // if the matrix is uncompressed
    //A.resize(4,4)
    
    // It is possible to construct the matrix with the right dimension directly
    //Matrix<double, Order> A(4,4);

    const unsigned int n{4};
    for (unsigned int i = 0; i < n; ++i)
    {
      if (i > 0)
        A(i,i-1)= -1;

      if (i < n - 1)
        A(i,i+1) = -1;

      A(i,i) = 4;
    }
    //I can print the matrix
    std::cout<<A;
    //I can modify an existing element with the call operator. 
    //A warning will be printed
    A(0,0)=12;

    //It is possible to read the matrix with the call operator but it is UNSAFE
    std::cout<<"Value in (0,0): "<<A(0,0)<<std::endl;
    //indeed if the element is not present, a 0 will be added to the map.
    // to read the matrix, if you are not sure if the key is present or not
    // prefer the at() method.
    std::cout<<"Value in (1,3): "<<A(1,3)<<std::endl; // a zero will be added in the map and stored (undesired)
    
    A.erase(1,3); // I can remove an element if the matrix is uncompressed
    //If compressed erase will have no effect and a warning will be printed
    

    // The at method will print a worning if the key is not present. This is because
    // the method does not check if the key is within the dimension of the matrix and it will print
    // in any case 0
    //if key is (1,3) (within the bounds of a 4*4 matrix) I have:
    std::cout<<A.at(1,3)<<std::endl;
    //if key is (100, 100) (outside the bounds of a 4*4 matrix) I still have:
    std::cout<<A.at(100,100)<<std::endl;
    //If the matrix is compressed this call would cause a segmentation fault.

    //NOTE: if a zero is inserted no checks are made. Be careful!

    //now I will compress the matrix
    std::vector<double>       val;
    std::vector<unsigned int> col_ind, row_ptr;

    
  A.compress(val, col_ind, row_ptr);
  // If the matrix is compressed I can read it with the at method
  // I can try to read also with the call operato but it is unsafe.
  // If the key is present all works fine, if not the call will cause an abort
  //to avoid a modification of the map in compressed state. 
  //In this state only non-zero entries can be modified

  


  //std::cout<<"Attempt: "<<A(10,70)<<std::endl; // In compressed state a key
  //greater than the matrix will cause a segmentation fault. Be careful.
  std::cout<<"Reading with call operator:" <<A(1,3)<<std::endl;
  std::cout<<"Reading with at method:" <<A.at(1,3)<<std::endl;//prefer at()
  A(1,3)=5;//This call will have no effect
  //indeed E(1,3) will be 0+i0;
  std::cout<<"A(1,3) is still: "<<A(1,3)<<std::endl;
  A(0,0)=5; //OK, the element will be modified
  std::cout<<"New value of A(0,0): "<<A(0,0)<<std::endl;
   std::cout<<"New value of A(0,0): "<<A.at(0,0)<<std::endl;
  A(0,0)=4; // back to the previous value
  std::cout<<"New value of A(0,0): "<<A(0,0)<<std::endl;
  std::cout<<"New value of A(0,0): "<<A.at(0,0)<<std::endl;

  //With the previous call we have modified the private variables of the matrix.
  //call the update method to update val
  A.update_compressed_values(val);
  std::cout << "CSR vectors:" << std::endl;
  for (auto i : val)
    std::cout << i << " ";
  std::cout << std::endl;

  for (auto i : col_ind)
    std::cout << i << " ";
  std::cout << std::endl;

  for (auto i : row_ptr)
    std::cout << i << " ";
  std::cout << std::endl << std::endl;

  std::cout << "Matrix entries from CSR:" << std::endl;
  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = row_ptr[i]; j < row_ptr[i + 1]; ++j)
      std::cout << "A[" << i << "][" << col_ind[j] << "] = " << val[j]
                << std::endl;
  std::cout << std::endl;

  // I can check if the matrix is compreed
  if (A.is_compressed())
  {
    std::cout<<"è compressa"<<std::endl;
  }

  //I can uncompress the matrix
  A.uncompress();
  //and check again the state
  if(!A.is_compressed()){
    std::cout<<"non è compressa"<<std::endl;
  }
  //////////////////////////////////////////
  //************Product*******************//
  //////////////////////////////////////////

  std::vector<double>       val2;
  std::vector<unsigned int> col_ind2, row_ptr2;

    
  A.compress(val2, col_ind2, row_ptr2);
  A(0,0)=4;
  A.update_compressed_values(val2);

  std::vector<double> b{3.0,4.0,2.0,7.0};
  std::vector<double> prod=A*b;
  
  //Printing the result of the product
  std::cout<<"The result of the product with CSR matrix is: "<<std::endl;
  for (auto it=prod.begin();it!=prod.end();++it){
    std::cout<<*it<<std::endl;
  }
  
  /////////////////////////////////////////////
  //************Col-Major Ordering***********//
  /////////////////////////////////////////////
  const StorageOrder order=StorageOrder::ColWise;
  Matrix<double, order> B;
  for (unsigned int i = 0; i < n; ++i)
    {
      if (i > 0){
        B(i,i-1)= -1;
      }
      if (i < n - 1){
        B(i,i+1) = -1;
      }
      B(i,i) = 4;
    }
  std::vector<double> b2{1.0,2.0,3.0,4.0};
  std::vector<double>       val3;
  std::vector<unsigned int> outer,inner;
  std::vector<double> prod_uncompress;
  //Note that if the matrix is uncompressed the resize is compulsory
  //otherwise the program will abort with an error message
  B.resize(4,4);
  //Test the product when the matrix is uncompress
  prod_uncompress=B*b2;
  std::cout<<"The result of the product, with the uncompressed matrix, is: "<<std::endl;
  for (auto ite=prod_uncompress.begin();ite!=prod_uncompress.end();++ite){
    std::cout<<*ite<<std::endl;
  }
  B.compress(val3,outer,inner);
  std::cout << "Matrix entries from CSC:" << std::endl;
  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = inner[i]; j < inner[i + 1]; ++j)
      std::cout << "B[" << outer[j] << "][" << i << "] = " << val3[j]
                << std::endl;
  std::vector<double> prod2=B*b2;

  std::cout<<"The result of the product, with the compressed matrix(CSC), is: "<<std::endl;
  for (auto ite=prod2.begin();ite!=prod2.end();++ite){
    std::cout<<*ite<<std::endl;
  }
  
  ///////////////////////////////////////////////////////////////
  /***************Read from a file in a Market Matrix Format*/
  Matrix<double> C;
  Timings::Chrono     clock_compressed, clock_uncompressed;
  std::vector<double> c(131,5.0), prod_mark_compressed, prod_mark_uncompressed;
  std::string filename("./data/matrix.mtx");
  C.read_market_matrix(filename);
  C.resize(131,131);//compulsory resize when uncompressed
  clock_uncompressed.start();
  prod_mark_uncompressed=C*c;
  clock_uncompressed.stop();
  std::cout << "Uncompressed case(Coomap). "<<clock_uncompressed;
  //std::cout<<C.at(9,5)<<" "<<C.at(38,13)<<std::endl;
  std::vector<double>       val_market;
  std::vector<unsigned int> outer_market,inner_market;
  C.compress(val_market, outer_market, inner_market);
  // std::cout<<val_market[0];


  clock_compressed.start();
  prod_mark_compressed=C*c;
  clock_compressed.stop();
  std::cout << "Compressed case(CSR). "<<clock_compressed;

  const StorageOrder ordering=StorageOrder::ColWise;
  Matrix<double,ordering > D;
  D.read_market_matrix(filename);
  std::vector<double>       val_market2;
  std::vector<unsigned int> outer_market2,inner_market2;
  D.resize(131,131);
  D.compress(val_market2, outer_market2, inner_market2);
  std::vector<double> prod_mark_compressed_csc;
  Timings::Chrono clock_compressed_csc;
  clock_compressed_csc.start();
  prod_mark_compressed_csc=D*c;
  clock_compressed_csc.stop();
  std::cout << "Compressed case(CSC). "<<clock_compressed_csc;

/////////////////////////////////////////////////////////////
/************************COMPLEX NUMBERS*********************/
////////////////////////////////////////////////////////////
//Fill the matrix as before
  Matrix<std::complex<double>> E;
  std::complex<double> z1(-1,1),z2(4,4);
  for (unsigned int i = 0; i < n; ++i)
    {
      if (i > 0){
        E(i,i-1)= z1;
      }
      if (i < n - 1){
        E(i,i+1) = z1;
      }
      E(i,i) = z2;
    }
  //Read the matrix
  std::cout<<"E(0,0)= "<<E.at(0,0)<<std::endl;
  //Read with the call operato (UNSAFE)
  std::cout<<"Read with call operator matrix E="<<E(1,3)<<std::endl;
  E.erase(1,3); //Erase the element
  std::cout<<"Read with at() method matrix E "<<std::endl;
  std::cout<<E.at(1,3)<<std::endl;// Read Again

  //Compress the matrix, ROW_MAJOR ordering by default
  std::vector<std::complex<double>>       val_complex;
  std::vector<unsigned int> outer_complex,inner_complex;

  E.compress(val_complex, outer_complex, inner_complex);
  std::cout<<"Complex matrix E:"<<std::endl;
  std::cout<<E<<std::endl;
  
  //Read again when it is compressed
  std::cout<<"Read with call operator matrix E "<<std::endl;
  std::cout<<E(1,3)<<std::endl;
  std::cout<<"Read with at() method matrix E "<<std::endl;
  std::cout<<E.at(1,3)<<std::endl;// Read Again
  if(E.is_compressed()){
    std::cout<<"Complex matrix compressed"<<std::endl;
  }
  //Test the product with CSR matrix
  {
  std::vector<std::complex<double>> prod_complex;
  std::vector<std::complex<double>> complex{z1,z2,z1,z2};
  Timings::Chrono clock_complex;
  clock_complex.start();
  prod_complex=E*complex;
  clock_complex.stop();
  std::cout<<"The result of the product, with the compressed complex matrix(CSR), is: "<<std::endl;
  for (auto ite=prod_complex.begin();ite!=prod_complex.end();++ite){
    std::cout<<*ite<<std::endl;
  }
  std::cout << "Compressed case(CSR) with complex matrix. "<<clock_complex;
  }
  //Test the uncompression and the product
  E.uncompress();
  if(!E.is_compressed()){
    std::cout<<"Complex matrix uncompressed"<<std::endl;
  }

  {
  std::vector<std::complex<double>> prod_complex;
  std::vector<std::complex<double>> complex{z1,z2,z1,z2};
  E.resize(4,4);
  Timings::Chrono clock_complex;
  clock_complex.start();
  prod_complex=E*complex;
  clock_complex.stop();

 
  std::cout<<"The result of the product, with the uncompressed complex matrix, is: "<<std::endl;
  for (auto ite=prod_complex.begin();ite!=prod_complex.end();++ite){
    std::cout<<*ite<<std::endl;
  }
   std::cout << "Uncompressed case with complex matrix. "<<clock_complex;
  }

  //now CSC compression
  const StorageOrder complex_ordering=StorageOrder::ColWise;
  Matrix<std::complex<double>, complex_ordering> F;

  for (unsigned int i = 0; i < n; ++i)
    {
      if (i > 0){
        F(i,i-1)= z1;
      }
      if (i < n - 1){
        F(i,i+1) = z1;
      }
      F(i,i) = z2;
    }
  std::vector<std::complex<double>>       val_complex_csc;
  std::vector<unsigned int> outer_complex_csc,inner_complex_csc;

  F.compress(val_complex_csc, outer_complex_csc, inner_complex_csc);
  {
  std::vector<std::complex<double>> prod_complex;
  std::vector<std::complex<double>> complex{z1,z2,z1,z2};
  Timings::Chrono clock_complex;
  clock_complex.start();
  prod_complex=F*complex;
  clock_complex.stop();
  std::cout<<"The result of the product, with the compressed complex matrix(CSC), is: "<<std::endl;
  for (auto ite=prod_complex.begin();ite!=prod_complex.end();++ite){
    std::cout<<*ite<<std::endl;
  }
  std::cout << "Compressed case(CSC) with complex matrix. "<<clock_complex;
  }
  return 0;
}
