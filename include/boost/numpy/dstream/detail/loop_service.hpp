/**
 * $Id$
 *
 * Copyright (C)
 * 2013 - $Date$
 *     Martin Wolf <boostnumpy@martin-wolf.org>
 *
 * \file    boost/numpy/dstream/detail/loop_service.hpp
 * \version $Revision$
 * \date    $Date$
 * \author  Martin Wolf <boostnumpy@martin-wolf.org>
 *
 * \brief This file defines the loop_service_arity<N>::loop_service template
 *        that provides functionalities for iterating over the loop dimensions
 *        of a set of input arrays.
 *
 *        This file is distributed under the Boost Software License,
 *        Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 *        http://www.boost.org/LICENSE_1_0.txt).
 */
#if !defined(BOOST_PP_IS_ITERATING)

#ifndef BOOST_NUMPY_DSTREAM_DETAIL_LOOP_SERVICE_HPP_INCLUDED
#define BOOST_NUMPY_DSTREAM_DETAIL_LOOP_SERVICE_HPP_INCLUDED

#include <algorithm>
#include <vector>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/iterate.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#include <boost/numpy/dstream/detail/input_array_service.hpp>

namespace boost {
namespace numpy {
namespace dstream {
namespace detail {

struct max_loop_shape_selector
{
    max_loop_shape_selector(std::vector<intptr_t> const & arr_shape, int const arr_loop_nd)
      : arr_shape_(arr_shape)
      , arr_loop_nd_(arr_loop_nd)
    {}

    std::vector<intptr_t>
    get_arr_loop_shape() const
    {
        std::vector<intptr_t> arr_loop_shape(arr_loop_nd_);
        std::copy(arr_shape_.begin(), arr_shape_.begin() + arr_loop_nd_, arr_loop_shape.begin());
        return arr_loop_shape;
    }

    std::vector<intptr_t> const & arr_shape_;
    int const arr_loop_nd_;
};

bool operator>(max_loop_shape_selector const & lhs, max_loop_shape_selector const & rhs)
{
    return (lhs.arr_loop_nd_ > rhs.arr_loop_nd_);
}

template <int Arity>
struct loop_service_arity;

#define BOOST_PP_ITERATION_PARAMS_1                                            \
    (3, (1, BOOST_NUMPY_LIMIT_INPUT_ARITY, <boost/numpy/dstream/detail/loop_service.hpp>))
#include BOOST_PP_ITERATE()

}// namespace detail
}// namespace dstream
}// namespace numpy
}// namespace boost

#endif // ! BOOST_NUMPY_DSTREAM_DETAIL_LOOP_SERVICE_HPP_INCLUDED
#else

#define N BOOST_PP_ITERATION()

template <>
struct loop_service_arity<N>
{
    BOOST_STATIC_CONSTANT(int, arity = N);

    template <BOOST_PP_ENUM_PARAMS_Z(1, N, class InArrCoreShape)>
    class loop_service
    {
      public:
        #define BOOST_NUMPY_DEF(z, n, data) \
            BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(_in_arr_service_,n) ( BOOST_PP_CAT(in_arr_service_,n) )
        loop_service( BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, input_array_service<InArrCoreShape, > & in_arr_service_) )
          : BOOST_PP_REPEAT(N, BOOST_NUMPY_DEF, ~)
        #undef BOOST_NUMPY_DEF
        {
            // Calculate the loop shape. It's just the biggest loop shape of all
            // individual input array loop shapes.
            #define BOOST_NUMPY_DEF(z, n, data) \
                BOOST_PP_COMMA_IF(n) max_loop_shape_selector( \
                      BOOST_PP_CAT(_in_arr_service_,n).get_arr_shape() \
                    , BOOST_PP_CAT(_in_arr_service_,n).get_arr_loop_nd() )
            _loop_shape = boost::numpy::detail::max(
                BOOST_PP_REPEAT(N, BOOST_NUMPY_DEF, ~)
            ).get_arr_loop_shape();
            #undef BOOST_NUMPY_DEF

            // Set the broadcasting rules for all input arrays.
            #define BOOST_NUMPY_DEF(z, n, data) \
                BOOST_PP_CAT(_in_arr_service_,n) .set_arr_bcr(get_loop_nd());
            BOOST_PP_REPEAT(N, BOOST_NUMPY_DEF, ~)
            #undef BOOST_NUMPY_DEF
        }

        inline
        int
        get_loop_nd() const
        {
            return _loop_shape.size();
        }

        inline
        std::vector<intptr_t>
        get_loop_shape() const
        {
            return _loop_shape;
        }

        /**
         * \brief Returns the maximum length of the core dimension that has the
         *     given id. All input arrays are searched and the maximum number
         *     is returned.
         *     If the given dimension id is not found for all the input array,
         *     0 will be returned.
         */
        intptr_t
        get_len_of_core_dim(int const id)
        {
            return boost::numpy::detail::max(
                BOOST_PP_ENUM_BINARY_PARAMS_Z(1, N, _in_arr_service_, .get_len_of_core_dim(id) BOOST_PP_INTERCEPT)
            );
        }

      protected:
        std::vector<intptr_t> _loop_shape;
        #define BOOST_NUMPY_DEF(z, n, data) \
            input_array_service< BOOST_PP_CAT(InArrCoreShape,n) > & BOOST_PP_CAT(_in_arr_service_,n) ;
        BOOST_PP_REPEAT(N, BOOST_NUMPY_DEF, ~)
        #undef BOOST_NUMPY_DEF
    };
};

#undef N

#endif // BOOST_PP_IS_ITERATING