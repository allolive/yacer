/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  Image subtitle objects shown together keep their contrast when they are
 *  held to the chosen brightness: HDR ones, and SDR PGS ones that the renderer
 *  draws from a PQ copy. The renderer's glue (CRenderer::ShareSubtitlePeaks)
 *  is not compiled here; what it decides lives in OverlayRendererUtil.h.
 */

#include "cores/VideoPlayer/VideoRenderers/OverlayRendererUtil.h"

#include <vector>

#include <gtest/gtest.h>

using namespace OVERLAY;

namespace
{
//! An image subtitle object: the light it has alone, and the shared measure.
struct Object
{
  float ownNits;
  float group{0.0f};
  bool authoredHdr{false};
  bool sdrPgs{false};
  bool discMenu{false};
  bool hasPalette{true};
  double pts{1000.0};
  int measured{0};
};

constexpr bool CONVERTED = true; // PQ video with a known SDR white

SubtitlePeakSource SourceOf(Object& o, bool sdrPgsConvertedToPq)
{
  SubtitlePeakSource s;
  s.pqLight = DrawnFromPq(o.authoredHdr, o.sdrPgs, sdrPgsConvertedToPq);
  s.discMenu = o.discMenu;
  s.hasPalette = o.hasPalette;
  s.pts = o.pts;
  s.groupPeakNits = &o.group;
  s.measure = [&o]
  {
    o.measured++;
    return o.ownNits;
  };
  return s;
}

void Share(std::vector<Object>& objects, float ceiling, bool sdrPgsConvertedToPq = CONVERTED)
{
  std::vector<SubtitlePeakSource> sources;
  for (Object& o : objects)
    sources.push_back(SourceOf(o, sdrPgsConvertedToPq));
  ShareGroupPeaks(sources, ceiling);
}

//! What the texture holds the object's brightest light at: the rule of
//! COverlayTextureGLES for a PQ palette, scaling by the shared measure when
//! there is one and by the object's own otherwise, down to the ceiling only.
float DrawnPeak(const Object& o, float ceiling)
{
  const float peak = o.group > 0.0f ? o.group : o.ownNits;
  return o.ownNits * (peak > ceiling ? ceiling / peak : 1.0f);
}

Object SdrPgs(float nits)
{
  Object o;
  o.ownNits = nits;
  o.sdrPgs = true;
  return o;
}

Object Hdr(float nits)
{
  Object o;
  o.ownNits = nits;
  o.authoredHdr = true;
  return o;
}
} // namespace

// Two SDR PGS objects of different light at the same time: one measure, the
// brighter one's; the ratio between them survives the dimming.
TEST(SubtitlePeakSharing, SdrPgsObjectsWithTheSameTimeShareOneMeasure)
{
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(100.0f)};
  Share(o, 50.0f);

  EXPECT_FLOAT_EQ(o[0].group, 200.0f);
  EXPECT_FLOAT_EQ(o[1].group, 200.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[0], 50.0f), 50.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[1], 50.0f), 25.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[0], 50.0f) / DrawnPeak(o[1], 50.0f), 2.0f);
}

// Alone, the same two objects would both be brought to the ceiling.
TEST(SubtitlePeakSharing, WithoutSharingBothClampToTheCeilingAndLoseContrast)
{
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(100.0f)};
  EXPECT_FLOAT_EQ(DrawnPeak(o[0], 50.0f), 50.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[1], 50.0f), 50.0f);
}

TEST(SubtitlePeakSharing, SdrPgsAndHdrObjectsWithTheSameTimeShare)
{
  std::vector<Object> o{SdrPgs(120.0f), Hdr(300.0f)};
  Share(o, 100.0f);

  EXPECT_FLOAT_EQ(o[0].group, 300.0f);
  EXPECT_FLOAT_EQ(o[1].group, 300.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[1], 100.0f), 100.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[0], 100.0f), 40.0f);
}

