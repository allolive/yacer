# What the sink is told over HDMI

The 4k modes that carry no VIC in the AVI are named to the sink by the HDMI 1.4b
vendor infoframe instead, and that slot is shared with the Dolby Vision and
HDR10+ payloads. Releasing it puts the name back in the same operation, so the
sink is never left without one.

The S6 HDMI PLL is set from the mode's exact rate rather than from a pixel clock
rounded to whole kHz, which left each 23.976 and 10-bit mode a few parts per
million off and dropped or repeated a frame every hour or two against a player
timing frames on the crystal.
