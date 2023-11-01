#pragma once

namespace std {
    template<typename V, typename E = V>
    class result {
    public:
        result(const V& value, const E& error, bool success): _value(value), _error(error), _success(success) {}

        result(bool success): _success(success) {}

        result() = default;

        V get_or_else(const V& alternative) {
            return this->_success ? this->_value : alternative;
        }

        template<class Error = std::runtime_error>
        V get_or_throw(const std::string& message) {
            if(this->_success){
                return this->_value;
            } else {
                throw Error(message);
            }
        }

        template<class Error = std::runtime_error>
        V get_or_throw_with(std::function<std::string(E&)> errorMessageFunc) {
            if(this->_success){
                return this->_value;
            } else {
                throw Error(errorMessageFunc(this->_error));
            }
        }

        V get() {
            return this->_value;
        }

        E get_error() {
            return this->_error;
        }

        bool is_success() {
            return this->_success;
        }

        bool has_error() {
            return !this->_success;
        }

        V* operator->() {
            return &this->_value;
        }

    private:
        E _error;
        V _value;

        bool _success;
    };

    template<typename V>
    static result<V> ok(const V& value) {
        result<V> toReturn{value, value, true};
        return toReturn;
    }

    template<typename V, typename E>
    static result<V, E> ok(const V& value) {
        result<V, E> toReturn{value, E{}, true};
        return toReturn;
    }

    template<typename E>
    static result<E, E> error(E error) {
        result<E, E> toReturn{error, error, false};
        return toReturn;
    }

    template<typename V, typename E>
    static result<V, E> error(E error) {
        result<V, E> toReturn{V{}, error, false};
        return toReturn;
    }

    template<typename V>
    static result<V, V> error() {
        result<V, V> toReturn{false};
        return toReturn;
    }
}