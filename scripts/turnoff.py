import dpms, time

time.sleep(1)  # ensure key presses from running this application are done

d = dpms.DPMS()  # to use the current display, or alternatively DPMS(":1")

d.enable()
d.force_level(dpms.DPMSModeOff)
