#pragma once

#include "natalie/env.hpp"
#include "natalie/forward.hpp"
#include "natalie/gc.hpp"

namespace Natalie {

struct Block : public Cell {

    Block(Env &env, ValuePtr self, MethodFnPtr fn, int arity)
        : m_fn { fn }
        , m_arity { arity }
        , m_env { env }
        , m_self { self } {
        m_env.clear_caller();
    }

    // NOTE: This should only be called from one of the RUN_BLOCK_* macros!
    ValuePtr _run(Env *env, size_t argc = 0, ValuePtr *args = nullptr, Block *block = nullptr) {
        Env e = Env { &m_env, env };
        return m_fn(&e, m_self, argc, args, block);
    }

    int arity() { return m_arity; }

    Env *env() { return &m_env; }

    void set_self(ValuePtr self) { m_self = self; }

    void copy_fn_pointer_to_method(Method *);

private:
    MethodFnPtr m_fn;
    int m_arity { 0 };
    Env m_env {};
    ValuePtr m_self { nullptr };
};

}
