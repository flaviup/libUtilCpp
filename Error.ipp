//
//  Error.ipp
//  UtilCpp
//
//  Created by Flaviu Pasca on 12/15/23.
//

#ifndef UtilCpp_Error_ipp
#define UtilCpp_Error_ipp

#include <concepts>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include "expected.ipp"

#pragma GCC visibility push(default)

namespace UtilCpp {

    struct IError {
        virtual ~IError() noexcept = default;
        virtual uintptr_t GetClassId() const noexcept = 0;
        virtual explicit operator std::string() const noexcept = 0;
        virtual const std::string& GetType() const noexcept = 0;
        virtual const std::string& GetDetails() const noexcept = 0;
        virtual const std::string& GetInfo() const noexcept = 0;
    };

    template <typename T>
    concept ErrorConcept = std::derived_from<T, IError> || std::is_convertible_v<T, IError>;

    struct Error : public IError {

        Error() noexcept = default;

        Error(const std::string& details) noexcept :
            _details(details) {
        }

        Error(std::string&& details) noexcept :
            _details(std::move(details)) {
        }

        Error(const std::string& type, const std::string& details) noexcept :
            _type(type),
            _details(details) {
        }

        Error(std::string&& type, std::string&& details) noexcept :
            _type(std::move(type)),
            _details(std::move(details)) {
        }

        Error(const Error& error) noexcept = default;
        Error(Error&& error) noexcept = default;
        virtual ~Error() noexcept = default;

        static void Id() noexcept {}

        virtual uintptr_t GetClassId() const noexcept override {
            return reinterpret_cast<uintptr_t>(Id);
        }

        Error& operator =(const Error& other) noexcept = default;
        Error& operator =(Error&& other) noexcept = default;

        virtual explicit operator std::string() const noexcept override {
            return _details; 
        }

        inline Error& WithDetails(const std::string& details) noexcept {

            _details = details;
            return *this;
        }

        inline Error& WithDetails(std::string&& details) noexcept {

            _details = std::move(details);
            return *this;
        }

        inline Error& WithInfo(const std::string& info) noexcept {

            _devInfo = info;
            return *this;
        }

        inline Error& WithInfo(std::string&& info) noexcept {

            _devInfo = std::move(info);
            return *this;
        }

        virtual const std::string& GetType() const noexcept override {
            return _type;
        }

        virtual const std::string& GetDetails() const noexcept override {
            return _details;
        }

        virtual const std::string& GetInfo() const noexcept override {
            return _devInfo;
        }

    private:
        // Short name for the type of error that has occurred.
        std::string _type;
        // Additional details about the error.
        std::string _details;
        // Information to help debug the error.
        std::string _devInfo;
    };

    template<ErrorConcept T>
    struct WrappingError final : public IError {

        explicit WrappingError(T&& err) : _err(std::move(err)) {
        }
        ~WrappingError() noexcept override = default;

        uintptr_t GetClassId() const noexcept override {
            return _err.GetClassId();
        }

        WrappingError& operator =(const WrappingError& other) noexcept = default;
        WrappingError& operator =(WrappingError&& other) noexcept = default;

        explicit operator std::string() const noexcept override {
            return std::string(_err); 
        }

        const std::string& GetType() const noexcept override {
            return _err.GetType();
        }

        const std::string& GetDetails() const noexcept override {
            return _err.GetDetails();
        }

        const std::string& GetInfo() const noexcept override {
            return _err.GetInfo();
        }

    private:
        T _err;
    };

    struct ErrorWrapper {

        template<ErrorConcept T>
        ErrorWrapper(T t) :
            _ptr(std::make_shared<WrappingError<T>>(std::move(t))) {
        }

        /*template<ErrorConcept T>
        TypeWrapper(T&& t) :
            _ptr(std::make_shared<WrappingError<T>>(std::forward<T>(t))) {
        }*/

        inline uintptr_t GetClassId() const noexcept {
            return _ptr.get()->GetClassId();
        }

        inline explicit operator std::string() const noexcept {
            return std::string(*_ptr.get()); 
        }

        inline const std::string& GetType() const noexcept {
            return _ptr.get()->GetType();
        }

        inline const std::string& GetDetails() const noexcept {
            return _ptr.get()->GetDetails();
        }

        inline const std::string& GetInfo() const noexcept {
            return _ptr.get()->GetInfo();
        }

    private:
        std::shared_ptr<IError> _ptr;
    };

    using OptionalError = std::optional<ErrorWrapper>;

    template<ErrorConcept E>
    inline OptionalError MakeOptionalError(E&& error) { return ErrorWrapper{ std::move(error) }; }
    template<ErrorConcept E>
    inline OptionalError MakeOptionalError(const E& error) { return ErrorWrapper{ error }; }

    inline constexpr auto NoError = std::nullopt;

    template<typename T>
    using expected = tl::expected<T, ErrorWrapper>;
    using unexpected = tl::unexpected<ErrorWrapper>;

    using SuccessResult = tl::expected<bool, ErrorWrapper>;
    inline SuccessResult MakeResultError(ErrorWrapper&& error) { return tl::unexpected{ std::move(error) }; }
    inline SuccessResult MakeResultError(const ErrorWrapper& error) { return tl::unexpected{ error }; }

    template<typename E>
    requires ErrorConcept<E> || std::same_as<E, ErrorWrapper>
    inline unexpected MakeError(E&& error) { return unexpected(ErrorWrapper{ std::move(error) }); }
    template <>
    inline unexpected MakeError(ErrorWrapper&& error) { return unexpected({ std::move(error) }); }
    template<typename E>
    requires ErrorConcept<E> || std::same_as<E, ErrorWrapper>
    inline unexpected MakeError(const E& error) { return unexpected(ErrorWrapper{ error }); }
    template <>
    inline unexpected MakeError(const ErrorWrapper& error) { return unexpected({ error }); }
} // namespace UtilCpp

#pragma GCC visibility pop

#endif /* UtilCpp_Error_ipp */
