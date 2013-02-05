#ifndef TILEDARRAY_RANGE_ITERATOR_H__INCLUDED
#define TILEDARRAY_RANGE_ITERATOR_H__INCLUDED

#include <TiledArray/error.h>
#include <iterator>

namespace TiledArray {
  namespace detail {

    template <typename, typename>
    class RangeIterator;

  } // namespace detail
} // namespace TiledArray

namespace std {
  template <typename Value, typename Container>
  void advance(TiledArray::detail::RangeIterator<Value, Container>&,
      typename TiledArray::detail::RangeIterator<Value, Container>::difference_type );

  template <typename Value, typename Container>
  typename TiledArray::detail::RangeIterator<Value, Container>::difference_type
  distance(const TiledArray::detail::RangeIterator<Value, Container>&,
      const TiledArray::detail::RangeIterator<Value, Container>&);

} // namespace std

namespace TiledArray {
  namespace detail {

    /// Coordinate index iterate

    /// This is an input iterator that is used to iterate over the coordinate
    /// indexes of a \c Range.
    /// \tparam Value The value type of the iterator
    /// \tparam Container The container that the iterator references
    /// \note The container object must define the function
    /// \c Container::increment(Value&) \c const, and be accessible to
    /// \c RangeIterator.
    template <typename Value, typename Container>
    class RangeIterator {
    public:
      typedef RangeIterator<Value,Container> RangeIterator_; ///< This class type

      // Standard iterator typedefs
      typedef Value value_type; ///< Iterator value type
      typedef const Value& reference; ///< Iterator reference type
      typedef const Value* pointer; ///< Iterator pointer type
      typedef std::input_iterator_tag iterator_category; /// Iterator category tag
      typedef std::ptrdiff_t difference_type; ///< Iterator difference type

      /// Copy constructor

      /// \param other The other iterator to be copied
      RangeIterator(const RangeIterator_& other) :
        container_(other.container_), current_(other.current_) {
      }

      /// Construct an index iterator

      /// \param v The initial value of the iterator index
      /// \param c The container that the iterator will reference
      RangeIterator(const Value& v, const Container* c) :
          container_(c), current_(v)
      { }

      /// Copy constructor

      /// \param other The other iterator to be copied
      /// \return A reference to this object
      RangeIterator_& operator=(const RangeIterator_& other) {
        current_ = other.current_;
        container_ = other.container_;

        return *this;
      }

      const Container* container() const { return container_; }

      /// Dereference operator

      /// \return A \c reference to the current data
      reference operator*() const { return current_; }

      /// Increment operator

      /// Increment the iterator
      /// \return The modified iterator
      RangeIterator_& operator++() {
        container_->increment(current_);
        return *this;
      }

      /// Increment operator

      /// Increment the iterator
      /// \return An unmodified copy of the iterator
      RangeIterator_ operator++(int) {
        RangeIterator_ temp(*this);
        container_->increment(current_);
        return temp;
      }

      /// Pointer operator

      /// \return A \c pointer to the current data
      pointer operator->() const { return & current_; }


      void advance(difference_type n) {
        container_->advance(current_, n);
      }

      difference_type distance_to(const RangeIterator_& other) const {
        TA_ASSERT(container_ == other.container_);
        return container_->distance_to(current_, other.current_);
      }

    private:

      const Container* container_;  ///< The container that the iterator references
      Value current_;               ///< The current value of the iterator
    }; // class RangeIterator

    /// Equality operator

    /// Compares the iterators for equality. They must reference the same range
    /// object to be considered equal.
    /// \tparam Value The value type of the iterator
    /// \tparam Container The container that the iterator references
    /// \param left_it The left-hand iterator to be compared
    /// \param right_it The right-hand iterator to be compared
    /// \return \c true if the the value and container are equal for the \c left_it
    /// and \c right_it , otherwise \c false .
    template <typename Value, typename Container>
    bool operator==(const RangeIterator<Value, Container>& left_it, const RangeIterator<Value, Container>& right_it) {
      return ((*left_it) == (*right_it)) &&
          (left_it.container() == right_it.container());
    }

    /// Inequality operator

    /// Compares the iterators for inequality.
    /// \tparam Value The value type of the iterator
    /// \tparam Container The container that the iterator references
    /// \param left_it The left-hand iterator to be compared
    /// \param right_it The right-hand iterator to be compared
    /// \return \c true if the the value or container are not equal for the
    /// \c left_it and \c right_it , otherwise \c false .
    template <typename Value, typename Container>
    bool operator!=(const RangeIterator<Value, Container>& left_it, const RangeIterator<Value, Container>& right_it) {
      return ((*left_it) != (*right_it)) ||
          (left_it.container() != right_it.container());
    }

  } // namespace detail
} // namespace TiledArray

namespace std {
  template <typename Value, typename Container>
  void advance(TiledArray::detail::RangeIterator<Value, Container>& it,
      typename TiledArray::detail::RangeIterator<Value, Container>::difference_type n)
  {
    it.advance(n);
  }

  template <typename Value, typename Container>
  typename TiledArray::detail::RangeIterator<Value, Container>::difference_type
  distance(const TiledArray::detail::RangeIterator<Value, Container>& first,
      const TiledArray::detail::RangeIterator<Value, Container>& last)
  {
    return first.distance_to(last);
  }

} // namespace std

#endif // TILEDARRAY_RANGE_ITERATOR_H__INCLUDED
