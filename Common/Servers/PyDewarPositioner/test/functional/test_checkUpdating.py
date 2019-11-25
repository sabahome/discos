from __future__ import with_statement, division
import unittest
import time
from Antenna import ANT_HORIZONTAL, ANT_NORTH, ANT_SOUTH
from Management import MNG_TRACK, MNG_BEAMPARK
from DewarPositioner.posgenerator import PosGenerator

from Acspy.Clients.SimpleClient import PySimpleClient
from Acspy.Common.TimeHelper import getTimeStamp
from ComponentErrors import ComponentErrorsEx


class TestCheckUpdating(unittest.TestCase):
    """Test the DewarPositioner.startUpdating() end-to-end method"""
    def setUp(self):
        self.client = PySimpleClient()
        self.device = self.client.getComponent('RECEIVERS/SRTKBandDerotator')
        self.antenna = self.client.getComponent('ANTENNA/Boss')
        self.positioner = self.client.getComponent('RECEIVERS/DewarPositioner')
        self.positioner.setup('KKG')
        self.positioner.setConfiguration('custom')
        self.lat = 0.689 # 39.5 degrees

    def tearDown(self):
        if self.positioner.isReady():
            self.positioner.stopUpdating()
            time.sleep(1)
            self.positioner.park()
        time.sleep(0.5)
        self.client.releaseComponent('RECEIVERS/DewarPositioner')


    def _test_system_not_ready(self):
        """Raise ComponentErrorsEx when the system is not ready"""
        with self.assertRaisesRegexp(ComponentErrorsEx, 'positioner not configured'):
            self.positioner.park()
            time.sleep(0.5)
            starting_time = getTimeStamp().value + 100000000
            az, el, ra, dec = 0, 0, 0, 0
            self.positioner.checkUpdating(
                    starting_time,
                    MNG_TRACK,
                    ANT_SOUTH,
                    az, el, ra, dec
            )


    def _test_mng_beampark(self):
        """Return (True, starting_time) when axis is MNG_BEAMPARK"""
        starting_time = getTimeStamp().value + 100000000
        az, el, ra, dec = 0, 0, 0, 0
        value, feasible_time = self.positioner.checkUpdating(
                starting_time,
                MNG_BEAMPARK,
                ANT_SOUTH,
                az, el, ra, dec
        )
        self.assertEqual(value, True)
        self.assertEqual(feasible_time, starting_time)


    def test_starting_time_0_feasible_time_less_than_tolerance(self):
        """We have starting_time == 0 and the system is ready (because
        it requires less than 2 seconds to reach the position). In this case
        it has to return True and a feasible_time less than current time
        plus the tolerance."""
        starting_time = 0
        az, el, ra, dec = 0.6109, 0.6109, 0, 0  # 0.6109 -> 35 degrees
        self.antenna.setOffsets(az, el, ANT_HORIZONTAL)
        self.positioner.startUpdating(MNG_TRACK, ANT_NORTH, az, el, ra, dec)
        time.sleep(0.5)
        self.positioner.stopUpdating()
        time.sleep(0.5)
        timestamp = getTimeStamp().value
        value, feasible_time = self.positioner.checkUpdating(
                starting_time,
                MNG_TRACK,
                ANT_NORTH,
                az, el, ra, dec
        )
        self.assertEqual(value, True)
        self.assertGreater(feasible_time, timestamp)
        self.assertLess(feasible_time, timestamp + 10**7)  # 1 sec


    def test_starting_time_0_feasible_time_greater_than_tolerance(self):
        """We have starting_time == 0 but the system is not ready (because
        it requires more than 2 seconds to reach the position). In this case
        it has to return False and a feasible_time greater or equal to the
        time required to reach the posizion."""
        starting_time = 0
        az, el = 1.2217, 0.6109 # 70 degrees, 35 degrees
        p0 = 0
        self.positioner.setPosition(p0)
        p1 = PosGenerator.getParallacticAngle(self.lat, az, el) # -63 degrees
        speed = self.device.getSpeed()
        required_time = abs(p1 - p0)/speed * 10**7
        timestamp = getTimeStamp().value
        value, feasible_time = self.positioner.checkUpdating(
                starting_time,
                MNG_TRACK,
                ANT_NORTH,
                az, el, 0, 0
        )
        self.assertEqual(value, False)
        minimum_time = timestamp + int(required_time)
        self.assertGreater(feasible_time, minimum_time)
        safety_time = int(1.3*minimum_time)
        self.assertLess(feasible_time, safety_time)


    def test_starting_time_not_0_feasible_time_less_than_starting_time(self):
        """We have starting_time greater than 0 and feasible_time less than
        starting_time.  In this case it has to return True and a feasible_time
        greater than now and less than starting_time."""
        az, el = 1.2217, 0.6109 # 70 degrees, 35 degrees
        p0 = 0
        self.positioner.setPosition(p0)
        p1 = PosGenerator.getParallacticAngle(self.lat, az, el) # -63 degrees
        speed = self.device.getSpeed()
        required_time = abs(p1 - p0)/speed * 10**7
        timestamp = getTimeStamp().value
        # Starting time is 2 seconds more than the required time
        starting_time = timestamp + int(required_time*1.2) + 2*10**7
        value, feasible_time = self.positioner.checkUpdating(
                starting_time,
                MNG_TRACK,
                ANT_NORTH,
                az, el, 0, 0
        )
        self.assertEqual(value, True)
        self.assertLess(feasible_time, starting_time)
        self.assertGreater(feasible_time, required_time)


    def test_starting_time_not_0_feasible_time_greater_than_starting_time(self):
        """We have starting_time greater than 0 and feasible_time greater than
        starting_time.  In this case it has to return False and a feasible_time
        greater than starting_time."""
        az, el = 1.2217, 0.6109 # 70 degrees, 35 degrees
        p0 = 0
        self.positioner.setPosition(p0)
        p1 = PosGenerator.getParallacticAngle(self.lat, az, el) # -63 degrees
        speed = self.device.getSpeed()
        required_time = abs(p1 - p0)/speed * 10**7
        timestamp = getTimeStamp().value
        # Starting time is 2 seconds more than the required time
        starting_time = timestamp + int(required_time)
        value, feasible_time = self.positioner.checkUpdating(
                starting_time,
                MNG_TRACK,
                ANT_NORTH,
                az, el, 0, 0
        )
        self.assertEqual(value, False)
        self.assertGreater(feasible_time, starting_time)
        safety_time = timestamp + int(required_time*1.2) + 2*10**7
        self.assertLess(feasible_time, safety_time)



if __name__ == '__main__':
    unittest.main()
