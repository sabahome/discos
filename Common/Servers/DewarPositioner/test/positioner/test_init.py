import unittest2
import random
import time
from DewarPositioner.positioner import Positioner, NotAllowedError


class PositionerInitTest(unittest2.TestCase):

    def test___init__(self):
        """Verify the right behavior of the initializer"""
        cdb_info = {
                'updating_time': 0.1,
                'rewinding_timeout': 1.5,
                'rewinding_sleep_time': 1
        }
        p = Positioner(cdb_info)
        self.assertEqual(p.isUpdating(), False)
        self.assertEqual(p.isConfigured(), False)
        self.assertEqual(p.isTerminated(), True)
        self.assertEqual(p.getOffset(), 0.0)
        self.assertEqual(p.getStartingPosition(), 0.0)
        self.assertRaises(NotAllowedError, p.getDeviceName)
        self.assertRaises(NotAllowedError, p.startUpdating)

if __name__ == '__main__':
    unittest2.main()