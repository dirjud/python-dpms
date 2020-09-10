from __future__ import print_function

import dpms
import os
import sys
import unittest

print(
    """\
===========================================
Your monitor may turnoff during these tests
===========================================\
""",
    file=sys.stderr,
)


class TestDpms(unittest.TestCase):
    def setUp(self):
        self.d = dpms.DPMS()
        self.d.enable()
        # Using xfvb or a monitor that does not support DPMS,
        # you may get X11 errors after some methods.
        self.dpms_level, self.dpms_status = self.d.info()
        self.current_timeouts = self.d.get_timeouts()

    def tearDown(self):
        if self.dpms_status:
            self.d.enable()
            self.d.force_level(self.dpms_level)
            self.d.set_timeouts(*self.current_timeouts)
        else:
            self.d.disable()

    def test_init(self):
        with self.checkRaiseRegex(
            Exception, "Optional display keyword must be a string. e.g. ':0'"
        ):
            dpms.DPMS(display=1)

        with self.checkRaiseRegex(Exception, "Cannot open display"):
            dpms.DPMS(display="invalid")

    def test_display(self):
        self.assertEqual(self.d.display(), os.environ["DISPLAY"])

    def test_query_extension(self):
        extension = self.d.query_extension()
        self.assertTrue(type(extension) is tuple)

        self.assertTrue(type(extension[0]) is bool)
        self.assertTrue(type(extension[1]) is int)
        self.assertTrue(type(extension[2]) is int)

    def test_capable(self):
        self.assertTrue(type(self.d.capable()) is bool)

    def test_get_timeouts(self):
        timeouts = self.d.get_timeouts()
        self.assertTrue(type(timeouts) is tuple)

        for timeout in timeouts:
            self.assertTrue(type(timeout) is int)

    def test_set_timeouts(self):
        with self.checkRaiseRegex(
            Exception, "Bad arguments. Should be \(int standby, int suspend, int off\)."
        ):
            self.d.set_timeouts(standby="600", suspend="600", off="600")

        self.d.set_timeouts(standby=100, suspend=100, off=100)
        self.assertEqual(self.d.get_timeouts(), (100, 100, 100))

    def test_enable(self):
        self.d.enable()
        if self.dpms_status:
            self.assertTrue(self.d.info()[1])

    def test_disable(self):
        self.d.disable()
        self.assertFalse(self.d.info()[1])

    def test_force_level(self):
        current_level, _ = self.d.info()

        with self.checkRaiseRegex(Exception, "Bad arguments. Should be \(int level\)."):
            self.d.force_level(level="0")

        with self.checkRaiseRegex(Exception, "Bad level."):
            self.d.force_level(level=1000)

        if self.dpms_status:
            self.d.force_level(level=dpms.DPMSModeOn)
            self.d.force_level(level=dpms.DPMSModeStandby)
            self.d.force_level(level=dpms.DPMSModeSuspend)
            self.d.force_level(level=dpms.DPMSModeOff)

    def test_info(self):
        info = self.d.info()
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
