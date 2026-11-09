#pragma once

#include <any>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>
#include <algorithm>

class MethodResult {
public:
    MethodResult() = default;

    template <typename T>
    explicit MethodResult(T&& value)
        : _value(std::forward<T>(value)) {}

    [[nodiscard]]
    bool hasValue() const noexcept {
        return _value.has_value();
    }

    [[nodiscard]]
    const std::any& any() const noexcept {
        return _value;
    }

    template <typename T>
    T& get() {
        return std::any_cast<T&>(_value);
    }

    template <typename T>
    const T& get() const {
        return std::any_cast<const T&>(_value);
    }

    template <typename T>
    T take() {
        return std::any_cast<T>(std::move(_value));
    }

private:
    std::any _value;
};


// ============================================================
// Function traits
// ============================================================

template <typename T>
struct function_traits;


// ----------------------------
// Free function
// ----------------------------

template <typename Return, typename... Args>
struct function_traits<Return(*)(Args...)> {
    using return_type = Return;
    using args = std::tuple<Args...>;

    static constexpr std::size_t arity = sizeof...(Args);
    static constexpr bool is_member_function = false;
};


template <typename Return, typename... Args>
struct function_traits<Return(*)(Args...) noexcept>
    : function_traits<Return(*)(Args...)> {};


// ----------------------------
// Member function
// ----------------------------

template <typename Class, typename Return, typename... Args>
struct function_traits<Return(Class::*)(Args...)> {
    using return_type = Return;
    using args = std::tuple<Args...>;
    using class_type = Class;

    static constexpr std::size_t arity = sizeof...(Args);
    static constexpr bool is_member_function = true;
};


template <typename Class, typename Return, typename... Args>
struct function_traits<Return(Class::*)(Args...) const>
    : function_traits<Return(Class::*)(Args...)> {};


template <typename Class, typename Return, typename... Args>
struct function_traits<Return(Class::*)(Args...) volatile>
    : function_traits<Return(Class::*)(Args...)> {};


template <typename Class, typename Return, typename... Args>
struct function_traits<Return(Class::*)(Args...) const volatile>
    : function_traits<Return(Class::*)(Args...)> {};


template <typename Class, typename Return, typename... Args>
struct function_traits<Return(Class::*)(Args...) noexcept>
    : function_traits<Return(Class::*)(Args...)> {};


template <typename Class, typename Return, typename... Args>
struct function_traits<Return(Class::*)(Args...) const noexcept>
    : function_traits<Return(Class::*)(Args...) const> {};


template <typename Class, typename Return, typename... Args>
struct function_traits<Return(Class::*)(Args...) volatile noexcept>
    : function_traits<Return(Class::*)(Args...) volatile> {};


template <typename Class, typename Return, typename... Args>
struct function_traits<Return(Class::*)(Args...) const volatile noexcept>
    : function_traits<Return(Class::*)(Args...) const volatile> {};


// ============================================================
// Lambda / functor traits
// ============================================================

template <typename Callable>
struct function_traits
    : function_traits<
        decltype(&std::remove_cvref_t<Callable>::operator())
    > {

    using base =
        function_traits<
            decltype(&std::remove_cvref_t<Callable>::operator())
        >;

    static constexpr bool is_member_function = false;
};


// ============================================================
// Argument extraction
// ============================================================

