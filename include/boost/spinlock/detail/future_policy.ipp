/* future_policy.ipp
Non-allocating constexpr future-promise
(C) 2015 Niall Douglas http://www.nedprod.com/
File Created: July 2015


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef BOOST_SPINLOCK_FUTURE_NAME_POSTFIX
#error BOOST_SPINLOCK_FUTURE_NAME_POSTFIX needs to be defined
#endif
#define BOOST_SPINLOCK_GLUE2(a, b) a ## b
#define BOOST_SPINLOCK_GLUE(a, b) BOOST_SPINLOCK_GLUE2(a, b)
#ifndef BOOST_SPINLOCK_PROMISE_NAME
#define BOOST_SPINLOCK_PROMISE_NAME BOOST_SPINLOCK_GLUE(promise, BOOST_SPINLOCK_FUTURE_NAME_POSTFIX)
#endif
#ifndef BOOST_SPINLOCK_FUTURE_NAME
#define BOOST_SPINLOCK_FUTURE_NAME BOOST_SPINLOCK_GLUE(future, BOOST_SPINLOCK_FUTURE_NAME_POSTFIX)
#endif
#ifndef BOOST_SPINLOCK_SHARED_FUTURE_NAME
#define BOOST_SPINLOCK_SHARED_FUTURE_NAME BOOST_SPINLOCK_GLUE(shared_, BOOST_SPINLOCK_FUTURE_NAME)
#endif
#ifndef BOOST_SPINLOCK_FUTURE_POLICY_NAME
#define BOOST_SPINLOCK_FUTURE_POLICY_NAME BOOST_SPINLOCK_GLUE(BOOST_SPINLOCK_FUTURE_NAME, _policy)
#endif
#ifndef BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME
#define BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME BOOST_SPINLOCK_GLUE(BOOST_SPINLOCK_SHARED_FUTURE_NAME, _policy)
#endif
#ifndef BOOST_SPINLOCK_FUTURE_POLICY_BASE_NAME
#define BOOST_SPINLOCK_FUTURE_POLICY_BASE_NAME BOOST_SPINLOCK_GLUE(BOOST_SPINLOCK_FUTURE_NAME, _policy_base)
#endif
#ifndef BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME
#define BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME BOOST_SPINLOCK_GLUE(BOOST_SPINLOCK_SHARED_FUTURE_NAME, _policy_base)
#endif

namespace detail
{
  //! [future_policy]
  template<typename R> struct BOOST_SPINLOCK_FUTURE_POLICY_NAME;
  template<typename R> struct BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME;
  template<class future_storage, class value_type, class error_type=void, class exception_type=void> struct BOOST_SPINLOCK_FUTURE_POLICY_BASE_NAME;
  template<class future_storage, class value_type, class error_type=void, class exception_type=void> struct BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME;

  // Inherited from publicly by basic_monad, so whatever you expose here you expose in basic_monad
  template<class future_storage, class value_type, class error_type, class exception_type> struct BOOST_SPINLOCK_FUTURE_POLICY_BASE_NAME : public future_storage
  {
    template<class... Args> BOOST_SPINLOCK_FUTURE_CONSTEXPR BOOST_SPINLOCK_FUTURE_POLICY_BASE_NAME(Args &&... args) : future_storage(std::forward<Args>(args)...) { }
  protected:
    typedef basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME<value_type>> implementation_type;
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP bool _throw_error(monad_errc ec)
    {
      switch(ec)
      {
        case monad_errc::already_set:
          throw std::future_error(std::future_errc::promise_already_satisfied);
        case monad_errc::no_state:
          throw std::future_error(std::future_errc::no_state);
        default:
          abort();
      }
    }
  public:
    // Note we always return value_type by value.
    BOOST_SPINLOCK_FUTURE_MSVC_HELP value_type get()
    {
      static_cast<implementation_type *>(this)->wait();
      typename implementation_type::lock_guard_type h(this);
      static_cast<implementation_type *>(this)->_check_validity();
#if defined(BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE) || defined(BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE)
      if(future_storage::has_error() || future_storage::has_exception())
      {
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
        if(future_storage::has_error())
        {
          auto &category=future_storage::_storage.error.category();
          //! \todo Is there any way of making which exception type to throw from an error_category user extensible? Seems daft this isn't in the STL :(
          if(category==std::future_category())
          {
            std::future_error e(future_storage::_storage.error);
            future_storage::clear();
            throw e;
          }
          /*else if(category==std::iostream_category())
          {
            std::ios_base::failure e(std::move(future_storage::_storage.error));
            future_storage::clear();
            throw e;
          }*/
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
          else
          {
            std::system_error e(future_storage::_storage.error);
            future_storage::clear();
            throw e;
          }
