# Which image subtitle objects share one brightness measure. The sharing lives
# in OverlayRendererUtil.h as inline code that reaches nothing of Kodi's, so
# the suite includes it from the patched tree and builds no Kodi source.
yacer_add_test(
  NAME SubtitlePeakSharing
  SOURCES TestSubtitlePeakSharing.cpp
)
