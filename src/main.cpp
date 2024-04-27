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
#include <map>
#include <array>
#include <vector>
#include <utility>
int main()
{
    using namespace algebra;
    const StorageOrder Order=StorageOrder::RowWise;
    Matrix<double, Order> A;
    const unsigned int n{4};
    for (unsigned int i = 0; i < n; ++i)
    {
      if (i > 0)
        A(i,i-1)= -1;

      if (i < n - 1)
        A(i,i+1) = -1;

      A(i,i) = 4;
    }
    
    //A.resize(4,4);
    //unsigned int row, col;
    //row=A.size()[0];
    //col=A.size()[1];
    //std::cout<<row<<" "<<col<<std::endl;
    A(0,0)=7;
    double a=A(1,3);
    std::cout<<a<<"\n";
    A(0,0)=12;
    std::cout<<A(0,0)<<"\n";
    //std::cout<<A(2,3)<<"\n";
    //std::cout<<A(3,3)<<"\n";
    //std::cout<<A(1,3)<<"\n"; REMEMBER WE NEED TO HANDLE THE INSERTION IN THE MAP
    //Gestire il caso in cui l'utente inserisca uno zero
    //Gestire il caso in cui si inserisca un elemento dove è già presente
   
    std::vector<double>       val;
    std::vector<unsigned int> col_ind, row_ptr;

    
    A.compress(val, col_ind, row_ptr);
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

  if (A.is_compressed())
  {
    std::cout<<"è compressa"<<std::endl;
  }

  
  A.uncompress();
  if(!A.is_compressed()){
    std::cout<<"non è compressa"<<std::endl;
  }
  std::vector<double>       val2;
  std::vector<unsigned int> col_ind2, row_ptr2;

    
  A.compress(val2, col_ind2, row_ptr2);

  std::vector<double> b{3.0,4.0,2.0,7.0};
  std::vector<double> prod=A*b;

  auto it=prod.end();
  std::cout<<"test:"<<*(it-1)<<std::endl;  
  for (auto it=prod.begin();it!=prod.end();++it){
    std::cout<<*it<<std::endl;
  }
  
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
  B.compress(val3,outer,inner);
  std::cout << "Matrix entries from CSC:" << std::endl;
  for (unsigned int i = 0; i < n; ++i)
    for (unsigned int j = inner[i]; j < inner[i + 1]; ++j)
      std::cout << "A[" << outer[j] << "][" << i << "] = " << val3[j]
                << std::endl;
  std::vector<double> prod2=B*b2;


  for (auto ite=prod2.begin();ite!=prod2.end();++ite){
    std::cout<<*ite<<std::endl;
  }
  
  return 0;
}
