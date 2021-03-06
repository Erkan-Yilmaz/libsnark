/** @file
 *****************************************************************************
 Implementation of misc. math and serialization utility functions
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef FIELD_UTILS_TCC_
#define FIELD_UTILS_TCC_

#include "common/utils.hpp"

namespace libsnark {

template<typename FieldT>
FieldT coset_shift()
{
    return FieldT::multiplicative_generator.squared();
}

template<typename FieldT>
FieldT get_root_of_unity(const size_t n)
{
    const size_t logn = log2(n);
    assert(n == (1u << logn));
    assert(logn <= FieldT::s);

    FieldT omega = FieldT::root_of_unity;
    for (size_t i = FieldT::s; i > logn; --i)
    {
        omega *= omega;
    }

    return omega;
}

template<typename FieldT>
std::vector<FieldT> pack_int_vector_into_field_element_vector(const std::vector<size_t> &v, const size_t w)
{
    const size_t chunk_bits = FieldT::num_bits-1;
    const size_t repacked_size = div_ceil(v.size() * w, chunk_bits);
    std::vector<FieldT> result(repacked_size);

    for (size_t i = 0; i < repacked_size; ++i)
    {
        bigint<FieldT::num_limbs> b;
        for (size_t j = 0; j < chunk_bits; ++j)
        {
            const size_t word_index = (i * chunk_bits + j) / w;
            const size_t pos_in_word = (i * chunk_bits + j) % w;
            const size_t word_or_0 = (word_index < v.size() ? v[word_index] : 0);
            const size_t bit = (word_or_0 >> pos_in_word) & 1;

            b.data[j / GMP_NUMB_BITS] |= bit << (j % GMP_NUMB_BITS);
        }
        result[i] = FieldT(b);
    }

    return result;
}

template<typename FieldT>
std::vector<FieldT> pack_bit_vector_into_field_element_vector(const bit_vector &v, const size_t chunk_bits)
{
    assert(chunk_bits <= FieldT::num_bits-1);

    const size_t repacked_size = div_ceil(v.size(), chunk_bits);
    std::vector<FieldT> result(repacked_size);

    for (size_t i = 0; i < repacked_size; ++i)
    {
        bigint<FieldT::num_limbs> b;
        for (size_t j = 0; j < chunk_bits; ++j)
        {
            b.data[j / GMP_NUMB_BITS] |= ((i * chunk_bits + j) < v.size() && v[i * chunk_bits + j] ? 1ll : 0ll) << (j % GMP_NUMB_BITS);
        }
        result[i] = FieldT(b);
    }

    return result;
}

template<typename FieldT>
std::vector<FieldT> pack_bit_vector_into_field_element_vector(const bit_vector &v)
{
    return pack_bit_vector_into_field_element_vector<FieldT>(v, FieldT::num_bits-1);
}

template<typename FieldT>
std::vector<FieldT> convert_bit_vector_to_field_element_vector(const bit_vector &v)
{
    std::vector<FieldT> result;
    result.reserve(v.size());

    for (const bool b : v)
    {
        result.emplace_back(b ? FieldT::one() : FieldT::zero());
    }

    return result;
}

template<typename FieldT>
bit_vector convert_field_element_vector_to_bit_vector(const std::vector<FieldT> &v)
{
    bit_vector result;
    result.reserve(v.size());

    for (const FieldT &el : v)
    {
        assert(el == FieldT::one() || el == FieldT::zero());
        result.push_back(el == FieldT::one());
    }

    return result;
}

template<typename FieldT>
bit_vector convert_field_element_to_bit_vector(const FieldT &el)
{
    bit_vector result;

    bigint<FieldT::num_limbs> b = el.as_bigint();
    for (size_t i = 0; i < FieldT::size_in_bits(); ++i)
    {
        result.push_back(b.test_bit(i));
    }

    return result;
}

template<typename FieldT>
bit_vector convert_field_element_to_bit_vector(const FieldT &el, const size_t bitcount)
{
    bit_vector result = convert_field_element_to_bit_vector(el);
    return bit_vector(result.begin(), result.begin() + bitcount);
}

template<typename FieldT>
FieldT convert_bit_vector_to_field_element(const bit_vector &v)
{
    assert(v.size() <= FieldT::num_bits);

    FieldT res = FieldT::zero();
    FieldT c = FieldT::one();
    for (bool b : v)
    {
        res += b ? c : FieldT::zero();
        c += c;
    }
    return res;
}

template<typename T, typename FieldT>
T naive_plain_exp(const T &neutral,
                  typename std::vector<T>::const_iterator vec_start,
                  typename std::vector<T>::const_iterator vec_end,
                  typename std::vector<FieldT>::const_iterator scalar_start,
                  typename std::vector<FieldT>::const_iterator scalar_end)
{
    T result(neutral);

    typename std::vector<T>::const_iterator vec_it;
    typename std::vector<FieldT>::const_iterator scalar_it;

    for (vec_it = vec_start, scalar_it = scalar_start; vec_it != vec_end; ++vec_it, ++scalar_it)
    {
        result = result + (*vec_it) * (*scalar_it);
    }
    assert(scalar_it == scalar_end);

    return result;
}

template<typename FieldT>
void batch_invert(std::vector<FieldT> &vec)
{
    std::vector<FieldT> prod;
    prod.reserve(vec.size());

    FieldT acc = FieldT::one();

    for (auto el : vec)
    {
        assert(!el.is_zero());
        prod.emplace_back(acc);
        acc = acc * el;
    }

    FieldT acc_inverse = acc.inverse();

    for (long i = vec.size()-1; i >= 0; --i)
    {
        const FieldT old_el = vec[i];
        vec[i] = acc_inverse * prod[i];
        acc_inverse = acc_inverse * old_el;
    }
}

template<typename FieldT, mp_size_t m>
FieldT power(const FieldT &base, const bigint<m> &exponent)
{
    FieldT result = FieldT::one();

    bool found_one = false;
    for (long i = exponent.max_bits() - 1; i >= 0; --i)
    {
        if (found_one)
        {
            result = result * result;
        }

        if (exponent.test_bit(i))
        {
            found_one = true;
            result = result * base;
        }
    }

    return result;
}

template<typename FieldT>
FieldT power(const FieldT &base, const unsigned long exponent)
{
    return power<FieldT>(base, bigint<1>(exponent));
}

} // libsnark
#endif // FIELD_UTILS_TCC_
