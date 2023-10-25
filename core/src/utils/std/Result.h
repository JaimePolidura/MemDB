#pragma once

namespace std {
    template<typename V, typename E = V>
    class result {
    public:
        result(V value): _value(value), success(true) {}

        result(E error): _error(error), success(false) {}

        V get_or_else(const V& alternative) {
            return this->success ? this->_value : alternative;
        }

        template<class Error = std::runtime_error>
        V get_or_throw(const std::string& message) {
            if(this->success){
                return this->_value;
            } else {
                throw Error(message);
            }
        }

        template<class Error = std::runtime_error>
        V get_or_throw_with(std::function<std::string(E&)> errorMessageFunc) {
            if(this->success){
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
            return this->_error;
        }

        static result ok(V value) {
            return result(value);
        }

        static result error(E error) {
            return result(error);
        }

    private:
        E _error;
        V _value;

        bool success;
    };
}