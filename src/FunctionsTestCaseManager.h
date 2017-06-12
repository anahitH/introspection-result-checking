#pragma once

#include "Python.h"
#include "FunctionTestCase.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

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
    void collect_test_cases(llvm::Module& M, const result_checking::SensitiveFunctionInformation& sensitive_function_info)
    {
/*
        // get from python
        std::string name = "readKey";
        FunctionTestCase test_case(name);

        FunctionTestCase::ValueTy return_val(new IntValue(42));
        FunctionTestCase::ValueTy input_arg(new CharValue('^'));
        test_case.add_test_case(FunctionTestCase::TestCaseType{std::move(return_val), std::move(input_arg)});

        FunctionTestCase::ValueTy return_val1(new IntValue(43));
        FunctionTestCase::ValueTy input_arg1(new CharValue('*'));
        test_case.add_test_case(FunctionTestCase::TestCaseType{std::move(return_val1), std::move(input_arg1)});

        function_test_cases.insert(std::make_pair(name, test_case));
*/
//        auto& sensitive_function_info = getAnalysis<SensitiveFunctionMarkPass>().get_sensitive_functions_info();
        auto& id = M.getModuleIdentifier();

        // Set PYTHONPATH TO working directory
        setenv("PYTHONPATH",".",1);
  
        Py_Initialize();

        // setup python function call
        PyObject* myModule = PyImport_Import(PyString_FromString("get_tc"));
        PyObject* pyFunction = PyObject_GetAttrString(myModule, "generate_tests");

        // For each function, filename
        for (auto& F : M) {
            if (sensitive_function_info.is_sensitive_function(&F)) {
               function_test_cases.insert(std::make_pair(F.getName(), get_test_case(pyFunction, id, F.getName())));
            }
        }

        Py_Finalize();
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
    
    FunctionTestCase get_test_case(PyObject* pyFunction, std::string filename, std::string function) {
        FunctionTestCase tcs(function);

        PyObject* args = PyTuple_Pack(2, PyString_FromString(filename.c_str()), PyString_FromString(function.c_str()));

        // Call Python function to create testcases
        PyObject* testCases = PyObject_CallObject(pyFunction, args);

        // For each testcase
        for (int i = 0; i < PyList_Size(testCases); i++) {
            PyObject* testCase = PyList_GetItem(testCases, i);
            FunctionTestCase::TestCaseType tc;
            // for each argument
            for (int j = 0; j < PyList_Size(testCase); j++) {
                PyObject* arg = PyList_GetItem(testCase, j);
                if (PyInt_Check(arg)) {
                    long data = PyInt_AsLong(arg);
                    FunctionTestCase::ValueTy val(new IntValue(data));
                    tc.push_back(std::move(val));
                }
                else if (PyByteArray_Check(arg)) {
                    char* data = PyByteArray_AsString(arg);
                    int size = PyByteArray_Size(arg);
                    if (size == 1) {
                        FunctionTestCase::ValueTy val(new CharValue(data[0]));
                        tc.push_back(std::move(val));
                    }
                    else {
                        std::vector<char> arr;
                        for (int k = 0; k < size; k++) {
                            arr.push_back(data[k]);
                        }
                        FunctionTestCase::ValueTy val(new ByteArrayValue(arr));
                        tc.push_back(std::move(val));
                    }
                }
            }
            tcs.add_test_case(tc);
        }

        return tcs;
    }

  
};

}
