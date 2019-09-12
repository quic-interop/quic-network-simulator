from enum import Enum
import filecmp
import os
from prettytable import PrettyTable
import random
import shutil
import subprocess
import string
from termcolor import colored

class TestResult(Enum):
  SUCCEEDED = 1
  FAILED = 2
  UNSUPPORTED = 3

# add your QUIC implementation here
IMPLEMENTATIONS = { # name => docker image
  "quicgo": "quicgo"
}
TESTCASES = { # name => letter in the interop matrix
  "transfer": "HDC", 
  "retry": "S",
  "resumption": "R",
}

def random_string(length):
    """Generate a random string of fixed length """
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(length))

def check_transfer(files):
  for file in files:
    if not os.path.isfile("downloads/" + file) or not filecmp.cmp("www/" + file, "downloads/" + file, shallow=False):
      return False
  return True

def clear_directory(path):
  for file in os.listdir(path):
    file_path = os.path.join(path, file)
    if os.path.isfile(file_path):
        os.unlink(file_path)
    elif os.path.isdir(file_path): shutil.rmtree(file_path)

def get_test_result(output):
  lines = output.splitlines()
  if any("exit status 127" in str(l) for l in lines):
    return TestResult.UNSUPPORTED
  if any("client exited with code 0" in str(l) for l in lines):
    return TestResult.SUCCEEDED
  return TestResult.FAILED

def print_results(results):
  def get_letters(testcases):
    if len(testcases) == 0:
      return "-"
    return "".join([ TESTCASES[n] for n in testcases ])
    
  t = PrettyTable()
  t.field_names = [ "" ] + [ name for name in IMPLEMENTATIONS ]
  for server in IMPLEMENTATIONS:
    row = [ server ]
    for client in IMPLEMENTATIONS:
      cell = results[server][client]
      res = colored(get_letters(cell[TestResult.SUCCEEDED]), "green") + "\n"
      res += colored(get_letters(cell[TestResult.UNSUPPORTED]), "yellow") + "\n"
      res += colored(get_letters(cell[TestResult.FAILED]), "red")
      row += [ res ]
    t.add_row(row)
  print(t)

results = {}
for server in IMPLEMENTATIONS:
  results[server] = {}
  for client in IMPLEMENTATIONS:
    results[server][client] = {
      TestResult.SUCCEEDED: [],
      TestResult.FAILED: [],
      TestResult.UNSUPPORTED: [],
    }

for server in IMPLEMENTATIONS:
  for client in IMPLEMENTATIONS:
    # (re-) build the docker images
    cmd = (
      "SERVER=" + IMPLEMENTATIONS[server] + " " 
      "CLIENT=" + IMPLEMENTATIONS[server] + " "
      "docker-compose -f ../docker-compose.yml -f interop.yml build"
    )
    os.system(cmd)
    
    # check that the client is capable of returning UNSUPPORTED
    cmd = (
        "TESTCASE=" + random_string(6) + " "
        "SCENARIO=\"simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25\" "
        "CLIENT=" + IMPLEMENTATIONS[client] + " "
        "docker-compose -f ../docker-compose.yml -f interop.yml up --timeout 0 --abort-on-container-exit sim client"
      )
    output = subprocess.run(cmd, shell=True, capture_output=True)
    status = get_test_result(output.stdout)
    if status != TestResult.UNSUPPORTED:
      continue

    # check that the server is capable of returning UNSUPPORTED
    cmd = (
        "TESTCASE=" + random_string(6) + " "
        "SERVER=" + IMPLEMENTATIONS[server] + " "
        "docker-compose -f ../docker-compose.yml -f interop.yml up server"
      )
    output = subprocess.run(cmd, shell=True, capture_output=True)
    status = get_test_result(output.stdout)
    if status != TestResult.UNSUPPORTED:
      continue

    # run the test cases
    for testcase in TESTCASES:
      print("Server: " + server + ". Client: " + client + ". Test case: " + testcase)
      cmd = (
        "TESTCASE=" + testcase + " "
        "SCENARIO=\"simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25\" "
        "CLIENT=" + IMPLEMENTATIONS[client] + " "
        "SERVER=" + IMPLEMENTATIONS[server] + " "
        "CLIENT_PARAMS=\"https://server:443/file1.html https://server:443/file2.html\" "
        "docker-compose -f ../docker-compose.yml -f interop.yml up --abort-on-container-exit --timeout 1"
      )
      output = subprocess.run(cmd, shell=True, capture_output=True)
      status = get_test_result(output.stdout)

      print("Test result:", status)
      results[server][client][status] += [ testcase ]

      check_transfer(['file1.html', 'file2.html'])
      clear_directory("downloads/")

print("Results: ")
print_results(results)