TEST(SubtitlePeakSharing, ALoneObjectIsItsOwnGroup)
{
  std::vector<Object> o{Hdr(300.0f)};
  Share(o, 100.0f);
  EXPECT_FLOAT_EQ(o[0].group, 300.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[0], 100.0f), 100.0f);

  std::vector<Object> pgs{SdrPgs(80.0f)};
  Share(pgs, 100.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(pgs[0], 100.0f), 80.0f); // dimmer than the ceiling: kept
}

TEST(SubtitlePeakSharing, ADimmerGroupKeepsItsLight)
{
  std::vector<Object> o{SdrPgs(80.0f), SdrPgs(40.0f)};
  Share(o, 100.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[0], 100.0f), 80.0f);
  EXPECT_FLOAT_EQ(DrawnPeak(o[1], 100.0f), 40.0f);
}

TEST(SubtitlePeakSharing, AsAuthoredMeasuresNothing)
{
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(100.0f), Hdr(300.0f)};
  Share(o, 0.0f);
  for (const Object& x : o)
  {
    EXPECT_FLOAT_EQ(x.group, 0.0f);
    EXPECT_EQ(x.measured, 0);
  }
}

TEST(SubtitlePeakSharing, DifferentTimesAreDifferentGroups)
{
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(100.0f)};
  o[1].pts = 2000.0;
  Share(o, 50.0f);

  EXPECT_FLOAT_EQ(o[0].group, 200.0f);
  EXPECT_FLOAT_EQ(o[1].group, 100.0f);
}

TEST(SubtitlePeakSharing, ADiscMenuIsLeftOutOfTheGroupAndKeepsItsLight)
{
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(1000.0f), Hdr(900.0f)};
  o[1].discMenu = true;
  o[2].discMenu = true;
  Share(o, 50.0f);

  EXPECT_FLOAT_EQ(o[0].group, 200.0f); // not lifted to the menu's light
  EXPECT_FLOAT_EQ(o[1].group, 0.0f);
  EXPECT_FLOAT_EQ(o[2].group, 0.0f);
  EXPECT_EQ(o[1].measured, 0);
  EXPECT_EQ(o[2].measured, 0);
}

TEST(SubtitlePeakSharing, ASdrPgsNotDrawnFromPqStaysOutAsBefore)
{
  // SDR output, or PQ video without a known SDR white: the object is drawn as
  // sRGB and held by its own light.
  std::vector<Object> o{SdrPgs(200.0f), Hdr(100.0f)};
  Share(o, 50.0f, !CONVERTED);

  EXPECT_FLOAT_EQ(o[0].group, 0.0f);
  EXPECT_EQ(o[0].measured, 0);
  EXPECT_FLOAT_EQ(o[1].group, 100.0f); // the HDR object alone
}

TEST(SubtitlePeakSharing, AnObjectWithoutAPaletteIsLeftOut)
{
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(500.0f)};
  o[1].hasPalette = false;
  Share(o, 50.0f);

  EXPECT_FLOAT_EQ(o[0].group, 200.0f);
  EXPECT_FLOAT_EQ(o[1].group, 0.0f);
}

TEST(SubtitlePeakSharing, AGroupIsMeasuredOnceAndNotAgainOnTheNextFrame)
{
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(100.0f)};
  Share(o, 50.0f);
  const int first = o[0].measured + o[1].measured;
  EXPECT_EQ(first, 2);

  Share(o, 50.0f);
  EXPECT_EQ(o[0].measured + o[1].measured, first);
}

TEST(SubtitlePeakSharing, AnInvisibleGroupIsStillMarkedMeasured)
{
  std::vector<Object> o{SdrPgs(0.0f), SdrPgs(0.0f)};
  Share(o, 50.0f);
  EXPECT_TRUE(o[0].group > 0.0f);
  EXPECT_TRUE(o[1].group > 0.0f);

  Share(o, 50.0f);
  EXPECT_EQ(o[0].measured + o[1].measured, 2);
}

