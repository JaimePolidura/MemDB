#pragma once

namespace std {
    template<typename V, typename E = V>
    class result {
    public:
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

        static result ok(V value) {
            result toReturn{};
            toReturn._success = true;
            toReturn._value = value;

            return toReturn;
        }

        static result error(E error) {
            result toReturn{};
            toReturn._success = false;
            toReturn._error = error;

            return toReturn;
        }

        static result error() {
            result toReturn{};
            toReturn._success = false;

            return toReturn;
        }

    private:
        E _error;
        V _value;

        bool _success;
    };
}