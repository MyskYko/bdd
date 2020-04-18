#!/usr/bin/python3

from __future__ import print_function
#import adddeps  # fix sys.path
import os
import math

import opentuner
from opentuner import ConfigurationManipulator
from opentuner import IntegerParameter
from opentuner.search.manipulator import LogIntegerParameter
from opentuner.search.manipulator import PowerOfTwoParameter
from opentuner.search.manipulator import SwitchParameter
from opentuner.search.manipulator import BooleanParameter
from opentuner import MeasurementInterface
from opentuner import Result

class Tuner(MeasurementInterface):
  run_cmd = ""
  filename = ""
  params = [];

  def manipulator(self):
    """
    Define the search space by creating a
    ConfigurationManipulator
    """
    manipulator = ConfigurationManipulator()
    for i, param in enumerate(self.params):
      if param[0] == "None":
        continue
      if param[0] == "Int":
        manipulator.add_parameter(IntegerParameter(i, int(param[1]), int(param[2])))
        continue
      if param[0] == "Log":
        manipulator.add_parameter(LogIntegerParameter(i, int(param[1]), int(param[2])))
        continue
      if param[0] == "Pow":
        manipulator.add_parameter(PowerOfTwoParameter(i, 2 ** int(param[1]), 2 ** int(param[2])))
        continue
      if param[0] == "Switch":
        manipulator.add_parameter(SwitchParameter(i, int(param[1])))
        continue
      if param[0] == "Bool":
        manipulator.add_parameter(BooleanParameter(i))
        continue
    return manipulator

  def run(self, desired_result, input, limit):
    """
    Compile and run a given configuration then
    return performance
    """
    cfg = desired_result.configuration.data

    f = open(self.filename, mode="w")
    for i, param in enumerate(self.params):
      if param[0] == "None":
        f.write(param[1] + "\n")
        continue
      f.write(str(cfg[i]) + "\n")
    f.close()
    print(cfg)
    run_result = self.call_program(self.run_cmd, limit=120)
    print(run_result)
    if run_result['returncode'] != 0:
      return Result(time=float('inf'))
    return Result(time=run_result['time'])

  def save_final_config(self, configuration):
    cfg = configuration.data
    f = open(self.filename, mode="w")
    print("### Results ###")
    for i, param in enumerate(self.params):
      if param[0] == "None":
        f.write(param[1] + "\n")
        continue
      f.write(str(cfg[i]) + "\n")
      if param[0] == "Int" or param[0] == "Log":
        print(str(param[1]) + " < " + str(cfg[i]) + " < " + str(param[2]))
        continue
      if param[0] == "Pow":
        print(str(2 ** int(param[1])) + " < " + str(cfg[i]) + " < " + str(2 ** int(param[2])))
        continue
      if param[0] == "Bool" or param[0] == "Switch":
        print(cfg[i])
        continue
    f.close()

if __name__ == '__main__':
  argparser = opentuner.default_argparser()
  argparser.add_argument('command', help='command to run the program')
  argparser.add_argument('header', help='file including struct of parameters')
  args = argparser.parse_args()
  Tuner.run_cmd = args.command
  Tuner.filename = "_" + os.path.basename(args.header) + "_setting.txt"

  f = open(args.header)
  lines = f.readlines();
  fparam = 0
  for line in lines:
    words = line.split()
    if not words:
      continue
    if words[0] == "//" and words[1] == "Param":
      fparam = 1
      continue
    if words[0] == "//" and words[1] == "end":
      break
    if not fparam:
      continue
    param = words.index("//")
    assert param != -1
    print(words[param+1:])
    Tuner.params += [words[param+1:]]
  f.close()

  Tuner.main(args)
