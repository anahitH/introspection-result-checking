#include "Python.h"
#include <tuple>
#include <vector>

using namespace std;

struct ByteArrAndSize {
  char* data;
  int size;
};

FunctionTestCase get_test_case(char* filename, char* function) {
  FunctionTestCase tcs(function);

  // setup python function call
  PyObject* myModule = PyImport_Import(PyString_FromString("get_tc"));
  PyObject* myFunction = PyObject_GetAttrString(myModule, "generate_tests");
  PyObject* args = PyTuple_Pack(2, PyString_FromString(filename), PyString_FromString(function));

  // Call Python function to create testcases
  PyObject* testCases = PyObject_CallObject(myFunction, args);

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
        tc.push_back(val);
      }
      else if (PyByteArray_Check(arg)) {
        char* data = PyByteArray_AsString(arg);
        int size = PyByteArray_Size(arg);
        if (size == 1) {
          FunctionTestCase::ValueTy val(new CharValue(data[0]));
          tc.push_back(val);
        }
        else {
          Vector<char> arr;
          for (int k = 0; k < size; k++) {
            arr.push_back(data[k]);
          }
          FunctionTestCase::ValueTy val(new ByteArrayValue(arr));
          tc.push_back(val);
        }
      }
    }
    tcs.add_test_case(tc);
  }

  return tcs;
}

int main() {
  // Set PYTHONPATH TO working directory
  setenv("PYTHONPATH",".",1);
  
  Py_Initialize();
  
  get_test_case((char*) "out", (char*) "function");
  get_test_case((char*) "out2", (char*) "function");
  get_test_case((char*) "out3", (char*) "function");
  get_test_case((char*) "out4", (char*) "function");
  get_test_case((char*) "out5", (char*) "function");
  
  Py_Finalize();

  return 0;
}
