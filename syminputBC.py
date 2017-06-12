import argparse
from tempfile import TemporaryDirectory
from subprocess import run, STDOUT
from os import path

# small config section
OPT  = path.expanduser("~/build/llvm/Release/bin/opt")
MACKEOPT = path.expanduser("~/build/macke-opt-llvm/bin/libMackeOpt.so")
KLEE = path.expanduser("~/build/klee/Release+Asserts/bin/klee")
KTEST = path.expanduser("~/build/klee/Release+Asserts/bin/ktest-tool")


def run_KLEE(functionname, bcfile):
    # Put all KLEE generated files into a temporary directory
    with TemporaryDirectory() as tmpdir:
        run([OPT, "-load", MACKEOPT,
             "-encapsulatesymbolic", "--encapsulatedfunction=" + functionname,
             bcfile, "-o", path.join(tmpdir, "prepared.bc")], check=True)
        run([KLEE,
             "--entry-point=macke_" + functionname + "_main", "--max-time=300", "-watchdog",
             "--posix-runtime", "--libc=uclibc",
             "--only-output-states-covering-new", path.join(tmpdir, "prepared.bc")
            ], check=True)
        print()

        # show the details of all test cases
        run(["for ktest in "+ path.join(tmpdir, "klee-last") +"/*.ktest; do " + KTEST + " --write-ints $ktest; echo ""; done"], shell=True)

        # input("Press enter to delete " + tmpdir)


if __name__ == '__main__':
    # Some argument parsing
    parser = argparse.ArgumentParser(description='Get input for arbitrary functions')
    parser.add_argument('functionname', help='name of the function to generate inputs for')
    parser.add_argument('bcfile', type=argparse.FileType('r'), help='.bc-file containing the compiled bitcode')

    args = parser.parse_args()

    run_KLEE(args.functionname, args.bcfile.name)
