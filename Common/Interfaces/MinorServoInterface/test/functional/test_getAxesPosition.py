from __future__ import with_statement

import os
import math
import time
import datetime

import unittest2

import MinorServo
import Management
import Antenna

from Acspy.Clients.SimpleClient import PySimpleClient
from MinorServoErrors import MinorServoErrorsEx
from Acspy.Common.TimeHelper import getTimeStamp

__author__ = "Marco Buttu <mbuttu@oa-cagliari.inaf.it>"


class TestGetAxesPosition(unittest2.TestCase):

    telescope = os.getenv('TARGETSYS')

    @classmethod
    def setUpClass(cls):
        cls.client = PySimpleClient()
        cls.boss = cls.client.getComponent('MINORSERVO/Boss')
        
    @classmethod
    def tearDownClass(cls):
        cls.client.releaseComponent('MINORSERVO/Boss')

    def setUp(self):
        self.setup_code = "CCB" if self.telescope == "SRT" else "CCC"

    def test_not_ready(self):
        """Raise a MinorServoErrorsEx in case the system is not ready"""
        with self.assertRaises(MinorServoErrorsEx):
            position = self.boss.getAxesPosition(0)

    def test_ready(self):
        """Get the axes position"""
        self.boss.setup(self.setup_code)
        counter = 0 # Seconds
        now = time_ref = datetime.datetime.now()
        while not self.boss.isReady() or (time_ref - now).seconds < 20:
            time.sleep(1)
            now = datetime.datetime.now()

        position = self.boss.getAxesPosition(0)
        self.assertTrue(any(position)) 


if __name__ == '__main__':
    if 'Configuration' in os.getenv('ACS_CDB'):
        unittest2.main() # Real test using the antenna CDB
    else:
        from PyMinorServoTest import simunittest
        simunittest.run(TestGetAxesPosition)
