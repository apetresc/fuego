//----------------------------------------------------------------------------
/** @file GoUctUtilTest.cpp
    Unit tests for GoUctUtil.
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "GoBoard.h"
#include "GoSetupUtil.h"
#include "GoUctUtil.h"

using namespace std;
using GoUctUtil::DoSelfAtariCorrection;
using SgPointUtil::Pt;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

/** Test GoUctUtil::DoSelfAtariCorrection (self-atari) */
BOOST_AUTO_TEST_CASE(GoUctUtilTest_DoSelfAtariCorrection_1)
{
    // 3 . . . .
    // 2 X X O .
    // 1 . O . .
    //   A B C D
    GoSetup setup;
    setup.AddBlack(Pt(1, 2));
    setup.AddBlack(Pt(2, 2));
    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(3, 2));
    setup.m_player = SG_WHITE;
    GoBoard bd(19, setup);
    SgPoint p = Pt(1, 1);
    DoSelfAtariCorrection(bd, p);
    BOOST_CHECK_EQUAL(p, Pt(3, 1));
    DoSelfAtariCorrection(bd, p);
    BOOST_CHECK_EQUAL(p, Pt(3, 1));
    bd.SetToPlay(SG_BLACK);
    p = Pt(1, 1);
    DoSelfAtariCorrection(bd, p);
    BOOST_CHECK_EQUAL(p, Pt(1, 1));
}

/** Test GoUctUtil::DoSelfAtariCorrection (no self-atari; capture) */
BOOST_AUTO_TEST_CASE(GoUctUtilTest_DoSelfAtariCorrection_2)
{
    // 3 O O . .
    // 2 X X O .
    // 1 . O . .
    //   A B C D
    GoSetup setup;
    setup.AddBlack(Pt(1, 2));
    setup.AddBlack(Pt(2, 2));
    setup.AddWhite(Pt(1, 3));
    setup.AddWhite(Pt(2, 1));
    setup.AddWhite(Pt(2, 3));
    setup.AddWhite(Pt(3, 2));
    setup.m_player = SG_WHITE;
    GoBoard bd(19, setup);
    SgPoint p = Pt(1, 1);
    DoSelfAtariCorrection(bd, p);
    BOOST_CHECK_EQUAL(p, Pt(1, 1));
}

/** Test GoUctUtil::DoSelfAtariCorrection (single stone) */
BOOST_AUTO_TEST_CASE(GoUctUtilTest_DoSelfAtariCorrection_3)
{
    // 3 . .
    // 2 X .
    // 1 . .
    //   A B C D
    GoSetup setup;
    setup.AddBlack(Pt(1, 2));
    setup.m_player = SG_WHITE;
    GoBoard bd(19, setup);
    // single stone self-atari is replaced by adjacent empty point
    SgPoint p = Pt(1, 1);
    DoSelfAtariCorrection(bd, p);
    BOOST_CHECK_EQUAL(p, Pt(2, 1));
}

/** Test GoUctUtil::DoSelfAtariCorrection (single stone and capture) */
BOOST_AUTO_TEST_CASE(GoUctUtilTest_DoSelfAtariCorrection_4)
{
    std::string s("..X.\n"
                  "OX..\n"
                  "XOX.\n"
                  "....");
    int boardSize;
    GoSetup setup = GoSetupUtil::CreateSetupFromString(s, boardSize);
    setup.m_player = SG_BLACK;
    GoBoard bd(boardSize, setup);
    // single stone capture is not replaced
    SgPoint p = Pt(1, 1);
    DoSelfAtariCorrection(bd, p);
    BOOST_CHECK_EQUAL(p, Pt(1, 1));
    p = Pt(4, 1); // extension replaced by capture
    DoSelfAtariCorrection(bd, p);
    BOOST_CHECK_EQUAL(p, Pt(4, 2));
}

/** Test GoUctUtil::DoSelfAtariCorrection (single stone and capture) */
BOOST_AUTO_TEST_CASE(GoUctUtilTest_DoSelfAtariCorrection_5)
{
    std::string s("XO..O.\n"
                  ".XOO..\n"
                  "......\n"
                  "......\n"
                  "......\n"
                  "......");
    int boardSize;
    GoSetup setup = GoSetupUtil::CreateSetupFromString(s, boardSize);
    setup.m_player = SG_BLACK;
    GoBoard bd(boardSize, setup);
    // replace single stone selfatari by capture
    SgPoint p = Pt(1, 4);
    DoSelfAtariCorrection(bd, p);
    BOOST_CHECK_EQUAL(p, Pt(1, 3));
}

//----------------------------------------------------------------------------

} // namespace