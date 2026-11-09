
#ifndef QTIP_LAZY_H
#define QTIP_LAZY_H

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include <stdexcept>

template <typename T>
class Lazy {
public:
    // deps

    class Dependency {
    public:
        virtual ~Dependency() = default;

        [[nodiscard]] virtual bool changed() const = 0;
        virtual void update() = 0;
    };

    template <typename U, typename Comparator = std::equal_to<>>
    class TypedDependency final : public Dependency {
    public:
        explicit TypedDependency(
            U* value,
            Comparator comparator = {}
        )
            : _value(value),
              _old(*value),
              _comparator(std::move(comparator)) {}

        [[nodiscard]] bool changed() const override {
            return !_comparator(*_value, _old);
        }

        void update() override {
            _old = *_value;
        }

    private:
        U* _value;
        U _old;
        Comparator _comparator;
    };

    template <typename F>
    class CallbackDependency final : public Dependency {
    public:
        explicit CallbackDependency(F callback)
            : _callback(std::move(callback)),
              _old(_callback()) {}

        [[nodiscard]] bool changed() const override {
            return _callback() != _old;
        }

        void update() override {
            _old = _callback();
        }

    private:
        F _callback;
        std::decay_t<std::invoke_result_t<F>> _old;
    };

    // make

    template <typename F, typename... Deps>
    explicit Lazy(F&& function, Deps*... deps)
        : _function(std::forward<F>(function)) {
        addDependencies(deps...);
    }

    Lazy() = default;

    Lazy(const Lazy&) = delete;
    Lazy& operator=(const Lazy&) = delete;

    Lazy(Lazy&&) noexcept = default;
    Lazy& operator=(Lazy&&) noexcept = default;

    // init

    template <typename F, typename... Deps>
    void init(F&& function, Deps*... deps) {
        _function = std::forward<F>(function);

        _deps.clear();
        addDependencies(deps...);

        reset();
    }

    template <typename F>
    void setFunction(F&& function) {
        _function = std::forward<F>(function);
        invalidate();
    }

    // deps

    template <typename U, typename Comparator = std::equal_to<>>
    void addDependency(
        U* value,
        Comparator comparator = {}
    ) {
        if (!value)
            throw std::invalid_argument("Lazy dependency cannot be null");

        _deps.push_back(
            std::make_unique<TypedDependency<U, Comparator>>(
                value,
                std::move(comparator)
            )
        );

        invalidate();
    }

    template <typename F>
    void addCallbackDependency(F&& callback) {
        using Callback = std::decay_t<F>;

        _deps.push_back(
            std::make_unique<CallbackDependency<Callback>>(
                std::forward<F>(callback)
            )
        );

        invalidate();
    }

    void clearDependencies() {
        _deps.clear();
        invalidate();
    }

    [[nodiscard]] bool dependenciesChanged() const {
        for (const auto& dependency : _deps) {
            if (dependency->changed())
                return true;
        }

        return false;
    }

    // eval

    const T& get() const {
        if (!_value || _invalid || dependenciesChanged()) {
            if (!_function)
                throw std::bad_function_call();

            T result = _function();

            for (auto& dependency : _deps)
                dependency->update();

            _value = std::move(result);
            _invalid = false;
        }

        return *_value;
    }

    const T& refresh() const {
        invalidate();
        return get();
    }

    // i like cahce

    void invalidate() const {
        _invalid = true;
    }

    void reset() const {
        _value.reset();
        _invalid = true;
    }

    template <typename... Args>
    void emplace(Args&&... args) const {
        _value.emplace(std::forward<Args>(args)...);

        for (auto& dependency : _deps)
            dependency->update();

        _invalid = false;
    }

    [[nodiscard]] bool hasValue() const {
        return _value.has_value();
    }

    [[nodiscard]] bool isValid() const {
        return _value.has_value()
            && !_invalid
            && !dependenciesChanged();
    }

    [[nodiscard]] bool isDirty() const {
        return !isValid();
    }

    template <typename U>
    T valueOr(U&& fallback) const {
        if (isValid())
            return *_value;

        return std::forward<U>(fallback);
    }

    // ops

    operator const T&() const {
        return get();
    }

    const T& operator()() const {
        return get();
    }

    const T& operator*() const {
        return get();
    }

    const T* operator->() const {
        return &get();
    }

private:
    template <typename... Deps>
    void addDependencies(Deps*... deps) {
        (addDependency(deps), ...);
    }

    std::function<T()> _function;

    std::vector<std::unique_ptr<Dependency>> _deps;

    mutable std::optional<T> _value;
    mutable bool _invalid = false;
};

#endif // QTIP_LAZY_H