namespace moddable_detail {

template <typename T>
using remove_cvref_t =
    std::remove_cv_t<std::remove_reference_t<T>>;


template <typename T>
decltype(auto) getArgument(const std::any& value) {
    using U = remove_cvref_t<T>;

    if (value.type() == typeid(U)) {
        if constexpr (std::is_reference_v<T>) {
            return std::any_cast<U&>(
                const_cast<std::any&>(value)
            );
        } else {
            return std::any_cast<U>(value);
        }
    }

    if constexpr (std::is_same_v<U, std::string>) {
        if (value.type() == typeid(const char*)) {
            return std::string(
                std::any_cast<const char*>(value)
            );
        }

        if (value.type() == typeid(char*)) {
            return std::string(
                std::any_cast<char*>(value)
            );
        }
    }

    if constexpr (std::is_same_v<U, std::string_view>) {
        if (value.type() == typeid(std::string)) {
            return std::string_view(
                std::any_cast<const std::string&>(value)
            );
        }

        if (value.type() == typeid(const char*)) {
            return std::string_view(
                std::any_cast<const char*>(value)
            );
        }

        if (value.type() == typeid(char*)) {
            return std::string_view(
                std::any_cast<char*>(value)
            );
        }
    }

    throw std::invalid_argument(
        "Moddable method received an argument of the wrong type"
    );
}


// ============================================================
// Invoke implementation
// ============================================================

template <
    typename Callable,
    typename Object,
    typename Tuple,
    std::size_t... I
>
MethodResult invokeFreeImpl(
    Callable& callable,
    Object& object,
    const std::vector<std::any>& arguments,
    std::index_sequence<I...>
) {
    using Traits = function_traits<Callable>;
    using Return = Traits::return_type;

    using ObjectArgument =
        std::tuple_element_t<0, typename Traits::args>;

    static_assert(
        std::is_convertible_v<Object&, ObjectArgument>,
        "The first argument of a Moddable callable must accept the object"
    );

    if constexpr (std::is_void_v<Return>) {

        std::invoke(
            callable,
            object,
            getArgument<
                std::tuple_element_t<I + 1, typename Traits::args>
            >(arguments[I])...
        );

        return {};

    } else {

        static_assert(
            std::is_copy_constructible_v<std::remove_cv_t<Return>>,
            "Moddable return values must be copy constructible"
        );

        return MethodResult(
            std::invoke(
                callable,
                object,
                getArgument<
                    std::tuple_element_t<I + 1, typename Traits::args>
                >(arguments[I])...
            )
        );
    }
}


template <
    typename Callable,
    typename Object,
    typename Tuple,
    std::size_t... I
>
MethodResult invokeMemberImpl(
    Callable& callable,
    Object& object,
    const std::vector<std::any>& arguments,
    std::index_sequence<I...>
) {
    using Traits = function_traits<Callable>;
    using Return = Traits::return_type;
    using Class = Traits::class_type;

    static_assert(
        std::is_convertible_v<Object&, Class&>,
        "The member function does not belong to this object hierarchy"
    );

    if constexpr (std::is_void_v<Return>) {

        std::invoke(
            callable,
            object,
            getArgument<
                std::tuple_element_t<I, typename Traits::args>
            >(arguments[I])...
        );

        return {};

    } else {

        static_assert(
            std::is_copy_constructible_v<std::remove_cv_t<Return>>,
            "Moddable return values must be copy constructible"
        );

        return MethodResult(
            std::invoke(
                callable,
                object,
                getArgument<
                    std::tuple_element_t<I, typename Traits::args>
                >(arguments[I])...
            )
        );
    }
}


// ============================================================
// Callable -> erased Moddable function
// ============================================================

template <typename Object, typename Callable>
auto makeFunction(Callable&& callable) {

    using CallableType = std::remove_cvref_t<Callable>;
    using Traits = function_traits<CallableType>;

    constexpr bool IsMember =
        Traits::is_member_function;

    constexpr std::size_t Arity =
        Traits::arity;

    if constexpr (IsMember) {

        return [
            function = std::forward<Callable>(callable)
        ](
            Object& object,
            const std::vector<std::any>& arguments
        ) mutable -> MethodResult {

            if (arguments.size() != Arity) {
                throw std::invalid_argument(
                    "Moddable method received the wrong number of arguments"
                );
            }

            return invokeMemberImpl<
                CallableType,
                Object,
                typename Traits::args
            >(
                function,
                object,
                arguments,
                std::make_index_sequence<Arity>{}
            );
        };

    } else {

        static_assert(
            Arity >= 1,
            "A Moddable lambda/free function must take the object as its first argument"
        );

        constexpr std::size_t ArgumentCount =
            Arity - 1;

        return [
            function = std::forward<Callable>(callable)
        ](
            Object& object,
            const std::vector<std::any>& arguments
        ) mutable -> MethodResult {

            if (arguments.size() != ArgumentCount) {
                throw std::invalid_argument(
                    "Moddable method received the wrong number of arguments"
                );
            }

            return invokeFreeImpl<
                CallableType,
                Object,
                typename Traits::args
            >(
                function,
                object,
                arguments,
                std::make_index_sequence<ArgumentCount>{}
            );
        };
    }
}

} // namespace moddable_detail


// ============================================================
// Method
// ============================================================

template <typename Object>
class Method {
public:
    using Function =
        std::function<
            MethodResult(
                Object&,
                const std::vector<std::any>&
            )
        >;

