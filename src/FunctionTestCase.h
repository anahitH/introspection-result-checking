#pragma once

#include "Types.h"

#include <memory>

namespace result_checking {

class FunctionTestCase
{
public:
    using ValueTy = std::shared_ptr<Value>;
    using TestCaseType=std::vector<ValueTy>;
    using TestCases = std::vector<TestCaseType>;

public:
    FunctionTestCase(const std::string& n)
        : name(n)
        , test_case_size(-1)
    {
    }

public:
    void add_test_case(const TestCaseType& test_case)
    {
        if (test_case_size < 0) {
            test_case_size = test_case.size();
        } else {
            assert(test_case_size == test_case.size());
        }
        test_cases.push_back(test_case);
    }

    const TestCases& get_test_cases() const
    {
        return test_cases;
    }

private:
    const std::string name;
    TestCases test_cases;
    int test_case_size;
};

}

