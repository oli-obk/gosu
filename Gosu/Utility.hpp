//! \file Utility.hpp
//! General purpose utility functions.

#ifndef GOSU_UTILITY_HPP
#define GOSU_UTILITY_HPP

#include <string>
#include <vector>
#include <Gosu/Platform.hpp>
#if defined(GOSU_CPP11_ENABLED)
#include <type_traits>
#else
#include <stdint.h>
#endif

namespace Gosu
{
    //! Converts an std::string into an std::wstring.
    std::wstring utf8ToWstring(const std::string& utf8);
    //! Converts an std::wstring into an std::string.
    std::string wstringToUTF8(const std::wstring& ws);
    
    //! Converts an std::string into an std::wstring using local encoding.
    std::wstring widen(const std::string& s);
    //! Converts an std::wstring into an std::string using local encoding.
    std::string narrow(const std::wstring& ws);
    
    //! Returns the user's preferred language, at the moment of calling the function. Expect return
    //! values such as 'en_US', 'de_DE.UTF-8', 'ja', 'zh-Hans'. You can rely only on the first two letters
    //! being a common language abbreviation.
    std::string language();
    
#if !defined(GOSU_CPP11_ENABLED)
    namespace FlagWrapperHelper {
        template<size_t type_size> class size_to_type;
        template<> class size_to_type<1> { public: typedef uint8_t type; };
        template<> class size_to_type<2> { public: typedef uint16_t type; };
        template<> class size_to_type<4> { public: typedef uint32_t type; };
        template<> class size_to_type<8> { public: typedef uint64_t type; };
    }
#endif
    
    template<typename T>
    class FlagWrapper
    {
    private:
#if defined(GOSU_CPP11_ENABLED)
        typedef typename std::underlying_type<T>::type int_type;
        T flag = static_cast<T>(0);
        static_assert(std::is_enum<T>::value && !std::is_convertible<T, int>::value, "FlagWrapper can only be used with enum classes");
    public:
        template<typename Arg, typename... Args>
        FlagWrapper(Arg head, Args... tail)
        :FlagWrapper(tail...)
        {
            Add(head);
        }
        FlagWrapper() = default;
#else
        T flag;
    public:
        FlagWrapper(T f = T(0))
        {
            flag = f;
        }
        typedef typename FlagWrapperHelper::size_to_type<sizeof(T)>::type int_type;
#endif
    public:
        static FlagWrapper from_integral(int_type val)
        {
            return FlagWrapper(static_cast<T>(val));
        }
        void Add(T f)
        {
            flag = static_cast<T>(static_cast<int_type>(flag) | static_cast<int_type>(f));
        }
        void Remove(T f)
        {
            flag = static_cast<T>(static_cast<int_type>(flag) & ~static_cast<int_type>(f));
        }
        void Mask(T f)
        {
            flag = static_cast<T>(static_cast<int_type>(flag) & static_cast<int_type>(f));
        }
        FlagWrapper& operator|=(T f)
        {
            Add(f);
            return *this;
        }
        FlagWrapper& operator&=(T f)
        {
            Mask(f);
            return *this;
        }
        FlagWrapper operator|(T f) const
        {
            FlagWrapper ret = *this;
            ret.Add(f);
            return ret;
        }
        FlagWrapper operator&(T f) const
        {
            FlagWrapper ret = *this;
            ret.Mask(f);
            return ret;
        }
        FlagWrapper& operator|=(FlagWrapper f)
        {
            Add(f.flag);
            return *this;
        }
        FlagWrapper& operator&=(FlagWrapper f)
        {
            Mask(f.flag);
            return *this;
        }
        FlagWrapper operator|(FlagWrapper f) const
        {
            FlagWrapper ret = *this;
            ret.Add(f.flag);
            return ret;
        }
        FlagWrapper operator&(FlagWrapper f) const
        {
            FlagWrapper ret = *this;
            ret.Mask(f.flag);
            return ret;
        }
        FlagWrapper operator~() const
        {
            FlagWrapper ret = *this;
            ret.flag = static_cast<T>(~static_cast<int_type>(flag));
            return ret;
        }
        operator bool() const
        {
            return static_cast<bool>(flag);
        }
        bool operator==(const FlagWrapper& rhs) const
        {
            return flag == rhs.flag;
        }
        bool operator!=(const FlagWrapper& rhs) const
        {
            return flag != rhs.flag;
        }
    };

    template<typename T>
    FlagWrapper<T> operator|(const T& lhs, const FlagWrapper<T>& rhs)
    {
        return FlagWrapper<T>(lhs) | rhs;
    }

    template<typename T>
    FlagWrapper<T> operator&(const T& lhs, const FlagWrapper<T>& rhs)
    {
        return FlagWrapper<T>(lhs) & rhs;
    }
}

#endif