    Method(
        Object& owner,
        std::string name,
        Function function
    )
        : _owner(owner),
          _name(std::move(name)),
          _function(std::move(function)) {}

    MethodResult operator()() {
        return invoke({});
    }

    template <typename... Args>
    MethodResult operator()(Args&&... args) {
        return invoke({
            std::any(std::forward<Args>(args))...
        });
    }

    [[nodiscard]]
    const std::string& name() const noexcept {
        return _name;
    }

private:
    MethodResult invoke(
        const std::vector<std::any>& arguments
    ) {
        return _function(_owner, arguments);
    }

    Object& _owner;
    std::string _name;
    Function _function;
};


// ============================================================
// Moddable
// ============================================================

template <typename Object, typename Parent = void>
class Moddable {
public:

    using Function = Method<Object>::Function;


    // ========================================================
    // Per-object mod storage
    // ========================================================

    struct Storage {
        std::vector<std::any> variables;
    };


private:

    // ========================================================
    // Variable definition
    // ========================================================

    struct VariableDefinition {
        std::size_t index;
        std::function<std::any()> create;
        const std::type_info* type;
    };


    // ========================================================
    // Registry
    // ========================================================

    struct Registry {
        std::unordered_map<std::string, Function> methods;

        std::unordered_map<
            std::string,
            VariableDefinition
        > variables;

        std::size_t nextVariableIndex = 0;

        mutable std::shared_mutex mutex;
    };


    static Registry& registry() {
        static Registry instance;
        return instance;
    }


    // ========================================================
    // Own method lookup
    // ========================================================

    static std::optional<Function>
    findOwn(std::string_view name) {

        auto& r = registry();

        std::shared_lock lock(r.mutex);

        const auto it =
            r.methods.find(std::string(name));

        if (it == r.methods.end()) {
            return std::nullopt;
        }

        return it->second;
    }


    // ========================================================
    // Own variable lookup
    // ========================================================

    static std::optional<VariableDefinition>
    findOwnVariable(std::string_view name) {

        auto& r = registry();

        std::shared_lock lock(r.mutex);

        const auto it =
            r.variables.find(std::string(name));

        if (it == r.variables.end()) {
            return std::nullopt;
        }

        return it->second;
    }


    // ========================================================
    // Get this object's storage
    // ========================================================
    //
    // MODDABLE_ROOT / MODDABLE_DERIVED injects this member into
    // the actual class:
    //
    //     ModdableType::Storage _moddableStorage;
    //
    // ========================================================

    static Storage& storage(Object& object) {
        return object._moddableStorage;
    }


    static const Storage& storage(const Object& object) {
        return object._moddableStorage;
    }


public:

    // ========================================================
    // Method registration
    // ========================================================

    template <typename Callable>
    static bool extend(
        std::string name,
        Callable&& callable
    ) {
        Function function =
            moddable_detail::makeFunction<Object>(
                std::forward<Callable>(callable)
            );

        auto& r = registry();

        std::unique_lock lock(r.mutex);

        const bool replaced =
            r.methods.contains(name);

        r.methods.insert_or_assign(
            std::move(name),
            std::move(function)
        );

        return replaced;
    }


    // ========================================================
    // Variable registration
    // ========================================================
    //
    // Example:
    //
    //     Window::extendVariable<std::vector<UIObject*>>(
    //         "uiObjects"
    //     );
    //
    // Every Window gets its OWN vector.
    //
    // ========================================================

    template <typename T>
    static bool extendVariable(
        std::string name
    ) {
        auto& r = registry();

        std::unique_lock lock(r.mutex);

        const auto it =
            r.variables.find(name);

        if (it != r.variables.end()) {
            return true;
        }

        const std::size_t index =
            r.nextVariableIndex++;

        r.variables.emplace(
            std::move(name),
            VariableDefinition{
                index,
                [] {
                    return std::any(T{});
                },
                &typeid(T)
            }
        );

        return false;
    }


    // ========================================================
    // Variable access
    // ========================================================

    template <typename T>
    static T& getVariable(
        Object& object,
        std::string_view name
    ) {

        const auto definition =
            findVariable(name);

        if (definition.type != &typeid(T)) {
            throw std::invalid_argument(
                "Moddable variable has the wrong type: " +
                std::string(name)
            );
        }

        auto& s = storage(object);

        if (s.variables.size() <= definition.index) {
            s.variables.resize(
                definition.index + 1
            );
        }

        auto& slot =
            s.variables[definition.index];

        if (!slot.has_value()) {
            slot = definition.create();
        }

        return std::any_cast<T&>(slot);
    }


