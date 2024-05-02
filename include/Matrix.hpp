#ifndef HH_MATRIX_HH
#define HH_MATRIX_HH
#include <map>
#include <array>
#include <vector>
#include <iostream>
#include <functional>
#include <algorithm>
#include <ranges>
#include <sstream>
#include <fstream>
#include <complex>

namespace algebra{
    
    /**
     * @brief //enumerator that indicates the storage ordering
     * 
     */
    enum class StorageOrder{
        RowWise,
        ColWise
    };

    // type alias for the key of the map
    // key is something of the type (i,j) where i is the row index, while j the column one.
    using Indices = std::array<std::size_t, 2>;

    /**
     * @brief functor to specify the ordering relation of the map
     * 
     * @tparam order (row-major or column-major)
     */
    template<StorageOrder order>
    struct CustomCompare{
    };

    /**
     * @brief Specialization of CustonCompare in case of row-major ordering
     * 
     * @tparam  
     */
    template<>
    struct CustomCompare<StorageOrder::RowWise>{
        // In this case I will use the default comparison provided by std::less<T>
        bool operator()(const Indices& a, const Indices& b) const{
         //   return (a[0] < b[0] || (a[0] == b[0] && a[1] < b[1])); // come passare quello di default??
        std::less<Indices> less_than;
        return less_than(a,b);
        }
    };
    /**
     * @brief Specialization of CustonCompare in case of column-major ordering
     * 
     * @tparam  
     */
    template<>
        struct CustomCompare<StorageOrder::ColWise>{
        // I need to change the ordering comparing the column indeces first
        bool operator()(const Indices& a, const Indices& b) const{
            return (a[1] < b[1] || (a[1] == b[1] && a[0] < b[0]));
        }
    };

    // create a type: in COOmap format each elemet is mapped by to integer to which correspond a values
    template <class T, StorageOrder Order>
    using ElemType = std::map<Indices,T, CustomCompare<Order>>;

    /**
    * @brief Class to handle compressed and uncomprees sparse matrix format 
    * 
    */
    //declare the template classe Matrix with partial specialization for the ordering
    template <class T, StorageOrder Order=StorageOrder::RowWise>
    class Matrix {
        
        private:
        T m_dummy_value;
        //map that stores the values accordingly to the StorageOrder
        ElemType<T, Order> m_data; 
        
        // number of non-zero elements of the map
        std::size_t m_nnz;

        // number of non empy rows (if row-major ordering) 
        // or columns (if column-major ordering) 
        std::size_t m_m; 
        
        // size of the matrix
        // m_size[0] = number of rows
        // m_size[1] = number of columns
        std::array<std::size_t,2> m_size; 

        // State of the matrix can be compressed (CSR or CSC) or uncompressed
        bool m_state; // true if compressed

        //Vector that stores the data of the matrix when compressed
        std::vector<T>           m_val;
        /*We store two vectors of indexes. The first (the inner indexes),
        of length the number of rows plus one, contains the starting index for the elements of
        each row. The second vector of indexes (the outer indexes), of length the number of non-
        zeroes, contains the corresponding column index. Finally, we have a vector of values,
        again of length the number of non-zeroes. To be more specific, the elements of row i are
        stored in the elements of the vector of values in the interval inner(i) ≤ k < inner(i + 1)
        (the interval is open on the right) and the corresponding column index is outer(k).
        Using this scheme, we have a row-wise storage, since transversing the vector of values
        provides the non-zero elements ”by row”.*/
        std::vector<unsigned int> m_outer_index;
        std::vector<unsigned int> m_inner_index;

        // utility to update some private variables of the class
        void 
        update_properties();

        /**
         * @brief Get the zero object
         * 
         * @return T 
         */
        T
        get_zero();

        //method to read the value when the matrix is compressed, given a key 
        T&
        read_compressed_matrix(const Indices& key);
        public:
        //The default constructor
        Matrix();
        // constuctor that takes the size of the matrix
        Matrix(unsigned int i, unsigned int j); 
        
        /**
         * @brief return the size of the matrix
         * 
         */
        inline Indices
        size() const{
            return m_size;
        }
        /**
         * @brief method to interrogate the state of the matrix (true if compressed)
         * 
         * @return true if compressed (CSR or CSC)
         * @return false if uncompressed (COOmap format)
         */
        inline bool 
        is_compressed(){
            return m_state;
        }
        /**
         * @brief resize the matrix according given dimensions 
         * 
         * @param i number of rows 
         * @param j number of columns
         */
        
        void 
        resize(unsigned int i, unsigned int j);

        /**
         * @brief useful method for update the vector of values when one of them has been modified
         * 
         * @param val vector of values
         */
        void
        update_compressed_values(std::vector<T>   &val);
        /**
         * @brief The metod allows to uncompress a matrix from CSR/CSC to COOmap format
         * 
         */
        void 
        uncompress();

        /**
         * @brief This method allows the compression from COOmap format to a compressed format
         *          CSR or CSC
         * @param val vector of non-zero values of the sparse matrix 
         * @param outer_index vector containing the indeces of the colums/rows on the non-zero elements
         * @param inner_index vector containing the index indicating in the other vector where a new row/column starts
         */
        void 
        compress(std::vector<T>   &val,
        std::vector<unsigned int> &outer_index,
        std::vector<unsigned int> &inner_index);

        /**
         * @brief method to read the matrix provided a specific key
         * 
         * @param i  row index
         * @param j column index
         * @return const T read value
         */
        T at(unsigned int i, unsigned int j) ;

        /**
         * @brief delete an element previously inserted
         * 
         * @param i row index
         * @param j columns index
         */
        void 
        erase(unsigned int i, unsigned int j);

        /**
         * @brief This method read a matrix in Matrix Market format (.mtx)
         * 
         * @param filename 
         * @return true if the file has been read successfully
         * @return false if the reading has failed
         */
        bool
        read_market_matrix(const std::string& filename);
        /**
         * @brief the call operator() must be used for inserting values in a key={i,j}
         * 
         * @param i index of the row 
         * @param j index of the columns
         * @return T& value to be inserted
         */
        
        T&
        operator()(const unsigned int k, const unsigned int z);

        //Streaming operator overloading
        template<class U, StorageOrder order>
        friend std::ostream& 
        operator<<(std::ostream& out, const Matrix<U, order>& A);

        /**
         * @brief Matrix-vector product. Matrix can be compressed or uncompressed. 
         *  If uncompressed, resize is compulsory
         * 
         * @param A Matrix(compressed or uncompressed)
         * @param b vector
         * @return template<class U, StorageOrder order> 
         */
        //operator* overloading
        template<class U, StorageOrder order>
        friend std::vector<U> 
        operator*(const Matrix<U, order> &A,const std::vector<U> &b);


    };

// include the implementation
#include "Matrix_impl.hpp"
}// namespace algebra


#endif// HH_MATRIX_HPP