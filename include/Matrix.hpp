#ifndef HH_MATRIX_HH
#define HH_MATRIX_HH
#include <map>
#include <array>
#include <vector>
#include <iostream>
namespace algebra{

    //enumerator that indicates the storage ordering
    enum class StorageOrder{
        RowWise,
        ColWise
    };
    // create a type: in COOmap format each elemet is mapped by to integer to which correspond a values
    template <class T>
    using ElemType = std::map<std::array<std::size_t,2>,T>;

    //declare the template classe Matrix with partial specialization for the ordering
    template <class T, StorageOrder Order=StorageOrder::RowWise>
    class Matrix {
        
        private:
        
        ElemType<T> m_data; //map that stores the values
        size_t m_nnz; // number of elements of the map
        size_t m_m; // number of non empy columns/rows
        std::array<std::size_t,2> m_size; // size of the matrix
        bool m_state; // true if compressed

        //Vector to store the data of the matrix when compressed
        std::vector<T>           m_val;
        std::vector<unsigned int> m_outer_index;
        std::vector<unsigned int> m_inner_index;

        public:
        Matrix()=default;
        Matrix(unsigned int i, unsigned int j); // constuctor that takes the size of the matrix
        
        // return the size of the matrix
        inline std::array<std::size_t,2>  size() const{
            return m_size;
        }
        //resize the matrix according given dimensions
        void resize(unsigned int i, unsigned int j);

        // 
        void update_properties();
        
        //method to interrogate the state of the matrix (true if compressed)
        inline bool is_compressed(){
            return m_state;
        }

        /**
         * @brief The metod allows to uncompress a matrix from CSR/CSC to COOmap format
         * 
         */
        void uncompress();

        /**
         * @brief This method allows the compression from COOmap format to a compressed format
         *          CSR or CSC
         * @param val vector of non-zero values of the sparse matrix 
         * @param outer_index vector containing the indeces of the colums/rows on the non-zero elements
         * @param inner_index vector containing the index indicating in the other vector where a new row/column starts
         */
        // Compress method
        void 
        compress(std::vector<T>   &val,
        std::vector<unsigned int> &outer_index,
        std::vector<unsigned int> &inner_index);


        // const and non const version of the call operator
        // inline const T &operator()(unsigned int i, unsigned int j){
        //}
        inline T &operator()(unsigned int i, unsigned int j){
            std::array<std::size_t,2> key={i,j};  
            return m_data[key];
        }
        //Streaming operator overloading
        template<class U, StorageOrder order>
        friend std::ostream& operator<<(std::ostream& out, const Matrix<U, order>& A);


    }; 
// include the implementation
#include "Matrix_impl.hpp"
//extern template std::ostream& operator<<(std::ostream& out, const Matrix<double, StorageOrder::RowWise>& A);
}// namespace algebra




#endif// HH_MATRIX_HPP