    template <typename T>
    static const T& getVariable(
        const Object& object,
        std::string_view name
    ) {

        const auto definition =
            findVariable(name);

        if (definition.type != &typeid(T)) {
            throw std::invalid_argument(
                "Moddable variable has the wrong type: " +
                std::string(name)
            );
        }

        const auto& s = storage(object);

        if (s.variables.size() <= definition.index ||
            !s.variables[definition.index].has_value()) {

            throw std::runtime_error(
                "Moddable variable has not been initialized: " +
                std::string(name)
            );
        }

        return std::any_cast<const T&>(
            s.variables[definition.index]
        );
    }


private:

    // ========================================================
    // Full variable lookup
    // ========================================================

    static VariableDefinition findVariable(
        std::string_view name
    ) {

        if (const auto own = findOwnVariable(name)) {
            return *own;
        }

        if constexpr (!std::is_void_v<Parent>) {

            static_assert(
                std::is_base_of_v<Parent, Object>,
                "Moddable's Parent must be an actual C++ base class"
            );

            return Parent::findModdableVariable(name);
        }

        throw std::runtime_error(
            "Moddable variable not found: " +
            std::string(name)
        );
    }


public:

    // ========================================================
    // Remove method
    // ========================================================

    static bool removeMethod(
        std::string_view name
    ) {
        auto& r = registry();

        std::unique_lock lock(r.mutex);

        return r.methods.erase(
            std::string(name)
        ) != 0;
    }


    // ========================================================
    // Method lookup
    // ========================================================

    static bool hasOwnMethod(
        std::string_view name
    ) {
        return findOwn(name).has_value();
    }


    static bool hasMethod(
        std::string_view name
    ) {
        if (findOwn(name).has_value()) {
            return true;
        }

        if constexpr (!std::is_void_v<Parent>) {
            return Parent::hasModdableMethod(name);
        }

        return false;
    }


    static Function find(
        std::string_view name
    ) {
        if (const auto own = findOwn(name)) {
            return *own;
        }

        if constexpr (!std::is_void_v<Parent>) {

            static_assert(
                std::is_base_of_v<Parent, Object>,
                "Moddable's Parent must be an actual C++ base class"
            );

            const auto parentFunction =
                Parent::findModdableMethod(name);

            return [
                parentFunction
            ](
                Object& object,
                const std::vector<std::any>& arguments
            ) mutable -> MethodResult {

                return parentFunction(
                    static_cast<Parent&>(object),
                    arguments
                );
            };
        }

        throw std::runtime_error(
            "Moddable method not found: " +
            std::string(name)
        );
    }


    // ========================================================
    // Get method
    // ========================================================

    static Method<Object> getMethod(
        Object& object,
        std::string_view name
    ) {
        return Method<Object>(
            object,
            std::string(name),
            find(name)
        );
    }


    // ========================================================
    // Method names
    // ========================================================

    static std::vector<std::string>
    ownMethodNames() {

        auto& r = registry();

        std::shared_lock lock(r.mutex);

        std::vector<std::string> result;

        result.reserve(r.methods.size());

        for (const auto& [name, _] : r.methods) {
            result.push_back(name);
        }

        return result;
    }


    static std::vector<std::string>
    methodNames() {

        std::vector<std::string> result;

        if constexpr (!std::is_void_v<Parent>) {
            result = Parent::moddableMethodNames();
        }

        const auto own = ownMethodNames();

        for (const auto& name : own) {

            const auto it =
                std::find(
                    result.begin(),
                    result.end(),
                    name
                );

            if (it == result.end()) {
                result.push_back(name);
            }
        }

        return result;
    }


    // ========================================================
    // Variable names
    // ========================================================

    static std::vector<std::string>
    ownVariableNames() {

        auto& r = registry();

        std::shared_lock lock(r.mutex);

        std::vector<std::string> result;

        result.reserve(r.variables.size());

        for (const auto& [name, _] : r.variables) {
            result.push_back(name);
        }

        return result;
    }