#endif
        }
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
        if(future_storage::has_exception())
        {
          std::exception_ptr e(future_storage::_storage.exception);
          future_storage::clear();
          std::rethrow_exception(e);
        }
#endif
      }
#endif
      value_type v(std::move(future_storage::_storage.value));
      future_storage::clear();
      return v;
    }
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
    BOOST_SPINLOCK_FUTURE_MSVC_HELP error_type get_error()
    {
      static_cast<implementation_type *>(this)->wait();
      typename implementation_type::lock_guard_type h(this);
      static_cast<implementation_type *>(this)->_check_validity();
      if(future_storage::has_error())
      {
        error_type ec(future_storage::_storage.error);
        future_storage::clear();
        return ec;
      }
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
      if(future_storage::has_exception())
        return error_type((int) monad_errc::exception_present, monad_category());
#endif
      return error_type();
    }
#else
    BOOST_SPINLOCK_FUTURE_MSVC_HELP error_type get_error();
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
    BOOST_SPINLOCK_FUTURE_MSVC_HELP exception_type get_exception()
    {
      static_cast<implementation_type *>(this)->wait();
      typename implementation_type::lock_guard_type h(this);
      static_cast<implementation_type *>(this)->_check_validity();
      if(!future_storage::has_error() && !future_storage::has_exception())
        return exception_type();
      if(future_storage::has_error())
      {
        exception_type e(std::make_exception_ptr(std::system_error(future_storage::_storage.error)));
        future_storage::clear();
        return e;
      }
      if(future_storage::has_exception())
      {
        exception_type e(future_storage::_storage.exception);
        future_storage::clear();
        return e;
      }
      return exception_type();
    }
#else
    BOOST_SPINLOCK_FUTURE_MSVC_HELP exception_type get_exception();
