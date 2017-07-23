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
        BYTE_ARRAY,
        INT_ARRAY
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

    bool is_int_array() const
    {
        return m_type == INT_ARRAY;
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
    virtual llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx, unsigned width) const = 0;

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
    llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx, unsigned width) const override
    {
        printf("int: %d\n", value);
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctx), value);
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
    llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx, unsigned width) const override
    {
        printf("char: %d\n", value);
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(Ctx), value);
    }

private:
    char value;
};

class IntArrayValue;
class ByteArrayValue : public Value
{
public:
    ByteArrayValue(Byte_array array)
        : Value(Type::BYTE_ARRAY)
        , value(array)
    {
    }

public:
    llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx, unsigned width) const override
    {
        char* data = const_cast<char*>(value.data());
        int* int_data = reinterpret_cast<int*>(data);
        // llvm::StringRef::StringRef(const char *data, size_t length);
        printf("Bytestuff\n");
        llvm::APInt ap(width, 0);
        for (int n = value.size() - 1; n >= 0; n--) // value.size(); n++)
          ap = (ap << 8) + ((unsigned char ) value.data()[n]);
        return llvm::ConstantInt::get(llvm::IntegerType::get(Ctx, width), ap);
    }
    Byte_array* getVal() {
        return &value;
    }

    // cast to int array value
    operator IntArrayValue() const;

private:
    Byte_array value;
};


class IntArrayValue : public Value
{
public:
    IntArrayValue(const Byte_array& array)
        : Value(Type::INT_ARRAY)
    {
        to_int_array(array);
    }

    ~IntArrayValue()
    {
        delete[] value;
        value = nullptr;
    }

public:
    llvm::Value* to_llvm_value(llvm::LLVMContext& Ctx, unsigned width) const override
    {
        return llvm::ConstantInt::get(llvm::Type::getInt32PtrTy(Ctx), *value);
    }

private:
    void to_int_array(const Byte_array& array)
    {
        static_assert(sizeof(int) == 4, "Expecting int to be 4 byte");
        auto size = array.size();
        if (size % 4 != 0) {
            value = nullptr;
            return;
        }
        int array_size = size / 4;
        value = new int(array_size);
        for (int i = 0, j = 0; i < size ;) {
            int num = (array[i + 3] << 24) | (array[i + 2] << 16) | (array[i + 1] << 8) | array[i];
            value[j] = num;
            i += 4;
            ++j;
        }
    }

private:
    int* value;
};

ByteArrayValue::operator IntArrayValue() const
{
    return IntArrayValue(value);
}

}