    static std::vector<std::string>
    variableNames() {

        std::vector<std::string> result;

        if constexpr (!std::is_void_v<Parent>) {
            result =
                Parent::moddableVariableNames();
        }

        const auto own =
            ownVariableNames();

        for (const auto& name : own) {

            const auto it =
                std::find(
                    result.begin(),
                    result.end(),
                    name
                );

            if (it == result.end()) {
                result.push_back(name);
            }
        }

        return result;
    }


    static bool hasOwnVariable(
        std::string_view name
    ) {
        return findOwnVariable(name).has_value();
    }


    static bool hasVariable(
        std::string_view name
    ) {
        return findVariableNoThrow(name);
    }


private:

    static bool findVariableNoThrow(
        std::string_view name
    ) {

        if (findOwnVariable(name).has_value()) {
            return true;
        }

        if constexpr (!std::is_void_v<Parent>) {
            return Parent::hasModdableVariable(name);
        }

        return false;
    }
};


// ============================================================
// Macros
// ============================================================

#define MODDABLE_ROOT(TYPE)                                             \
private:                                                                \
    using ModdableType = Moddable<TYPE>;                                \
    friend class Moddable<TYPE>;                                          \
    ModdableType::Storage _moddableStorage;                             \
                                                                         \
public:                                                                 \
    template <typename Callable>                                        \
    static bool extend(std::string name, Callable&& callable) {         \
        return ModdableType::extend(                                    \
            std::move(name),                                             \
            std::forward<Callable>(callable)                            \
        );                                                               \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    static bool extendVariable(std::string name) {                      \
        return ModdableType::template extendVariable<T>(                \
            std::move(name)                                              \
        );                                                               \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    T& variable(std::string_view name) {                                \
        return ModdableType::template getVariable<T>(                   \
            *this, name                                                    \
        );                                                               \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    const T& variable(std::string_view name) const {                    \
        return ModdableType::template getVariable<T>(                   \
            *this, name                                                    \
        );                                                               \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    T& v(std::string_view name) {                                        \
        return variable<T>(name);                                        \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    const T& v(std::string_view name) const {                            \
        return variable<T>(name);                                        \
    }                                                                    \
                                                                         \
    static bool hasVariable(std::string_view name) {                    \
        return ModdableType::hasVariable(name);                         \
    }                                                                    \
                                                                         \
    static bool hasOwnVariable(std::string_view name) {                 \
        return ModdableType::hasOwnVariable(name);                      \
    }                                                                    \
                                                                        \
    static bool hasModdableVariable(std::string_view name) {            \
        return ModdableType::hasVariable(name);                         \
    }                                                                   \
                                                                         \
    static auto moddableVariableNames() {                               \
        return ModdableType::variableNames();                           \
    }                                                                    \
                                                                         \
    static bool removeMethod(std::string_view name) {                   \
        return ModdableType::removeMethod(name);                        \
    }                                                                    \
                                                                         \
    static bool hasMethod(std::string_view name) {                      \
        return ModdableType::hasMethod(name);                           \
    }                                                                    \
                                                                         \
    static bool hasOwnMethod(std::string_view name) {                   \
        return ModdableType::hasOwnMethod(name);                        \
    }                                                                    \
                                                                         \
    static bool hasModdableMethod(std::string_view name) {              \
        return ModdableType::hasMethod(name);                           \
    }                                                                    \
                                                                         \
    static auto findModdableMethod(std::string_view name) {             \
        return ModdableType::find(name);                                \
    }                                                                    \
                                                                         \
    static auto moddableMethodNames() {                                 \
        return ModdableType::methodNames();                             \
    }                                                                    \
                                                                         \
    Method<TYPE> m(std::string_view name) {                             \
        return ModdableType::getMethod(*this, name);                    \
    }                                                                    \
                                                                         \
    Method<TYPE> method(std::string_view name) {                        \
        return m(name);                                                  \
    }                                                                    \
                                                                         \
private:


#define MODDABLE_DERIVED(TYPE, PARENT)                                  \
private:                                                                \
    using ModdableType = Moddable<TYPE, PARENT>;                        \
    friend class Moddable<TYPE, PARENT>;                                \
    ModdableType::Storage _moddableStorage;                             \
                                                                         \
public:                                                                 \
    template <typename Callable>                                        \
    static bool extend(std::string name, Callable&& callable) {         \
        return ModdableType::extend(                                    \
            std::move(name),                                             \
            std::forward<Callable>(callable)                            \
        );                                                               \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    static bool extendVariable(std::string name) {                      \
        return ModdableType::template extendVariable<T>(                \
            std::move(name)                                              \
        );                                                               \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    T& variable(std::string_view name) {                                \
        return ModdableType::template getVariable<T>(                   \
            *this, name                                                    \
        );                                                               \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    const T& variable(std::string_view name) const {                    \
        return ModdableType::template getVariable<T>(                   \
            *this, name                                                    \
        );                                                               \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    T& v(std::string_view name) {                                        \
        return variable<T>(name);                                        \
    }                                                                    \
                                                                         \
    template <typename T>                                                \
    const T& v(std::string_view name) const {                            \
        return variable<T>(name);                                        \
    }                                                                    \
                                                                         \
    static bool hasVariable(std::string_view name) {                    \
        return ModdableType::hasVariable(name);                         \
    }                                                                    \
                                                                         \
    static bool hasOwnVariable(std::string_view name) {                 \
        return ModdableType::hasOwnVariable(name);                      \
    }                                                                    \
                                                                        \
    static bool hasModdableVariable(std::string_view name) {            \
        return ModdableType::hasVariable(name);                         \
    }                                                                   \
                                                                         \
    static auto moddableVariableNames() {                               \
        return ModdableType::variableNames();                           \
    }                                                                    \
                                                                         \
    static bool removeMethod(std::string_view name) {                   \
        return ModdableType::removeMethod(name);                        \
    }                                                                    \
                                                                         \
    static bool hasMethod(std::string_view name) {                      \
        return ModdableType::hasMethod(name);                           \
    }                                                                    \
                                                                         \
    static bool hasOwnMethod(std::string_view name) {                   \
        return ModdableType::hasOwnMethod(name);                        \
    }                                                                    \
                                                                         \
    static bool hasModdableMethod(std::string_view name) {              \
        return ModdableType::hasMethod(name);                           \
    }                                                                    \
                                                                         \
    static auto findModdableMethod(std::string_view name) {             \
        return ModdableType::find(name);                                \
    }                                                                    \
                                                                         \
    static auto moddableMethodNames() {                                 \
        return ModdableType::methodNames();                             \
    }                                                                    \
                                                                         \
    Method<TYPE> m(std::string_view name) {                             \
        return ModdableType::getMethod(*this, name);                    \
    }                                                                    \
                                                                         \
    Method<TYPE> method(std::string_view name) {                        \
        return m(name);                                                  \
    }                                                                    \
                                                                         \
private:


// ============================================================
// Automatic static registration
// ============================================================

#define MODDABLE_DETAIL_JOIN_IMPL(A, B) A##B
#define MODDABLE_DETAIL_JOIN(A, B) MODDABLE_DETAIL_JOIN_IMPL(A, B)


#define MODDABLE_DETAIL_EXTEND_IMPL(                                  \
    TYPE, NAME, FUNCTION, ID                                           \
)                                                                       \
namespace {                                                             \
    [[maybe_unused]] const bool                                         \
        MODDABLE_DETAIL_JOIN(_moddable_registration_, ID) = [] {       \
            TYPE::extend(                                               \
                NAME,                                                    \
                FUNCTION                                                 \
            );                                                           \
            return true;                                                \
        }();                                                             \
}


#define MODDABLE_EXTEND(TYPE, NAME, FUNCTION)                           \
    MODDABLE_DETAIL_EXTEND_IMPL(                                        \
        TYPE, NAME, FUNCTION, __COUNTER__                                \
    )


// ============================================================
// Automatic variable registration
// ============================================================

#define MODDABLE_DETAIL_VARIABLE_IMPL(                                 \
    TYPE, VARIABLE_TYPE, NAME, ID                                      \
)                                                                       \
namespace {                                                             \
    [[maybe_unused]] const bool                                         \
        MODDABLE_DETAIL_JOIN(_moddable_variable_registration_, ID) = [] { \
            TYPE::template extendVariable<VARIABLE_TYPE>(NAME);         \
            return true;                                                \
        }();                                                             \
}


#define MODDABLE_VARIABLE(TYPE, VARIABLE_TYPE, NAME)                    \
    MODDABLE_DETAIL_VARIABLE_IMPL(                                      \
        TYPE, VARIABLE_TYPE, NAME, __COUNTER__                           \
    )