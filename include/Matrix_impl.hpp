#ifndef HH_MATRIX_IMPL_HH
#define HH_MATRIX_IMPL_HH

#include "Matrix.hpp"

using namespace algebra;

//Default Constructor
template <class T, StorageOrder Order>
Matrix<T, Order>::Matrix():
m_size{0}, //initialize the size to 0 rows and columns
m_state{false}//the state of the matrix is initialized to false(uncompressed state)
{}

template <class T, StorageOrder Order>
Matrix<T, Order>::Matrix(unsigned int i, unsigned int j)
{
    m_size[0]=i; //number of rows
    m_size[1]=j; //number of columns
    m_size={0,0};
    m_state=false;
  
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
            for (auto it=m_inner_index.begin(); it!=(m_inner_index.end()-1); ++it){
                for(auto i=*it; i<*(it+1); ++i){
                    m_data.insert({{j,m_outer_index[i]} , m_val[i]});
                }
                ++j;
            }
        }    
    }
    // update the state and clear the vectors of the comprres state for memory saving
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
    // number of rows can be found looking at the last element of the map.
    // the same is true for column-major ordering for the number of columns
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
    if constexpr(Order==StorageOrder::RowWise){
        i=0;
        j=1;
    }else if constexpr(Order==StorageOrder::ColWise){
        i=1;
        j=0;
    }
    auto it=std::find(m_outer_index.begin()+m_inner_index[key[i]], 
                     m_outer_index.begin()+m_inner_index[key[i]+1], key[j]);
    if(it != m_outer_index.begin() +m_inner_index[key[i]+1]){//true if the element is present
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
    //This chenge can appen because modification of non zero elements are allowed with operator()
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
    inner_index.reserve(m_m+1);
    inner_index.emplace_back(0);//first element always zero
    unsigned int temp_idx{0}, key_old{0};
    int i, j;
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
        val.emplace_back(value);

        //outer index is filled with the column indeces if row-major ordering;
        // with the row indeces if column-major ordering
        outer_index.emplace_back(key[i]); 

        //fill the inner_index with the number of elements in a row/column counting until the next row/
        if (key[j]!=key_old)
        {
            inner_index.emplace_back(temp_idx);
            key_old=key[j];
        }

        temp_idx++;
    }
inner_index.emplace_back(temp_idx);
   
m_data.clear(); // clear the map after the compress to avoid waste of memory
//update the state of the matrix
m_state=true;
//update the private variables
m_val=val;
m_inner_index=inner_index;
m_outer_index=outer_index;
}


template<class T, StorageOrder Order>
T
Matrix<T, Order>::at(unsigned int i, unsigned int j) {
    Indices key={i,j};  
    if (!m_state){
        auto it=m_data.find(key);
        if(it != m_data.end()){
            return m_data.at(key); 
        }else{
            std::cerr<<"WARNING! Check bounds: no control on the size of the matrix is made."<<std::endl;
            std::cerr<<"key: [ "<<key[0]<<", "<<key[1]<<" ]"<<std::endl;
            std::cerr<<"Value: ";
        return 0.0;
        }
    }else{
        
        return read_compressed_matrix(key);
    }
}

template<class T, StorageOrder Order>
void
Matrix<T,Order>::erase(unsigned int i, unsigned int j){
    Indices key={i,j};  
    if(!m_state){
        m_data.erase(key);
        }else{
            std::cerr<<"Warning. Trying to delete an element in compressed state."<<std::endl;
            std::cerr<<"No changes. Pass to the uncompressed state before."<<std::endl;
        }
}

template<class T, StorageOrder Order>
bool Matrix<T, Order>::read_market_matrix(const std::string& filename){
    std::ifstream file(filename);
    if(!file.is_open()){
        std::cerr << "WARNING! Error while opening the file in Matrix Market format!"<<std::endl;
        return false;
    }
    std::string line;
    std::getline(file, line);
    if(line.find("%%MatrixMarket")== std::string::npos){
        std::cerr<<"The file is not in Matrix Market format"<<std::endl;
        return false;
    }
    while(std::getline(file, line) and line[0]=='%');
    std::istringstream iss(line);
    iss>>m_size[0]>>m_size[1];

    unsigned int row, col;
    T value;
    while(file>>row>>col>>value){
        Indices key{row-1, col-1};
        m_data.insert({{key, value}});

    }
    file.close();
    return true;
}
template<class T, StorageOrder Order>
T&
Matrix<T, Order>::operator()(const unsigned int k, const unsigned int z){
     Indices key={k,z};  

            if(!m_state){
                auto it=m_data.find(key); 
                if(it != m_data.end()){//true if the key is already present
                    return m_data[key];
                }else{//if the key is not present
                //I add the element in the uncompressed state
                return m_data[key];
                }
             
            }else{
                
                return read_compressed_matrix(key);
            }                
               
}

//Overloading streaming operator
template <class T, StorageOrder Order>
std::ostream& operator<<(std::ostream& out, const Matrix<T, Order>& A)
{
    if(!A.m_state){
        std::cout << "Printing a non compressed matrix" << std::endl;
        for (const auto& pair : A.m_data) {
            out << "[" << pair.first[0] << ", " << pair.first[1] << "]: " << pair.second << "\n";
        }
    }
    else{
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
    if(A.m_state){
        if constexpr(Order==StorageOrder::RowWise){
            T temp;
            output.reserve(A.m_inner_index.size()-1);
            for(unsigned int i = 0; i < A.m_inner_index.size()-1; ++i){
                temp = 0.0;
                for(unsigned int j = A.m_inner_index[i]; j<A.m_inner_index[i+1]; ++j){
                    temp += A.m_val[j] * b[A.m_outer_index[j]];
                }
                output.emplace_back(temp);
            }
            //std::cout<<output[120]<<std::endl;
        }else if constexpr(Order==StorageOrder::ColWise){
            auto max=std::max_element(A.m_outer_index.begin(), A.m_outer_index.end());
            std::vector<T> temp(static_cast<int>(*max)+1,0);
            for(unsigned int i = 0; i < A.m_inner_index.size()-1; ++i){
                for(unsigned int j = A.m_inner_index[i]; j<A.m_inner_index[i+1]; ++j){
                    temp[A.m_outer_index[j]]+= A.m_val[j] * b[i];
                }
            }
            std::ranges::copy(temp.begin(), temp.end(), std::back_inserter(output));

        }
    }else{
        if(A.m_size[0]==0){
            std::cerr<<"ERROR: Resize is compulsory if the matrix is uncompressed"<<std::endl;
        }
        std::vector<T> temp(A.m_size[0],0);

        for (auto [key, value] : A.m_data){
            temp[key[0]]+=value*b[key[1]];
        }
        std::ranges::copy(temp.begin(), temp.end(), std::back_inserter(output));
    }
  

    
    output.shrink_to_fit();
    
    return output;
    
}

#endif // HH_MATRIX_IMPL_HH