#endif
    // Makes share() available on this future.
    BOOST_SPINLOCK_FUTURE_MSVC_HELP shared_basic_future_ptr<basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<value_type>>> share()
    {
      typename implementation_type::lock_guard_type h(this);
      basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<value_type>> ret(nullptr, std::move(*static_cast<implementation_type *>(this)));
      return ret.share();
    }
  };
  
  template<class future_storage, class value_type, class error_type, class exception_type> struct BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME : public future_storage
  {
    template<class... Args> BOOST_SPINLOCK_FUTURE_CONSTEXPR BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME(Args &&... args) : future_storage(std::forward<Args>(args)...) { }
  protected:
    typedef basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<value_type>> implementation_type;
    static BOOST_SPINLOCK_FUTURE_MSVC_HELP bool _throw_error(monad_errc ec)
    {
      switch(ec)
      {
        case monad_errc::already_set:
          throw std::future_error(std::future_errc::promise_already_satisfied);
        case monad_errc::no_state:
          throw std::future_error(std::future_errc::no_state);
        default:
          abort();
      }
    }
  public:
    // Note we always return value_type by value.
    BOOST_SPINLOCK_FUTURE_MSVC_HELP value_type get() const
    {
      static_cast<const implementation_type *>(this)->wait();
      typename implementation_type::lock_guard_type h(const_cast<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME *>(this));
      static_cast<const implementation_type *>(this)->_check_validity();
#if defined(BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE) || defined(BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE)
      if(future_storage::has_error() || future_storage::has_exception())
      {
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
        if(future_storage::has_error())
        {
          auto &category=future_storage::_storage.error.category();
          //! \todo Is there any way of making which exception type to throw from an error_category user extensible? Seems daft this isn't in the STL :(
          if(category==std::future_category())
          {
            std::future_error e(future_storage::_storage.error);
            //future_storage::clear();
            throw e;
          }
          /*else if(category==std::iostream_category())
          {
            std::ios_base::failure e(std::move(future_storage::_storage.error));
            //future_storage::clear();
            throw e;
          }*/
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
          else
          {
            std::system_error e(future_storage::_storage.error);
            //future_storage::clear();
            throw e;
          }
#endif
        }
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
        if(future_storage::has_exception())
        {
          std::exception_ptr e(future_storage::_storage.exception);
          //future_storage::clear();
          std::rethrow_exception(e);
        }
#endif
      }
#endif
      return future_storage::_storage.value;
    }
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
    BOOST_SPINLOCK_FUTURE_MSVC_HELP error_type get_error() const
    {
      static_cast<const implementation_type *>(this)->wait();
      typename implementation_type::lock_guard_type h(const_cast<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME *>(this));
      static_cast<const implementation_type *>(this)->_check_validity();
      if(future_storage::has_error())
      {
        error_type ec(future_storage::_storage.error);
        //future_storage::clear();
        return ec;
      }
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
      if(future_storage::has_exception())
        return error_type((int) monad_errc::exception_present, monad_category());
#endif
      return error_type();
    }
#else
    BOOST_SPINLOCK_FUTURE_MSVC_HELP error_type get_error() const;
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
    BOOST_SPINLOCK_FUTURE_MSVC_HELP exception_type get_exception() const
    {
      static_cast<const implementation_type *>(this)->wait();
      typename implementation_type::lock_guard_type h(const_cast<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME *>(this));
      static_cast<const implementation_type *>(this)->_check_validity();
      if(!future_storage::has_error() && !future_storage::has_exception())
        return exception_type();
      if(future_storage::has_error())
      {
        exception_type e(std::make_exception_ptr(std::system_error(future_storage::_storage.error)));
        //future_storage::clear();
        return e;
      }
      if(future_storage::has_exception())
      {
        exception_type e(future_storage::_storage.exception);
        //future_storage::clear();
        return e;
      }
      return exception_type();
    }
#else
    BOOST_SPINLOCK_FUTURE_MSVC_HELP exception_type get_exception() const;
#endif
    BOOST_SPINLOCK_FUTURE_MSVC_HELP shared_basic_future_ptr<basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<value_type>>> share()
    {
      return shared_basic_future_ptr<basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<value_type>>>(future_storage::shared_from_this());
    }
  };

  template<typename R> struct BOOST_SPINLOCK_FUTURE_POLICY_NAME
  {
    typedef basic_monad<BOOST_SPINLOCK_FUTURE_POLICY_NAME> monad_type;
    // In a monad policy, this is identical to monad_type. Not here.
    typedef basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME> implementation_type;
    typedef R value_type;
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
    typedef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE error_type;
#else
    typedef void error_type;
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
    typedef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE exception_type;
#else
    typedef void exception_type;
#endif
    // This type is void for monad, here it points to our future type
    typedef basic_future_base *pointer_type;
    // Future.get() locks, so define our own monad base type.
    typedef BOOST_SPINLOCK_FUTURE_POLICY_BASE_NAME<basic_future_storage<BOOST_SPINLOCK_FUTURE_POLICY_NAME, false>, value_type, error_type, exception_type> base;
    typedef BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME<basic_future_storage<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<value_type>, true>, value_type, error_type, exception_type> other_base;
    template<typename U> using rebind = basic_future<BOOST_SPINLOCK_FUTURE_POLICY_NAME<U>>;
    template<typename U> using rebind_policy = BOOST_SPINLOCK_FUTURE_POLICY_NAME<U>;

    // Does getting this future's state consume it?
    BOOST_STATIC_CONSTEXPR bool is_consuming=true;
    // Is this future managed by shared_basic_future_ptr?
    BOOST_STATIC_CONSTEXPR bool is_shared=false;
    // The type of future_errc to use for issuing errors
    typedef std::future_errc future_errc;
    // The type of future exception to use for issuing exceptions
    typedef std::future_error future_error;
    // The category of error code to use
    static const std::error_category &future_category() noexcept { return std::future_category(); }
    // The STL future type to use for waits and timed waits
    typedef std::pair<std::promise<void>, std::future<void>> wait_future_type;
  };
  template<typename R> struct BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME
  {
    typedef basic_monad<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME> monad_type;
    // In a monad policy, this is identical to monad_type. Not here.
    typedef basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME> implementation_type;
    typedef R value_type;
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
    typedef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE error_type;
#else
    typedef void error_type;
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
    typedef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE exception_type;
#else
    typedef void exception_type;
#endif
    // This type is void for monad, here it points to our future type
    typedef basic_future_base *pointer_type;
    // Future.get() locks, so define our own monad base type.
    typedef BOOST_SPINLOCK_SHARED_FUTURE_POLICY_BASE_NAME<basic_future_storage<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME, true>, value_type, error_type, exception_type> base;
    typedef BOOST_SPINLOCK_FUTURE_POLICY_BASE_NAME<basic_future_storage<BOOST_SPINLOCK_FUTURE_POLICY_NAME<value_type>, false>, value_type, error_type, exception_type> other_base;
    template<typename U> using rebind = basic_future<BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<U>>;
    template<typename U> using rebind_policy = BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<U>;

    // Does getting this future's state consume it?
    BOOST_STATIC_CONSTEXPR bool is_consuming=false;
    // Is this future managed by shared_basic_future_ptr?
    BOOST_STATIC_CONSTEXPR bool is_shared=true;
    // The type of future_errc to use for issuing errors
    typedef std::future_errc future_errc;
    // The type of future exception to use for issuing exceptions
    typedef std::future_error future_error;
    // The category of error code to use
    static const std::error_category &future_category() noexcept { return std::future_category(); }
    // The STL future type to use for waits and timed waits
    typedef std::pair<std::promise<void>, std::future<void>> wait_future_type;
  };

}

