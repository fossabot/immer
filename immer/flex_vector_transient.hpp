//
// immer - immutable data structures for C++
// Copyright (C) 2016, 2017 Juan Pedro Bolivar Puente
//
// This file is part of immer.
//
// immer is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// immer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with immer.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <immer/detail/rbts/rrbtree.hpp>
#include <immer/detail/rbts/rrbtree_iterator.hpp>
#include <immer/memory_policy.hpp>

namespace immer {

template <typename T,
          typename MemoryPolicy,
          detail::rbts::bits_t B,
          detail::rbts::bits_t BL>
class flex_vector;

template <typename T,
          typename MemoryPolicy,
          detail::rbts::bits_t B,
          detail::rbts::bits_t BL>
class vector_transient;

/*!
 * Mutable version of `immer::flex_vector`.
 *
 * @rst
 *
 * Refer to :doc:`transients` to learn more about when and how to use
 * the mutable versions of immutable containers.
 *
 * @endrst
 */
template <typename T,
          typename MemoryPolicy   = default_memory_policy,
          detail::rbts::bits_t B  = default_bits,
          detail::rbts::bits_t BL = detail::rbts::derive_bits_leaf<T, MemoryPolicy, B>>
class flex_vector_transient
    : MemoryPolicy::transience_t::owner
{
    using impl_t = detail::rbts::rrbtree<T, MemoryPolicy, B, BL>;
    using base_t = typename MemoryPolicy::transience_t::owner;
    using owner_t = typename MemoryPolicy::transience_t::owner;

public:
    static constexpr auto bits = B;
    static constexpr auto bits_leaf = BL;
    using memory_policy = MemoryPolicy;

    using value_type = T;
    using reference = const T&;
    using size_type = detail::rbts::size_t;
    using difference_type = std::ptrdiff_t;
    using const_reference = const T&;

    using iterator         = detail::rbts::rrbtree_iterator<T, MemoryPolicy, B, BL>;
    using const_iterator   = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;

    using persistent_type  = flex_vector<T, MemoryPolicy, B, BL>;

    /*!
     * Default constructor.  It creates a flex_vector of `size() == 0`.  It
     * does not allocate memory and its complexity is @f$ O(1) @f$.
     */
    flex_vector_transient() = default;

    /*!
     * Default constructor.  It creates a flex_vector with the same
     * contents as `v`.  It does not allocate memory and is
     * @f$ O(1) @f$.
     */
    flex_vector_transient(vector_transient<T, MemoryPolicy, B, BL> v)
        : base_t { std::move(static_cast<base_t&>(v)) }
        , impl_ { v.impl_.size, v.impl_.shift,
                  v.impl_.root->inc(), v.impl_.tail->inc() }
    {}

    /*!
     * Returns an iterator pointing at the first element of the
     * collection. It does not allocate memory and its complexity is
     * @f$ O(1) @f$.
     */
    iterator begin() const { return {impl_}; }

    /*!
     * Returns an iterator pointing just after the last element of the
     * collection. It does not allocate and its complexity is @f$ O(1) @f$.
     */
    iterator end()   const { return {impl_, typename iterator::end_t{}}; }

    /*!
     * Returns an iterator that traverses the collection backwards,
     * pointing at the first element of the reversed collection. It
     * does not allocate memory and its complexity is @f$ O(1) @f$.
     */
    reverse_iterator rbegin() const { return reverse_iterator{end()}; }

    /*!
     * Returns an iterator that traverses the collection backwards,
     * pointing after the last element of the reversed collection. It
     * does not allocate memory and its complexity is @f$ O(1) @f$.
     */
    reverse_iterator rend()   const { return reverse_iterator{begin()}; }

    /*!
     * Returns the number of elements in the container.  It does
     * not allocate memory and its complexity is @f$ O(1) @f$.
     */
    size_type size() const { return impl_.size; }

    /*!
     * Returns `true` if there are no elements in the container.  It
     * does not allocate memory and its complexity is @f$ O(1) @f$.
     */
    bool empty() const { return impl_.size == 0; }

    /*!
     * Returns a `const` reference to the element at position `index`.
     * It does not allocate memory and its complexity is *effectively*
     * @f$ O(1) @f$.
     */
    reference operator[] (size_type index) const
    { return impl_.get(index); }

    /*!
     * Inserts `value` at the end.  It may allocate memory and its
     * complexity is *effectively* @f$ O(1) @f$.
     */
    void push_back(value_type value)
    { impl_.push_back_mut(*this, std::move(value)); }

    /*!
     * Sets to the value `value` at position `idx`.
     * Undefined for `index >= size()`.
     * It may allocate memory and its complexity is
     * *effectively* @f$ O(1) @f$.
     */
    void set(size_type index, value_type value)
    { impl_.assoc_mut(*this, index, std::move(value)); }

    /*!
     * Updates the vector to contain the result of the expression
     * `fn((*this)[idx])` at position `idx`.
     * Undefined for `0 >= size()`.
     * It may allocate memory and its complexity is
     * *effectively* @f$ O(1) @f$.
     */
    template <typename FnT>
    void update(size_type index, FnT&& fn)
    { impl_.update_mut(*this, index, std::forward<FnT>(fn)); }

    /*!
     * Resizes the vector to only contain the first `min(elems, size())`
     * elements. It may allocate memory and its complexity is
     * *effectively* @f$ O(1) @f$.
     */
    void take(size_type elems)
    { impl_.take_mut(*this, elems); }

    /*!
     * Removes the first the first `min(elems, size())`
     * elements. It may allocate memory and its complexity is
     * *effectively* @f$ O(1) @f$.
     */
    void drop(size_type elems)
    { impl_.drop_mut(*this, elems); }

    /*!
     * Returns an @a immutable form of this container, an
     * `immer::flex_vector`.
     */
    persistent_type persistent() &
    {
        this->owner_t::operator=(owner_t{});
        return persistent_type{ impl_ };
    }
    persistent_type persistent() &&
    { return persistent_type{ std::move(impl_) }; }

    /*!
     * Appends the contents of the `r` at the end.  It may allocate
     * memory and its complexity is:
     * @f$ O(log(max(size_r, size_l))) @f$
     */
    void append(const flex_vector_transient& r)
    { concat_mut_l(impl_, *this, r.impl_); }
    void append(flex_vector_transient&& r)
    { concat_mut_lr_l(impl_, *this, r.impl_, r); }

    /*!
     * Prepends the contents of the `l` at the beginning.  It may
     * allocate memory and its complexity is:
     * @f$ O(log(max(size_r, size_l))) @f$
     */
    void prepend(const flex_vector_transient& l)
    { concat_mut_r(l.impl_, impl_, *this); }
    void prepend(flex_vector_transient&& l)
    { concat_mut_lr_r(l.impl_, l, impl_, *this); }

private:
    friend persistent_type;

    flex_vector_transient(impl_t impl)
        : impl_(std::move(impl))
    {}

    impl_t  impl_  = impl_t::empty;
};

} // namespace immer
