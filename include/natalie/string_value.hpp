#pragma once

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <string>

#include "natalie/class_value.hpp"
#include "natalie/encoding_value.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/macros.hpp"
#include "natalie/value.hpp"

namespace Natalie {

struct StringValue : Value {
    const int STRING_GROW_FACTOR = 2;

    StringValue(Env *env, ClassValue *klass)
        : Value { Value::Type::String, klass } {
        m_str = GC_STRDUP("");
    }

    StringValue(Env *env)
        : StringValue { env, "" } { }

    StringValue(Env *env, std::string str)
        : Value { Value::Type::String, env->String() } {
        set_str(str.c_str());
    }

    StringValue(Env *env, const char *str)
        : Value { Value::Type::String, env->String() } {
        set_str(str);
    }

    StringValue(Env *env, const char *str, size_t length)
        : Value { Value::Type::String, env->String() } {
        set_str(str, length);
    }

    StringValue(Env *env, StringValue &other)
        : Value { env, other } {
        set_str(other.c_str(), other.length());
    }

    static StringValue *sprintf(Env *, const char *, ...);
    static StringValue *vsprintf(Env *, const char *, va_list);

    const char *c_str() const { return m_str; }
    size_t bytesize() const { return m_length; }
    size_t length() const { return m_length; }
    size_t capcity() const { return m_capacity; }
    Encoding encoding() const { return m_encoding; }

    void set_str(const char *str) {
        assert(str);
        m_str = GC_STRDUP(str);
        m_length = strlen(str);
        m_capacity = m_length;
    }

    void set_str(const char *str, size_t length) {
        assert(str);
        m_str = new char[length + 1];
        assert(strlen(str) >= length);
        strncpy(m_str, str, length);
        m_str[length] = 0;
        m_length = length;
        m_capacity = length;
    }

    void set_encoding(Encoding encoding) { m_encoding = encoding; }

    void prepend_char(Env *, char);

    void insert(Env *, size_t, char);

    void append(Env *, char);
    void append(Env *, const char *);
    void append(Env *, const std::string);
    void append(Env *, const StringValue *);
    void append(Env *, ValuePtr);

    StringValue *next_char(Env *, size_t *);
    ValuePtr each_char(Env *, Block *);
    ArrayValue *chars(Env *);

    SymbolValue *to_symbol(Env *);
    ValuePtr to_sym(Env *);

    StringValue *to_str() { return this; }

    StringValue *inspect(Env *);

    bool operator==(const Value &) const;

    StringValue *successive(Env *);

    ValuePtr index(Env *, ValuePtr);
    ValuePtr index(Env *, ValuePtr, size_t start);
    nat_int_t index_int(Env *, ValuePtr, size_t start);

    void truncate(size_t length) {
        assert(length <= m_length);
        m_str[length] = 0;
        m_length = length;
    }

    ValuePtr initialize(Env *, ValuePtr);
    ValuePtr ltlt(Env *, ValuePtr);

    bool eq(ValuePtr arg) {
        return *this == *arg;
    }

    ValuePtr to_s() {
        return this;
    }

    bool start_with(Env *, ValuePtr);
    bool end_with(Env *, ValuePtr);
    bool is_empty() { return m_length == 0; }

    ValuePtr gsub(Env *, ValuePtr, ValuePtr = nullptr, Block *block = nullptr);
    ValuePtr sub(Env *, ValuePtr, ValuePtr = nullptr, Block *block = nullptr);

    ValuePtr add(Env *, ValuePtr);
    ValuePtr bytes(Env *);
    ValuePtr cmp(Env *, ValuePtr);
    ValuePtr downcase(Env *);
    ValuePtr encode(Env *, ValuePtr);
    ValuePtr encoding(Env *);
    ValuePtr eqtilde(Env *, ValuePtr);
    ValuePtr force_encoding(Env *, ValuePtr);
    ValuePtr ljust(Env *, ValuePtr, ValuePtr);
    ValuePtr match(Env *, ValuePtr);
    ValuePtr mul(Env *, ValuePtr);
    ValuePtr ord(Env *);
    ValuePtr ref(Env *, ValuePtr);
    ValuePtr reverse(Env *);
    ValuePtr size(Env *);
    ValuePtr split(Env *, ValuePtr, ValuePtr);
    ValuePtr strip(Env *);
    ValuePtr to_i(Env *, ValuePtr);
    ValuePtr upcase(Env *);

    template <typename... Args>
    static StringValue *format(Env *env, const char *fmt, Args... args) {
        auto out = new StringValue { env };
        format(env, out, fmt, args...);
        return out;
    }

    static void format(Env *env, StringValue *out, const char *fmt) {
        for (const char *c = fmt; *c != 0; c++) {
            out->append(env, *c);
        }
    }

    template <typename T, typename... Args>
    static void format(Env *env, StringValue *out, const char *fmt, T first, Args... rest) {
        for (const char *c = fmt; *c != 0; c++) {
            if (*c == '{' && *(c + 1) == '}') {
                c++;
                out->append(env, first);
                format(env, out, c + 1, rest...);
                return;
            } else {
                out->append(env, *c);
            }
        }
    }

private:
    StringValue *expand_backrefs(Env *, StringValue *, MatchDataValue *);
    StringValue *regexp_sub(Env *, RegexpValue *, StringValue *, MatchDataValue **, StringValue **, size_t = 0);

    using Value::Value;

    void grow(Env *, size_t);
    void grow_at_least(Env *, size_t);

    void increment_successive_char(Env *, char, char, char);

    void raise_encoding_invalid_byte_sequence_error(Env *, size_t);

    char *m_str { nullptr };
    size_t m_length { 0 };
    size_t m_capacity { 0 };
    Encoding m_encoding { Encoding::UTF_8 };
};
}
