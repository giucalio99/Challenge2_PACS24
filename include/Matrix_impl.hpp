#ifndef HH_MATRIX_IMPL_HH
#define HH_MATRIX_IMPL_HH

#include "Matrix.hpp"

using namespace algebra;

//Default Constructor 
template <class T, StorageOrder Order>
Matrix<T, Order>::Matrix():
m_size{0},        //initialize the size to 0 rows and columns
m_state{false}    //the state of the matrix is initialized to false(uncompressed state)
{}

template <class T, StorageOrder Order>
Matrix<T, Order>::Matrix(unsigned int i, unsigned int j)
{
    m_size[0]=i;  //number of rows
    m_size[1]=j;  //number of columns
    m_size={0,0}; //initialize the size to 0 rows and columns
    m_state=false;//the state of the matrix is initialized to false(uncompressed state)
  
}

template <class T, StorageOrder Order>
void
Matrix<T, Order>::resize(unsigned int i, unsigned int j){
   // check the state of matrix, and uncompress if it is compressed 
   if(this->is_compressed())
   {
        uncompress();
   }
   //assign the new value to the number of rows and columns
    m_size[0]=i;
    m_size[1]=j;
}
template<class T, StorageOrder Order>
void
Matrix<T, Order>::uncompress(){
    if (is_compressed()){
        if (Order==StorageOrder::RowWise){
            unsigned int j=m_inner_index[0];
            //fill the map with the elements of the compressed matrix
            for (auto it=m_inner_index.begin(); it!=(m_inner_index.end()-1); ++it){
                for(auto i=*it; i<*(it+1); ++i){
                    m_data.insert({{j,m_outer_index[i]} , m_val[i]});//insert the element in the map
                }
                ++j;//increment the row index
            }
        }    
    }
    // update the state and clear the vectors of the comprres state for memory saving
    m_state=false;
    m_val.clear();
    m_outer_index.clear();
    m_inner_index.clear();
}
//Update Properties of the matrix
template <class T, StorageOrder Order>
void
Matrix<T, Order>::update_properties()
{
    m_nnz=0; //initialize the number of non zero elements to 0
    m_m=0;   //initialize the number of non empty rows/columns to 0

    auto it=m_data.rbegin(); //get the last element of the map
    // The number of rows can be found looking at the last element of the map.
    // The same is true for column-major ordering for the number of columns.
    

    if constexpr(Order==StorageOrder::RowWise){
        m_m=it->first[0]+1;
    }else if constexpr(Order==StorageOrder::ColWise){
        m_m=it->first[1]+1;
    }
    m_nnz=m_data.size(); //use the size of the map to retrieve the number of elements 
}

template <class T, StorageOrder Order>
T
Matrix<T, Order>::get_zero()
{
    static T zeroValue;
    return zeroValue;
}

