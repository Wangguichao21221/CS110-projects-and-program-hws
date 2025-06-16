#!/usr/bin/env python

import os
import os.path
import tempfile
import subprocess
import time
import signal
import re
import sys
import shutil

file_locations = os.getcwd()
# logisim_location = os.path.join(os.getcwd(), os.pardir)
logisim_location = os.getcwd()
logisim_location = os.path.join(logisim_location,"../logisim-evolution.jar")

class TestCase():
  """
      Runs specified circuit file and compares output against the provided reference trace file.
  """

  def __init__(self, circfile, tracefile, maskfile, register_doc):
    self.circfile  = circfile
    self.tracefile = tracefile
    self.maskfile  = maskfile
    self.register_doc = register_doc

  def __call__(self, filename):
    output = tempfile.TemporaryFile(mode='r+')
    try:
      stdinf = open('/dev/null')
    except Exception as e:
      try:
        stdinf = open('nul')
      except Exception as e:
         print("Could not open nul or /dev/null. Program will most likely error now.")
    # start_time = time.time()
    try:
      proc = subprocess.Popen(["java","-jar",logisim_location,"--tty","table",self.circfile], stdin=stdinf, stdout=subprocess.PIPE)
      proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
      proc.kill()
      print("Process took too long to complete and was terminated.")
      return (False, "Test timed out")
    # proc = subprocess.Popen(["java","-jar",logisim_location,"--tty","table",self.circfile], stdin=stdinf, stdout=subprocess.PIPE)
    # if time.time() - start_time > 1:
    #   os.kill(proc.pid,signal.SIGTERM)
    #   print("Test timed out")
    #   return (False, "Test timed out")
    try:
      reference = open(self.tracefile)
      # mask = open(self.maskfile)
      passed = compare_unbounded(proc.stdout,reference, filename)
      # passed = compare_with_mask(proc.stdout, reference, mask, filename)
    finally:
      try: 
        os.kill(proc.pid,signal.SIGTERM)
      except Exception as e: 
        pass
    if passed:
      return (True, "Matched expected output")
    else:
      return (False, "Did not match expected output")

def compare_unbounded(student_out, reference_out, filename):
  passed = True
  student_output_array = []
  while True:
    line1 = student_out.readline().rstrip().decode("utf-8", "namereplace")
    line2 = reference_out.readline().rstrip()
    if line2 == '':
      break
    student_output_array.append(line1)
    m = re.match(line2, line1)
    if m == None or m.start() != 0 or m.end() != len(line2):
      passed = False
  with open(filename, "w") as student_output:
    for line in student_output_array:
      student_output.write(line + '\n')
  return passed

def compare_with_mask(student_out, reference_out, reference_mask, filename):
    # print(filename)
    passed = True
    student_output_array = []   
    # Read the reference mask 
    # mask_lines = reference_mask.readlines()
    while True:
        line1 = student_out.readline().rstrip().decode("utf-8", "namereplace")
        line2 = reference_out.readline().rstrip()
        mask_line = reference_mask.readline().rstrip()
        if line2 == '':
            break
        # Apply the mask to the reference output
        masked_line1 = ''.join([line1[i] if mask_line[i].strip() == '1' else ' ' for i in range(len(line1))])
        masked_line2 = ''.join([line2[i] if mask_line[i].strip() == '1' else ' ' for i in range(len(line2))])
        
        student_output_array.append(line1)
        m = re.match(masked_line2, masked_line1)
        
        if m is None or m.start() != 0 or m.end() != len(masked_line2):
            passed = False
    
    # Write student output to the specified file
    with open(filename, "w") as student_output:
        for line in student_output_array:
            student_output.write(line + '\n')
    
    return passed


def run_tests(tests):
  print("Testing files...")
  tests_passed = 0
  tests_failed = 0

  for description,filename,test in tests:
    test_passed, reason = test(filename)
    if test_passed:
      print("\t\033[92mPASSED\033[0m test: %s" % description)
      tests_passed += 1
      
    else:
      print("\t\033[91mFAILED\033[0m test: %s (%s)" % (description, reason))
      tests_failed += 1
  
  print("Passed %d/%d tests" % (tests_passed, (tests_passed + tests_failed)))


#local test list
tests = [
    ("Testcase 0 (Local test)", "student_output/p21_top_localtest.csv",TestCase(os.path.join(file_locations,'circ_files/p21_localtest.circ'), os.path.join
(file_locations,'reference_output/proj_2_1_case0.csv'), "", ""))
]

if __name__ == '__main__':
  run_tests(tests)