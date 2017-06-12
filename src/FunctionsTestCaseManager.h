#pragma once

#include "FunctionTestCase.h"

#include <unordered_map>

namespace result_checking {

class FunctionsTestCaseManager
{
public:
    static FunctionsTestCaseManager& get()
    {
        static FunctionsTestCaseManager mgr;
        return mgr;
    }

public:
    void collect_test_cases()
    {
        // get from python
        std::string name = "readKey";
        FunctionTestCase::ValueTy return_val(new IntValue(5));
        FunctionTestCase::ValueTy input_arg(new CharValue('^'));
        FunctionTestCase test_case(name);
        test_case.add_test_case(FunctionTestCase::TestCaseType{std::move(return_val), std::move(input_arg)});
        function_test_cases.insert(std::make_pair(name, test_case));
    }

    const FunctionTestCase& get_function_test_case(const std::string& name) const
    {
        auto pos = function_test_cases.find(name);
        assert(pos != function_test_cases.end());
        return pos->second;
    }

private:
    FunctionsTestCaseManager() = default;
private:
    std::unordered_map<std::string, FunctionTestCase> function_test_cases;
};

}
