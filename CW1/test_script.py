import unittest
import subprocess
import os 


class TestMaze(unittest.TestCase):
    # 无参数运行
    def test_no_args(self):
        result = subprocess.run(['./maze']),