template <class T, StorageOrder Order>
T&
Matrix<T, Order>::read_compressed_matrix(const Indices& key){
    int i, j;
    //check the order of the storage
    if constexpr(Order==StorageOrder::RowWise){
        i=0;
        j=1;
    }else if constexpr(Order==StorageOrder::ColWise){
        i=1;
        j=0;
    }
    //search the element in the outer_index vector
    auto it=std::find(m_outer_index.begin()+m_inner_index[key[i]], 
                     m_outer_index.begin()+m_inner_index[key[i]+1], key[j]);
    //if the element is present in the outer_index vector
    //I can find the value in the m_val vector
    if(it != m_outer_index.begin() +m_inner_index[key[i]+1]){
        std::size_t index=it-m_outer_index.begin();
        return m_val[index];
    }else{
        //std::cerr<<"Warning:Matrix is compressed: reading only"<<std::endl;
        m_dummy_value=get_zero();//if the element is not present I will return 0
        return m_dummy_value;
    }      
}
template<class T, StorageOrder Order>
void
Matrix<T, Order>::update_compressed_values(std::vector<T>    &val)
{
    val=m_val;//update val after a change of m_val.
    //This chenge can happen because modification of non zero elements are allowed with operator()
}
//Compress the matrix 
template <class T, StorageOrder Order>
void 
Matrix<T, Order>::compress(std::vector<T>             &val,
                    std::vector<unsigned int> &outer_index,
                    std::vector<unsigned int> &inner_index)
{
    update_properties();

    // val has the dimension of the number of the elements in the map
    val.reserve(m_nnz); 
    //outer_index has the dimension of the number of the elements in the map
    outer_index.reserve(m_nnz); 

    // inner_index has the dimension of nrows+1 or ncols+1
    inner_index.reserve(m_m+1);//initialize the inner_index with the number of rows+1
    inner_index.emplace_back(0);//first element always zero
    unsigned int temp_idx{0}, key_old{0};
    int i, j;
    //check the order of the storage
    if constexpr(Order==StorageOrder::RowWise){
        i=1;
        j=0;
    }else if constexpr(Order==StorageOrder::ColWise){
        i=0;
        j=1;
    }
    // traversing the map and fill the vector that represents the matrix in
    // the compressed state
    for (auto [key, value] : m_data)
    {
        val.emplace_back(value);//fill the val vector with the values of the map

        //outer index is filled with the column indeces if row-major ordering;
        // with the row indeces if column-major ordering
        outer_index.emplace_back(key[i]); 

        //fill the inner_index with the number of elements in a row/column counting until the next one
        //if the row/column is different from the previous element the temp_idx is inserted in the inner_index and
        // the key_old is updated
        if (key[j]!=key_old)
        {
            inner_index.emplace_back(temp_idx);
            key_old=key[j];
        }
        //if the row/column is the same of the previous element the counter is incremented
        temp_idx++;
    }
inner_index.emplace_back(temp_idx);
   
m_data.clear(); // clear the map after the compress to avoid waste of memory 
m_state=true;   //update the state of the matrix

//update the private variables of the class
m_val=val;
m_inner_index=inner_index;
m_outer_index=outer_index;
}


template<class T, StorageOrder Order>
T
Matrix<T, Order>::at(unsigned int i, unsigned int j) {
    Indices key={i,j};  
    //check the state of the matrix
    if (!m_state){
        //if the matrix is in the uncompressed state
        //I can use the find method of the map to search the element with key
        auto it=m_data.find(key);
        if(it != m_data.end()){
            //if the element is present I return the value
            return m_data.at(key); 
        }else{
            //if the element is not present I return 0
            std::cerr<<"WARNING! Check bounds: no control on the size of the matrix is made."<<std::endl;
            std::cerr<<"key: [ "<<key[0]<<", "<<key[1]<<" ]"<<std::endl;
            std::cerr<<"Value: ";
        return 0.0;
        }
    }else{
        //if the matrix is in the compressed state
        //I can use the read_compressed_matrix method to search the element with key
        return read_compressed_matrix(key);
    }
}

template<class T, StorageOrder Order>
void
Matrix<T,Order>::erase(unsigned int i, unsigned int j){
    Indices key={i,j};  
    //check the state of the matrix
    if(!m_state){
        //if the matrix is in the uncompressed state
        //I can use the erase method of the map to delete the element with key
        m_data.erase(key);
        }else{
            //if the matrix is in the compressed state
            //I will do nothing and print a warning message
            std::cerr<<"Warning. Trying to delete an element in compressed state."<<std::endl;
            std::cerr<<"No changes. Pass to the uncompressed state before."<<std::endl;
        }
}

template<class T, StorageOrder Order>
bool Matrix<T, Order>::read_market_matrix(const std::string& filename){
    std::ifstream file(filename);//open the file
    if(!file.is_open()){
        //if the file is not open print a warning message
        std::cerr << "WARNING! Error while opening the file in Matrix Market format!"<<std::endl;
        return false;
    }
    std::string line;
    std::getline(file, line);//read the first line of the file
    if(line.find("%%MatrixMarket")== std::string::npos){
        //if the file is not in Matrix Market format print a warning message
        std::cerr<<"The file is not in Matrix Market format"<<std::endl;
        return false;
    }
    //read the size of the matrix
    while(std::getline(file, line) and line[0]=='%');
    std::istringstream iss(line);
    iss>>m_size[0]>>m_size[1];

    //read the non zero elements of the matrix
    //and fill the map with the elements
    unsigned int row, col;
    T value;
    while(file>>row>>col>>value){
        Indices key{row-1, col-1};
        m_data.insert({{key, value}});

    }
    file.close();//close the file
    return true;
}
template<class T, StorageOrder Order>
T&
Matrix<T, Order>::operator()(const unsigned int k, const unsigned int z){
     Indices key={k,z};  
    //check the state of the matrix
            if(!m_state){
                //if the matrix is in the uncompressed state
                //I add the element in the map representing the
                //matrix in the uncompressed state
                return m_data[key];
            }else{
                //if the matrix is in the compressed state
                //I can use the read_compressed_matrix method to search the element with key
                return read_compressed_matrix(key);
            }
}

