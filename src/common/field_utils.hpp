/** @file
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef FIELD_UTILS_HPP_
#define FIELD_UTILS_HPP_
#include <cstdint>

#include "common/utils.hpp"
#include "algebra/fields/bigint.hpp"

namespace libsnark {

// returns root of unity of order n (for n a power of 2), if one exists
template<typename FieldT>
FieldT get_root_of_unity(const size_t n);

template<typename FieldT>
std::vector<FieldT> pack_int_vector_into_field_element_vector(const std::vector<size_t> &v, const size_t w);

template<typename FieldT>
std::vector<FieldT> pack_bit_vector_into_field_element_vector(const bit_vector &v, const size_t chunk_bits);

template<typename FieldT>
std::vector<FieldT> pack_bit_vector_into_field_element_vector(const bit_vector &v);

template<typename FieldT>
std::vector<FieldT> convert_bit_vector_to_field_element_vector(const bit_vector &v);

template<typename FieldT>
bit_vector convert_field_element_vector_to_bit_vector(const std::vector<FieldT> &v);

template<typename FieldT>
bit_vector convert_field_element_to_bit_vector(const FieldT &el);

template<typename FieldT>
bit_vector convert_field_element_to_bit_vector(const FieldT &el, const size_t bitcount);

template<typename FieldT>
FieldT convert_bit_vector_to_field_element(const bit_vector &v);

template<typename T, typename FieldT>
T naive_plain_exp(const T &neutral,
                  typename std::vector<T>::const_iterator vec_start,
                  typename std::vector<T>::const_iterator vec_end,
                  typename std::vector<FieldT>::const_iterator scalar_start,
                  typename std::vector<FieldT>::const_iterator scalar_end);

template<typename FieldT>
void batch_invert(std::vector<FieldT> &vec);

template<typename FieldT, mp_size_t m>
FieldT power(const FieldT &base, const bigint<m> &exponent);

template<typename FieldT>
FieldT power(const FieldT &base, const unsigned long exponent);

} // libsnark
#include "common/field_utils.tcc"

#endif // FIELD_UTILS_HPP_
