// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_HOOFS_CXX_FUNCTIONAL_INTERFACE_INL
#define IOX_HOOFS_CXX_FUNCTIONAL_INTERFACE_INL

#include "iceoryx_hoofs/cxx/functional_interface.hpp"

namespace iox
{
namespace cxx
{
namespace internal
{
///////////////
// BEGIN expect
///////////////
template <typename Derived>
template <typename StringType>
inline void Expect<Derived>::expect(const StringType& msg) const noexcept
{
    static_assert(is_char_array<StringType>::value || is_cxx_string<StringType>::value,
                  "Only char arrays and iox::cxx::strings are allowed as message type.");

    if (!(*static_cast<const Derived*>(this)))
    {
        print_expect_message(&msg[0]);
        Ensures(false);
    }
}

template <typename Derived, typename ValueType>
template <typename StringType>
inline ValueType& ExpectWithValue<Derived, ValueType>::expect(const StringType& msg) & noexcept
{
    static_assert(is_char_array<StringType>::value || is_cxx_string<StringType>::value,
                  "Only char arrays and iox::cxx::strings are allowed as message type.");

    Derived* derivedThis = static_cast<Derived*>(this);

    if (!(*derivedThis))
    {
        print_expect_message(&msg[0]);
        Ensures(false);
    }

    return derivedThis->value();
}

template <typename Derived, typename ValueType>
template <typename StringType>
inline const ValueType& ExpectWithValue<Derived, ValueType>::expect(const StringType& msg) const& noexcept
{
    using Self = ExpectWithValue<Derived, ValueType>;
    return const_cast<const ValueType&>(const_cast<Self*>(this)->expect(msg));
}

template <typename Derived, typename ValueType>
template <typename StringType>
inline ValueType&& ExpectWithValue<Derived, ValueType>::expect(const StringType& msg) && noexcept
{
    return std::move(this->expect(msg));
}

template <typename Derived, typename ValueType>
template <typename StringType>
inline const ValueType&& ExpectWithValue<Derived, ValueType>::expect(const StringType& msg) const&& noexcept
{
    using Self = ExpectWithValue<Derived, ValueType>;
    return const_cast<const ValueType&&>(std::move(const_cast<Self*>(this)->expect(msg)));
}
// END expect

/////////////////
// BEGIN value_or
/////////////////
template <typename Derived, typename ValueType>
template <typename U>
inline ValueType ValueOr<Derived, ValueType>::value_or(U&& alternative) const& noexcept
{
    const Derived* derivedThis = static_cast<const Derived*>(this);

    if (!(*derivedThis))
    {
        return std::forward<U>(alternative);
    }

    return derivedThis->value();
}

template <typename Derived, typename ValueType>
template <typename U>
inline ValueType ValueOr<Derived, ValueType>::value_or(U&& alternative) && noexcept
{
    const Derived* derivedThis = static_cast<const Derived*>(this);

    if (!(*derivedThis))
    {
        return std::forward<U>(alternative);
    }

    return std::move(derivedThis->value());
}
// END value_or

/////////////////
// BEGIN and_then
/////////////////
template <typename Derived, typename ValueType>
template <typename Functor>
inline Derived& AndThenWithValue<Derived, ValueType>::and_then(const Functor& callable) & noexcept
{
    static_assert(cxx::is_invocable<Functor, ValueType&>::value,
                  "Only callables with a signature of void(ValueType&) are allowed!");

    Derived* derivedThis = static_cast<Derived*>(this);

    if (*derivedThis)
    {
        auto callback = static_cast<and_then_callback_t>(callable);
        if (callback)
        {
            callback(derivedThis->value());
        }
    }

    return *derivedThis;
}

template <typename Derived, typename ValueType>
template <typename Functor>
inline Derived&& AndThenWithValue<Derived, ValueType>::and_then(const Functor& callable) && noexcept
{
    return std::move(this->and_then(callable));
}

template <typename Derived, typename ValueType>
template <typename Functor>
inline const Derived& AndThenWithValue<Derived, ValueType>::and_then(const Functor& callable) const& noexcept
{
    static_assert(cxx::is_invocable<Functor, const ValueType&>::value,
                  "Only callables with a signature of void(const ValueType&) are allowed!");

    const Derived* derivedThis = static_cast<const Derived*>(this);

    if (*derivedThis)
    {
        auto callback = static_cast<const_and_then_callback_t>(callable);
        if (callback)
        {
            callback(derivedThis->value());
        }
    }

    return *derivedThis;
}

template <typename Derived, typename ValueType>
template <typename Functor>
inline const Derived&& AndThenWithValue<Derived, ValueType>::and_then(const Functor& callable) const&& noexcept
{
    return std::move(this->and_then(callable));
}

template <typename Derived>
inline Derived& AndThen<Derived>::and_then(const and_then_callback_t& callable) & noexcept
{
    Derived* derivedThis = static_cast<Derived*>(this);

    if (*derivedThis && callable)
    {
        callable();
    }

    return *derivedThis;
}

template <typename Derived>
inline const Derived& AndThen<Derived>::and_then(const and_then_callback_t& callable) const& noexcept
{
    using Self = AndThen<Derived>;
    return const_cast<const Derived&>(const_cast<Self*>(this)->and_then(callable));
}

template <typename Derived>
inline Derived&& AndThen<Derived>::and_then(const and_then_callback_t& callable) && noexcept
{
    return std::move(this->and_then(callable));
}

template <typename Derived>
inline const Derived&& AndThen<Derived>::and_then(const and_then_callback_t& callable) const&& noexcept
{
    using Self = AndThen<Derived>;
    return std::move(const_cast<const Derived&>(const_cast<Self*>(this)->and_then(callable)));
}
// END and_then

////////////////
// BEGIN or_else
////////////////
template <typename Derived, typename ErrorType>
template <typename Functor>
inline Derived& OrElseWithValue<Derived, ErrorType>::or_else(const Functor& callable) & noexcept
{
    static_assert(cxx::is_invocable<Functor, ErrorType&>::value,
                  "Only callables with a signature of void(ErrorType&) are allowed!");

    Derived* derivedThis = static_cast<Derived*>(this);

    if (!(*derivedThis))
    {
        auto callback = static_cast<or_else_callback_t>(callable);

        if (callback)
        {
            callback(derivedThis->get_error());
        }
    }

    return *derivedThis;
}

template <typename Derived, typename ErrorType>
template <typename Functor>
inline Derived&& OrElseWithValue<Derived, ErrorType>::or_else(const Functor& callable) && noexcept
{
    return std::move(this->or_else(callable));
}

template <typename Derived, typename ErrorType>
template <typename Functor>
inline const Derived& OrElseWithValue<Derived, ErrorType>::or_else(const Functor& callable) const& noexcept
{
    static_assert(cxx::is_invocable<Functor, ErrorType&>::value,
                  "Only callables with a signature of void(const ErrorType&) are allowed!");

    const Derived* derivedThis = static_cast<const Derived*>(this);

    if (!(*derivedThis))
    {
        auto callback = static_cast<const_or_else_callback_t>(callable);
        if (callback)
        {
            callback(derivedThis->get_error());
        }
    }

    return *derivedThis;
}

template <typename Derived, typename ErrorType>
template <typename Functor>
inline const Derived&& OrElseWithValue<Derived, ErrorType>::or_else(const Functor& callable) const&& noexcept
{
    return std::move(this->or_else(callable));
}

template <typename Derived>
inline Derived& OrElse<Derived>::or_else(const or_else_callback_t& callable) & noexcept
{
    Derived* derivedThis = static_cast<Derived*>(this);

    if (!(*derivedThis) && callable)
    {
        callable();
    }

    return *derivedThis;
}

template <typename Derived>
inline Derived&& OrElse<Derived>::or_else(const or_else_callback_t& callable) && noexcept
{
    return std::move(this->or_else(callable));
}

template <typename Derived>
inline const Derived& OrElse<Derived>::or_else(const or_else_callback_t& callable) const& noexcept
{
    using Self = OrElse<Derived>;
    return const_cast<const Derived&>(const_cast<Self*>(this)->or_else(callable));
}

template <typename Derived>
inline const Derived&& OrElse<Derived>::or_else(const or_else_callback_t& callable) const&& noexcept
{
    return std::move(this->or_else(callable));
}
// END or_else


} // namespace internal
} // namespace cxx
} // namespace iox
#endif