// What CRenderer does at the start of a frame: take up the video's composite
// settings (the SDR PGS measures follow them), then share.
namespace
{
struct Composite
{
  int transfer{0};
  float sdrWhite{0.0f};
  float sdrGamma{0.0f};
  bool operator==(const Composite& o) const
  {
    return transfer == o.transfer && sdrWhite == o.sdrWhite && sdrGamma == o.sdrGamma;
  }
};

constexpr int PQ = 16; // AVCOL_TRC_SMPTE2084

struct Renderer
{
  Composite taken; // what the renderer last drew against; default until the first frame
  bool released{false};

  //! Mirrors the order in CRenderer::ShareSubtitlePeaks.
  void Frame(std::vector<Object>& objects, const Composite& actual, float ceiling)
  {
    std::vector<float*> dependent;
    for (Object& o : objects)
      if (o.sdrPgs)
        dependent.push_back(&o.group);
    if (FollowComposite(taken, actual, dependent))
      released = true;

    const bool asPq = taken.transfer == PQ && taken.sdrWhite > 0.0f;
    Share(objects, ceiling, asPq);
  }
};
} // namespace

TEST(SubtitlePeakSharing, TheFirstFrameTakesUpTheCompositeBeforeSharing)
{
  Renderer r;
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(100.0f)};
  r.Frame(o, {PQ, 203.0f, 2.4f}, 50.0f);

  EXPECT_TRUE(r.released);
  EXPECT_FLOAT_EQ(o[0].group, 200.0f);
  EXPECT_FLOAT_EQ(o[1].group, 200.0f);
}

TEST(SubtitlePeakSharing, ACompositeChangeMeasuresSdrPgsAgainAndKeepsHdrMeasures)
{
  Renderer r;
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(100.0f), Hdr(300.0f)};
  o[2].pts = 2000.0;
  r.Frame(o, {PQ, 203.0f, 2.4f}, 50.0f);
  EXPECT_FLOAT_EQ(o[0].group, 200.0f);
  EXPECT_FLOAT_EQ(o[2].group, 300.0f);
  const int hdrMeasured = o[2].measured;

  // the objects now measure less: the white they are drawn at is lower
  o[0].ownNits = 100.0f;
  o[1].ownNits = 50.0f;
  r.released = false;
  r.Frame(o, {PQ, 100.0f, 2.4f}, 50.0f);

  EXPECT_TRUE(r.released);
  EXPECT_FLOAT_EQ(o[0].group, 100.0f);
  EXPECT_FLOAT_EQ(o[1].group, 100.0f);
  EXPECT_FLOAT_EQ(o[2].group, 300.0f);
  EXPECT_EQ(o[2].measured, hdrMeasured);
}

TEST(SubtitlePeakSharing, AnUnchangedCompositeChangesNothing)
{
  Renderer r;
  std::vector<Object> o{SdrPgs(200.0f), SdrPgs(100.0f)};
  r.Frame(o, {PQ, 203.0f, 2.4f}, 50.0f);
  const int measured = o[0].measured + o[1].measured;
  r.released = false;
  o[0].ownNits = 10.0f; // would show if it were measured again

  r.Frame(o, {PQ, 203.0f, 2.4f}, 50.0f);
  EXPECT_FALSE(r.released);
  EXPECT_FLOAT_EQ(o[0].group, 200.0f);
  EXPECT_EQ(o[0].measured + o[1].measured, measured);
}

TEST(SubtitlePeakSharing, WithoutPqVideoSdrPgsStayOutAfterTheCompositeIsTaken)
{
  Renderer r;
  std::vector<Object> o{SdrPgs(200.0f)};
  r.Frame(o, {0, 0.0f, 0.0f}, 50.0f);
  EXPECT_FALSE(r.released);
  EXPECT_FLOAT_EQ(o[0].group, 0.0f);
}
