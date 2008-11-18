//----------------------------------------------------------------------------
/** @file GoGtpExtraCommands.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "GoGtpExtraCommands.h"

#include "GoBoard.h"
#include "GoBoardUtil.h"
#include "GoGtpCommandUtil.h"
#include "GoLadder.h"
#include "GoStaticLadder.h"

using namespace std;

using GoGtpCommandUtil::PointArg;
using GoGtpCommandUtil::StoneArg;

//----------------------------------------------------------------------------

GoGtpExtraCommands::GoGtpExtraCommands(GoBoard& bd)
    : m_bd(bd)
{
}

void GoGtpExtraCommands::AddGoGuiAnalyzeCommands(GtpCommand& cmd)
{
    cmd <<
        "sboard/Go CFG Distance/go_cfg_distance %p\n"
        "string/Go Ladder/go_ladder %p\n"
        "string/Go Static Ladder/go_static_ladder %p\n";
}

/** Compute the distance from a point as defined in GoBoardUtil::CfgDistance.
    This command is compatible with GoGui's analyze command type
    @c sboard <br>
    Argument: point <br>
    Returns: Board of integer numbers
*/
void GoGtpExtraCommands::CmdCfgDistance(GtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    SgPoint p = PointArg(cmd, 0, m_bd);
    cmd << SgWritePointArray<int>(GoBoardUtil::CfgDistance(m_bd, p),
                                  m_bd.Size());
}

/** Return fast ladder status.
    Arguments: prey point<br>
    Returns: escaped|captured|unsettled<br>
    @see FastLadderStatus
*/
void GoGtpExtraCommands::CmdLadder(GtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    SgPoint prey = StoneArg(cmd, 0, m_bd);
    GoLadderStatus status = GoLadderUtil::LadderStatus(m_bd, prey);
    switch (status)
    {
    case GO_LADDER_ESCAPED:
        cmd << "escaped";
        break;
    case GO_LADDER_CAPTURED:
        cmd << "captured";
        break;
    case GO_LADDER_UNSETTLED:
        cmd << "unsettled";
        break;
    default:
        throw GtpFailure() << "Unexpected ladder status: " << status;
    }
}

/** Return static ladder status.
    Arguments: prey point<br>
    Returns: escaped|captured|unsettled<br>
    @see GoStaticLadder
*/
void GoGtpExtraCommands::CmdStaticLadder(GtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    SgPoint p = StoneArg(cmd, 0, m_bd);
    SgBlackWhite c = m_bd.GetColor(p);
    if (GoStaticLadder::IsLadder(m_bd, p, c))
        cmd << "captured";
    else if (GoStaticLadder::IsLadder(m_bd, p, SgOppBW(c)))
        cmd << "unsettled";
    else
        cmd << "escaped";
}

void GoGtpExtraCommands::Register(GtpEngine& e)
{
    Register(e, "go_cfg_distance", &GoGtpExtraCommands::CmdCfgDistance);
    Register(e, "go_ladder", &GoGtpExtraCommands::CmdLadder);
    Register(e, "go_static_ladder", &GoGtpExtraCommands::CmdStaticLadder);
}

void GoGtpExtraCommands::Register(GtpEngine& engine,
                               const std::string& command,
                               GtpCallback<GoGtpExtraCommands>::Method method)
{
    engine.Register(command,
                    new GtpCallback<GoGtpExtraCommands>(this, method));
}

//----------------------------------------------------------------------------
