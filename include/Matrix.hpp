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

        // utility to update some private variables of the class
        void 
        update_properties();
        


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
        T at(unsigned int i, unsigned int j) const;

        /**
         * @brief delete an element previously inserted
         * 
         * @param i row index
         * @param j columns index
         */
        void 
        erase(unsigned int i, unsigned int j);

        
        bool
        read_market_matrix(const std::string& filename);
        /**
         * @brief the call operator() must be used for inserting values in a key={i,j}
         * 
         * @param i index of the row 
         * @param j index of the columns
         * @return T& value to be inserted
         */
        inline T& operator()(unsigned int i, unsigned int j){

            Indices key={i,j}; 
            auto it=m_data.find(key);
            if(it != m_data.end()){//true if the key is already present
                std::cerr<<"\n Warning: Matrix is uncompressed: modyfing existing element."<<std::endl;
               return m_data[key];
            }else{ //if the key is not present
                if(!m_state){
                    return m_data[key];
                }else{
                    std::cerr<<"Warning:trying to add an element in compressed state. Aborting."<<std::endl;
                }
            }
        }

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

        /*
        template<class U,algebra::StorageOrder order>
        struct std::less<algebra::ElemType<U>>{
        bool operator()(algebra::ElemType<U> &lhs, algebra::ElemType<U> &rhs){
        bool comp;
        // lhs<rhs
        if (order==algebra::StorageOrder::ColWise){
            comp=(lhs.first[1] < rhs.first[1]) || (lhs.first[1] == rhs.first[1] && lhs.first[0] < rhs.first[0])
        }else{
            comp=std::lexicographical_compare(lhs,rhs);
        }
        return comp;*/
    };

// include the implementation
#include "Matrix_impl.hpp"
//extern template std::ostream& operator<<(std::ostream& out, const Matrix<double, StorageOrder::RowWise>& A);
}// namespace algebra


#endif// HH_MATRIX_HPP