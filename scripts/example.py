import dpms

d = dpms.DPMS()  # to use the current display, or alternatively DPMS(":1")

# print up the display
print("Display         :", d.display())

# query extension, should return True as the first element
print("Query Extension :", d.query_extension())

# query Capable, not sure what it does
print("Capable         :", d.capable())

# query current version
print("Version         :", d.get_version())

# query the current state of things
(level, enabled) = d.info()

print("DPMS enabled    :", enabled)
if level == dpms.DPMSModeOn:
    current_level = "On"
elif level == dpms.DPMSModeStandby:
    current_level = "Standby"
elif level == dpms.DPMSModeSuspend:
    current_level = "Suspend"
elif level == dpms.DPMSModeOff:
    current_level = "Off"
else:
    current_level = "Unknown (%d)" % level

print("Current Level   :", current_level, "(", level, ")")

# query the current timeout settings
(standby, suspend, off) = d.get_timeouts()
print("Timeouts")
print("  Standby       :", standby)
print("  Suspend       :", suspend)
print("  Off           :", off)


# We have queried everything, now we will set everything back to its
# current state. We set back to the current state so as not to alter
# any state and so that you can see examples of setting state.

# set the timeout settings
d.set_timeouts(standby, suspend, off)

# enable/disable DPMS
if enabled:
    d.enable()
else:
    d.disable()

# force DPMS to a certain level
d.force_level(level)

# if you wanted to force the monitor off, you could
# d.force_level(dpms.DPMSModeOff)

# to force it back on
# d.force_level(dpms.DPMSModeOn)

# You can also suspend or standby
# d.force_level(dpms.DPMSModeSuspend)
# d.force_level(dpms.DPMSModeStandby)
