#ifndef HH_MATRIX_IMPL_HH
#define HH_MATRIX_IMPL_HH

#include "Matrix.hpp"

using namespace algebra;
/*
//Constructor
template <class T, StorageOrder Order>
Matrix<T, Order>::Matrix():
m_size{0}
{}
*/
template <class T, StorageOrder Order>
Matrix<T, Order>::Matrix(unsigned int i, unsigned int j)
{
    if (Order==StorageOrder::RowWise)
    {
        m_size[0]=i;
        m_size[1]=j;
    }
}
template <class T, StorageOrder Order>
void
Matrix<T, Order>::resize(unsigned int i, unsigned int j){
   if(is_compressed())
   {
        uncompress();
   }
    if (Order== StorageOrder::RowWise)
    {
        m_size[0]=i;
        m_size[1]=j;
    }
}
template<class T, StorageOrder Order>
void
Matrix<T, Order>::uncompress(){
    if (is_compressed())
    {
        if (Order==StorageOrder::RowWise){
            unsigned int j=m_inner_index[0];
            for (auto it=m_inner_index.begin(); it!=m_inner_index.end(); ++it){
                for(auto i=*it; i<*(it+1); ++i){
                    m_data.insert({{j,m_outer_index[i]} , m_val[i]});
                }
                ++j;
            }
        }
        
    }
    m_state=false;
    m_val.clear();
    m_outer_index.clear();
    m_inner_index.clear();
}
//Update Properties
template <class T, StorageOrder Order>
void
Matrix<T, Order>::update_properties()
{
    m_nnz=0;
    m_m=0;

    auto it=m_data.rbegin();
    m_m=it->first[0]+1; // VA CAMBIATO: se inverto l'rdine è first[1]
    m_nnz=m_data.size(); //use the size of the map to retrieve the number of elements 

   
}
//Compress the matrix
template <class T, StorageOrder Order>
void 
Matrix<T, Order>::compress(std::vector<T>           &val,
                    std::vector<unsigned int> &outer_index,
                    std::vector<unsigned int> &inner_index)
{
if (Order == StorageOrder::RowWise){
    //implement CSR
    update_properties();
    val.reserve(m_nnz); // val has the dimension of the number of non zero elements 
    outer_index.reserve(m_nnz); //outer_index has the dimension of the number of NON (???? PER ORA LI PRENDE TUTTI, ANCHE ZERO)zero elements

    inner_index.reserve(m_m+1);// inner_index has the dimension of nrows+1 
    inner_index.emplace_back(0);
    unsigned int temp_idx{0}, key_old{0};
    for (auto [key, value] : m_data)
    {
        std::cout<<"key:" <<key[0]<<" "<<key[1]<<std::endl;
        std::cout<<"values:"<<value<<std::endl;
        val.emplace_back(value);

        outer_index.emplace_back(key[1]);
        
        if (key[0]!=key_old)
        {
            inner_index.emplace_back(temp_idx);
            key_old=key[0];
        }
        temp_idx++;
        //std::cout<<*inner_index.rbegin()<<std::endl;
    }
   inner_index.emplace_back(temp_idx);
}
//else if (order== StorageOrder::ColWise)
    // implement CSC
m_data.clear(); // clear the map after the compress to avoid waste of memory
m_state=true;
m_val=val;
m_inner_index=inner_index;
m_outer_index=outer_index;
}

//Overloading streaming operator
template <class T, StorageOrder Order>
std::ostream& operator<<(std::ostream& out, const Matrix<T, Order>& A)
{
    out<<A.m_data->second <<std::endl;
    return out;
}


#endif // HH_MATRIX_IMPL_HH