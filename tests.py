from __future__ import print_function

import dpms
import os
import sys
import unittest


class TestDpms(unittest.TestCase):
    def setUp(self):
        self.d = dpms.DPMS()
        self.d.Enable()
        # Using xfvb or a monitor that does not support DPMS,
        # you may get X11 errors after some methods.
        self.dpms_level, self.dpms_status = self.d.Info()
        self.current_timeouts = self.d.GetTimeouts()

    def tearDown(self):
        if self.dpms_status:
            self.d.Enable()
            self.d.ForceLevel(self.dpms_level)
            self.d.SetTimeouts(*self.current_timeouts)
        else:
            self.d.Disable()

    def test_init(self):
        with self.checkRaiseRegex(
            Exception,
            "Optional display keyword must be a string. e.g. ':0'"
        ):
            dpms.DPMS(display=1)

        with self.checkRaiseRegex(
            Exception,
            "Cannot open display"
        ):
            dpms.DPMS(display="invalid")

    def test_Display(self):
        self.assertEqual(
            self.d.Display(),
            os.environ["DISPLAY"]
        )

    def test_QueryExtension(self):
        extension = self.d.QueryExtension()
        self.assertTrue(type(extension) is tuple)

        self.assertTrue(type(extension[0]) is bool)
        self.assertTrue(type(extension[1]) is int)
        self.assertTrue(type(extension[2]) is int)

    def test_Capable(self):
        self.assertTrue(type(self.d.Capable()) is bool)

    def test_GetTimeouts(self):
        timeouts = self.d.GetTimeouts()
        self.assertTrue(type(timeouts) is tuple)

        for timeout in timeouts:
            self.assertTrue(type(timeout) is int)

    def test_SetTimeouts(self):
        with self.checkRaiseRegex(
            Exception,
            "Bad arguments. Should be \(int standby, int suspend, int off\)."
        ):
            self.d.SetTimeouts(standby="600", suspend="600", off="600")

        self.d.SetTimeouts(standby=100, suspend=100, off=100)
        self.assertEqual(
            self.d.GetTimeouts(),
            (100, 100, 100)
        )

    def test_Enable(self):
        self.d.Enable()
        if self.dpms_status:
            self.assertTrue(self.d.Info()[1])

    def test_Disable(self):
        self.d.Disable()
        self.assertFalse(self.d.Info()[1])

    def test_ForceLevel(self):
        current_level, _ = self.d.Info()

        with self.checkRaiseRegex(
            Exception,
            "Bad arguments. Should be \(int level\)."
        ):
            self.d.ForceLevel(level="0")

        with self.checkRaiseRegex(
            Exception,
            "Bad level."
        ):
            self.d.ForceLevel(level=1000)

        if self.dpms_status:
            print("\nYour monitor may turnoff during this test", file=sys.stderr)
            self.d.ForceLevel(level=dpms.DPMSModeOn)
            self.d.ForceLevel(level=dpms.DPMSModeStandby)
            self.d.ForceLevel(level=dpms.DPMSModeSuspend)
            self.d.ForceLevel(level=dpms.DPMSModeOff)

    def test_Info(self):
        info = self.d.Info()
        self.assertTrue(type(info) is tuple)

        level, state = info
        self.assertTrue(type(level) is int)
        self.assertTrue(type(state) is bool)

    # From https://github.com/swagger-api/swagger-codegen/pull/2529
    def checkRaiseRegex(self, expected_exception, expected_regex):
        if sys.version_info < (3, 0):
            return self.assertRaisesRegexp(expected_exception, expected_regex)
        return self.assertRaisesRegex(expected_exception, expected_regex)


if __name__ == "__main__":
    unittest.main()
