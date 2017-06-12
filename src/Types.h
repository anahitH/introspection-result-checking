#pragma once

#include "llvm/IR/Constants.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/LLVMContext.h"

#include <vector>

namespace result_checking {

using Byte_array = std::vector<char>;

class Type
{
public:
    enum type
    {
        INT,
        CHAR,
        BYTE_ARRAY
    };

public:
    Type(type t)
        : m_type(t)
    {
    }

    bool is_int() const
    {
        return m_type == INT;
    }

    bool is_char() const
    {
        return m_type == CHAR;
    }

    bool is_byte_array() const
    {
        return m_type == BYTE_ARRAY;
    }

private:
    type m_type;
};


class Value
{
public:
    Value(const Type& ty)
        : type(ty)
    {
    }

    virtual ~Value()
    {
    }

public:
    Type get_type() const
    {
        return type;
    }

    bool is_int() const
    {
        return type.is_int();
    }

    bool is_char() const
    {
        return type.is_char();
    }

    bool is_byte_array() const
    {
        return type.is_byte_array();
    }

public:
    virtual llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx) const = 0;

protected:
    Type type;
};


class IntValue : public Value
{
public:
    IntValue(int val)
        : Value(Type::INT)
        , value(val)
    {
    }

public:
    llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx) const override
    {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(Ctx), value);
    }

private:
    int value;
};

class CharValue : public Value
{
public:
    CharValue(char val)
        : Value(Type::CHAR)
        , value(val)
    {
    }

public:
    llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx) const override
    {
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(Ctx), value);
    }

private:
    char value;
};

class ByteArrayValue : public Value
{
public:
    ByteArrayValue(Byte_array array)
        : Value(Type::BYTE_ARRAY)
        , value(array)
    {
    }

public:
    std::vector<int> to_int_vector(const Byte_array& byte_array)
    {
        return std::vector<int>();
    }

    int* to_int_array(const Byte_array& byte_array)
    {
        //TODO:
        return nullptr;
    }

public:
    llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx) const override
    {
        // TODO:
        return nullptr;
    }

private:
    Byte_array value;
};
}