//Overloading streaming operator
template <class T, StorageOrder Order>
std::ostream& operator<<(std::ostream& out, const Matrix<T, Order>& A)
{
    //check the state of the matrix
    if(!A.m_state){
        //if the matrix is in the uncompressed state
        //I can print the elements of the map
        std::cout << "Printing a non compressed matrix" << std::endl;
        for (const auto& pair : A.m_data) {
            out << "[" << pair.first[0] << ", " << pair.first[1] << "]: " << pair.second << "\n";
        }
    }
    else{
        //if the matrix is in the compressed state
        //I can print the elements of the vectors
        if constexpr(Order==StorageOrder::RowWise){
            std::cout << "Printing a CSR matrix" << std::endl;
            for (unsigned int i = 0; i < A.m_inner_index.size()-1; ++i)
                for (unsigned int k = A.m_inner_index[i]; k < A.m_inner_index[i + 1]; ++k)
                    out << "[" << i << ", " << A.m_outer_index[k] << "] = " << A.m_val[k] << "\n";
        }
        else if constexpr(Order==StorageOrder::ColWise){
            std::cout << "Printing a CSC matrix" << std::endl;
            for (unsigned int i = 0; i < A.m_inner_index.size()-1; ++i)
                for (unsigned int k = A.m_inner_index[i]; k < A.m_inner_index[i + 1]; ++k)
                    out << "[" << A.m_outer_index[k] << ", " << i << "] = " << A.m_val[k] << "\n";
        }
    }
    return out;
}

//Overload operator* for Matrix-vector multiplication
template<class T, StorageOrder Order>
std::vector<T> operator*(const Matrix<T, Order> &A, const std::vector<T> &b){

    std::vector<T> output;
    //check the state of the matrix
    if(A.m_state){
        //if the matrix is in the compressed state
        //I apply the matrix-vector multiplication using the compressed representation
        //differentiating the implementation for row-wise and column-wise storage.

        //If the storage is row-wise I loop over the rows of the matrix
        if constexpr(Order==StorageOrder::RowWise){
            T temp;
            output.reserve(A.m_inner_index.size()-1);//reserve the space for the output vector
            for(unsigned int i = 0; i < A.m_inner_index.size()-1; ++i){
                temp = 0.0;
                //loop over the elements of the row
                for(unsigned int j = A.m_inner_index[i]; j<A.m_inner_index[i+1]; ++j){
                    //multiply the element of the matrix by the corresponding element of the vector
                    temp += A.m_val[j] * b[A.m_outer_index[j]];
                }
                output.emplace_back(temp);//add the result to the output vector
            }
        }else if constexpr(Order==StorageOrder::ColWise){
            
            auto max=std::max_element(A.m_outer_index.begin(), A.m_outer_index.end());
            std::vector<T> temp(static_cast<int>(*max)+1,0);//initialize a temporary vector with the size of the number of rows
            for(unsigned int i = 0; i < A.m_inner_index.size()-1; ++i){
                for(unsigned int j = A.m_inner_index[i]; j<A.m_inner_index[i+1]; ++j){
                    temp[A.m_outer_index[j]]+= A.m_val[j] * b[i];
                }
            }
            std::ranges::copy(temp.begin(), temp.end(), std::back_inserter(output));//copy the result in the output vector

        }
    }else{
        if(A.m_size[0]==0){
            //if the matrix is in the uncompressed state and the number of rows is 0
            //I print an error message
            std::cerr<<"ERROR: Resize is compulsory if the matrix is uncompressed"<<std::endl;
        }
        std::vector<T> temp(A.m_size[0],0);//initialize a temporary vector with the size of the number of rows
        //loop over the elements of the matrix and multiply the element of the matrix by the corresponding element of the vector
        for (auto [key, value] : A.m_data){
            temp[key[0]]+=value*b[key[1]];
        }
        std::ranges::copy(temp.begin(), temp.end(), std::back_inserter(output));//copy the result in the output vector
    }
  

    
    output.shrink_to_fit();//shrink the output vector
    
    return output;
    
}

#endif // HH_MATRIX_IMPL_HH