//! \brief A predefined promise convenience type \ingroup future_promise
template<typename R> using BOOST_SPINLOCK_PROMISE_NAME = basic_promise<detail::BOOST_SPINLOCK_FUTURE_POLICY_NAME<R>>;
//! \brief A predefined future convenience type \ingroup future_promise
template<typename R> using BOOST_SPINLOCK_FUTURE_NAME = basic_future<detail::BOOST_SPINLOCK_FUTURE_POLICY_NAME<R>>;

#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME BOOST_SPINLOCK_GLUE(make_ready_, BOOST_SPINLOCK_FUTURE_NAME)
//! \brief A predefined make ready future convenience function \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_FUTURE_NAME<typename std::decay<R>::type> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(R &&v)
{
  return BOOST_SPINLOCK_FUTURE_NAME<typename std::decay<R>::type>(std::forward<R>(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME BOOST_SPINLOCK_GLUE(make_errored_, BOOST_SPINLOCK_FUTURE_NAME)
//! \brief A predefined make errored future convenience function \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_FUTURE_NAME<R> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE v)
{
  return BOOST_SPINLOCK_FUTURE_NAME<R>(std::move(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME BOOST_SPINLOCK_GLUE(make_exceptional_, BOOST_SPINLOCK_FUTURE_NAME)
//! \brief A predefined make excepted future convenience function \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_FUTURE_NAME<R> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE v)
{
  return BOOST_SPINLOCK_FUTURE_NAME<R>(std::move(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#endif

//! \brief A predefined shared future convenience type \ingroup future_promise
template<typename R> using BOOST_SPINLOCK_SHARED_FUTURE_NAME = shared_basic_future_ptr<basic_future<detail::BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME<R>>>;

#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME BOOST_SPINLOCK_GLUE(make_ready_, BOOST_SPINLOCK_SHARED_FUTURE_NAME)
//! \brief A predefined make ready shared future convenience function \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_SHARED_FUTURE_NAME<typename std::decay<R>::type> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(R &&v)
{
  return BOOST_SPINLOCK_SHARED_FUTURE_NAME<typename std::decay<R>::type>(std::forward<R>(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME BOOST_SPINLOCK_GLUE(make_errored_, BOOST_SPINLOCK_SHARED_FUTURE_NAME)
//! \brief A predefined make errored shared future convenience function \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_SHARED_FUTURE_NAME<R> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE v)
{
  return BOOST_SPINLOCK_SHARED_FUTURE_NAME<R>(std::move(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#endif
#ifdef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
#define BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME BOOST_SPINLOCK_GLUE(make_exceptional_, BOOST_SPINLOCK_SHARED_FUTURE_NAME)
//! \brief A predefined make excepted shared future convenience function \ingroup future_promise
template<typename R> inline BOOST_SPINLOCK_SHARED_FUTURE_NAME<R> BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME(BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE v)
{
  return BOOST_SPINLOCK_SHARED_FUTURE_NAME<R>(std::move(v));
}
#undef BOOST_SPINLOCK_MAKE_READY_FUTURE_NAME
#endif

#undef BOOST_SPINLOCK_FUTURE_NAME_POSTFIX
#undef BOOST_SPINLOCK_GLUE
#undef BOOST_SPINLOCK_PROMISE_NAME
#undef BOOST_SPINLOCK_FUTURE_NAME
#undef BOOST_SPINLOCK_SHARED_FUTURE_NAME
#undef BOOST_SPINLOCK_FUTURE_POLICY_NAME
#undef BOOST_SPINLOCK_SHARED_FUTURE_POLICY_NAME
#undef BOOST_SPINLOCK_FUTURE_POLICY_ERROR_TYPE
#undef BOOST_SPINLOCK_FUTURE_POLICY_EXCEPTION_TYPE
