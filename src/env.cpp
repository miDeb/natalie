#include <stdarg.h>

#include "natalie.hpp"
#include "natalie/string.hpp"

namespace Natalie {

using namespace TM;

void Env::build_vars(size_t size) {
    m_vars = new Vector<ValuePtr>(size, NilValue::the());
}

ValuePtr Env::global_get(SymbolValue *name) {
    return GlobalEnv::the()->global_get(this, name);
}

ValuePtr Env::global_set(SymbolValue *name, ValuePtr val) {
    return GlobalEnv::the()->global_set(this, name, val);
}

Method *Env::current_method() {
    Env *env = this;
    while (!env->method() && env->outer()) {
        env = env->outer();
    }
    return env->method();
}

const String *Env::build_code_location_name(Env *location_env) {
    if (location_env->is_main())
        return new String("<main>");
    if (location_env->method())
        return new String(location_env->method()->name());
    // we're in a block, so try to build a string like "block in foo", "block in block in foo", etc.
    if (location_env->outer()) {
        auto outer_name = build_code_location_name(location_env->outer());
        return String::format("block in {}", outer_name);
    }
    // fall back to just "block" if we don't know where this block came from
    return new String("block");
}

void Env::raise(ClassValue *klass, StringValue *message) {
    ExceptionValue *exception = new ExceptionValue { klass, message };
    this->raise_exception(exception);
}

void Env::raise(ClassValue *klass, class String *message) {
    ExceptionValue *exception = new ExceptionValue { klass, new StringValue { *message } };
    this->raise_exception(exception);
}

void Env::raise(const char *class_name, const class String *message) {
    ClassValue *klass = GlobalEnv::the()->Object()->const_fetch(SymbolValue::intern(class_name))->as_class();
    ExceptionValue *exception = new ExceptionValue { klass, new StringValue { *message } };
    this->raise_exception(exception);
}

void Env::raise_exception(ExceptionValue *exception) {
    if (!exception->backtrace()) {
        // only build a backtrace the first time the exception is raised (not on a re-raise)
        exception->build_backtrace(this);
    }
    throw exception;
}

void Env::raise_local_jump_error(ValuePtr exit_value, const char *message) {
    ExceptionValue *exception = new ExceptionValue { GlobalEnv::the()->Object()->const_find(this, SymbolValue::intern("LocalJumpError"))->as_class(), new StringValue { message } };
    exception->ivar_set(this, SymbolValue::intern("@exit_value"), exit_value);
    this->raise_exception(exception);
}

void Env::raise_errno() {
    ValuePtr exception_args[] = { ValuePtr::integer(errno) };
    ExceptionValue *error = GlobalEnv::the()->Object()->const_find(this, SymbolValue::intern("SystemCallError")).send(this, "exception", 1, exception_args, nullptr)->as_exception();
    raise_exception(error);
}

void Env::ensure_argc_is(size_t argc, size_t expected) {
    if (argc != expected) {
        raise("ArgumentError", "wrong number of arguments (given {}, expected {})", argc, expected);
    }
}

void Env::ensure_argc_between(size_t argc, size_t expected_low, size_t expected_high) {
    if (argc < expected_low || argc > expected_high) {
        raise("ArgumentError", "wrong number of arguments (given {}, expected {}..{})", argc, expected_low, expected_high);
    }
}

void Env::ensure_argc_at_least(size_t argc, size_t expected) {
    if (argc < expected) {
        raise("ArgumentError", "wrong number of arguments (given {}, expected {}+)", argc, expected);
    }
}

void Env::ensure_block_given(Block *block) {
    if (!block) {
        raise("ArgumentError", "called without a block");
    }
}

ValuePtr Env::last_match() {
    if (m_match) {
        return m_match;
    } else {
        return NilValue::the();
    }
}

ValuePtr Env::var_get(const char *key, size_t index) {
    if (index >= m_vars->size()) {
        printf("Trying to get variable `%s' at index %zu which is not set.\n", key, index);
        abort();
    }
    ValuePtr val = m_vars->at(index);
    if (val) {
        return val;
    } else {
        return NilValue::the();
    }
}

ValuePtr Env::var_set(const char *name, size_t index, bool allocate, ValuePtr val) {
    size_t needed = index + 1;
    size_t current_size = m_vars ? m_vars->size() : 0;
    if (needed > current_size) {
        if (allocate) {
            if (!m_vars) {
                build_vars(needed);
            } else {
                m_vars->set_size(needed, NilValue::the());
            }
        } else {
            printf("Tried to set a variable without first allocating space for it.\n");
            abort();
        }
    }
    m_vars->at(index) = val;
    return val;
}

void Env::visit_children(Visitor &visitor) {
    if (m_vars) {
        for (auto &val : *m_vars) {
            visitor.visit(val);
        }
    }
    visitor.visit(m_outer);
    visitor.visit(m_block);
    visitor.visit(m_caller);
    visitor.visit(m_method);
    visitor.visit(m_match);
}

}
