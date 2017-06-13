import re
from subprocess import Popen, PIPE

def generate_tests(filename, funcname):
  lines = []

  print filename, funcname
  proc = Popen(['python3', 'syminputC.py', funcname, filename[:-3] + '.c'], stdout=PIPE)
  lines = proc.communicate()[0].split('\n')

  i = 0

  # find number of TC
  '''numCases = 0
  numTestsStr = 'KLEE: done: generated tests ='
  for line in lines:
    i += 1
    ndx = line.find(numTestsStr)
    if ndx != -1:
      numCases = int(line[ndx + len(numTestsStr):].strip())
      break'''

  numObjsStr = 'num objects:'
  testcases = []
  #for j in range(0, numCases):
  while len(lines) != 0:
    lines = lines[i:]
    i = 0
    # find number of obj for TC
    numObjs = 0
    for line in lines:
      i += 1
      ndx = line.find(numObjsStr)
      if ndx != -1:
        numObjs = int(line[ndx+ + len(numObjsStr):].strip())
        break

    if numObjs == 0:
      break

    # generate_tests objects for TC
    dataStr = 'data: '
    sizeStr = 'size: '
    case = []
    noVersion = True
    for k in range(0, numObjs):
      name = lines[i + k * 3]
      size = lines[i + k * 3 + 1]
      data = lines[i + k * 3 + 2]

      
      if re.compile(r"'macke_sizeof_.+'").search(name) != None or re.compile(r"'model_version'").search(name) != None and noVersion:
        noVersion = False
        continue

      data = data[data.find(dataStr) + len(dataStr):].strip()
      size = int(size[size.find(sizeStr) + len(sizeStr):].strip())

      if data.lstrip('-').isdigit():
        data = int(data)
        print 'int:', data
      else:
        # Little endian Byte array
        data = data[1:-1].decode('string-escape')
        uni = data
        data = bytearray(data)
        print 'char or bytes', uni.encode('hex')


      if name.find('macke_result') != -1:
        case.insert(0, data)
      else:
        case.append(data)
    testcases.append(case)


  return testcases

print generate_tests('test.bc', 'readKey')
#print generate_tests('out2', 'a')
#print generate_tests('out3', 'a')
#print generate_tests('out4', 'a')
#print generate_tests('out5', 'a')
#print generate_tests('out6', 'a')
