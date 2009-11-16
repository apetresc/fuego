//----------------------------------------------------------------------------
/** @file SgSearchTracer.cpp
    See SgSearchTracer.h.
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgSearchTracer.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <math.h>
#include "SgDebug.h"
#include "SgHashTable.h"
#include "SgVector.h"
#include "SgMath.h"
#include "SgNode.h"
#include "SgSearchValue.h"
#include "SgTime.h"
#include "SgWrite.h"

using namespace std;

//----------------------------------------------------------------------------
SgSearchTracer::SgSearchTracer(SgNode* root) : m_traceNode(root)
{ }

void SgSearchTracer::AddMoveProp(SgNode* node, SgMove move, SgBlackWhite player)
{
    // GoSearch uses SgPropMove
    node->AddMoveProp(move, player);
}

void SgSearchTracer::AddTraceNode(SgMove move, SgBlackWhite player)
{
    if (m_traceNode)
    {
        m_traceNode = m_traceNode->NewRightMostSon();
        AddMoveProp(m_traceNode, move, player);
    }
}

void SgSearchTracer::AppendTrace(SgNode* toNode)
{
    if (m_traceNode)
    {
        m_traceNode->Root()->AppendTo(toNode);
        m_traceNode = 0;
    }
}

void SgSearchTracer::InitTracing(const string& type)
{
    SG_ASSERT(! m_traceNode);
    if (TraceIsOn())
    {
        m_traceNode = new SgNode();
        m_traceNode->Add(new SgPropText(SG_PROP_COMMENT, type));
    }
}

void SgSearchTracer::StartOfDepth(int depth)
{
    SG_ASSERT(m_traceNode);
    if (depth > 0 && m_traceNode->HasFather())
    {
        // true for each depth except the very first
        // AR: the 0 should really be the depthMin parameter of iterated
        // search. this will break if depthMin != 0 and generate strange
        // trace trees.
        m_traceNode = m_traceNode->Father();
        // go from root of previous level to root
    }
    m_traceNode = m_traceNode->NewRightMostSon();
    SG_ASSERT(m_traceNode);
    m_traceNode->SetIntProp(SG_PROP_MAX_DEPTH, depth);
    ostringstream o;
    o << "Iteration d=" << depth << ' ';
    m_traceNode->AddComment(o.str());

    // @todo would be interesting to know time used for each depth,
    // create SG_PROP_TIME_USED property at EndOfDepth (doesn't exist yet)
}

void SgSearchTracer::TakeBackTraceNode()
{
    SG_ASSERT(m_traceNode);
    if (m_traceNode)
        m_traceNode = m_traceNode->Father();
}

void SgSearchTracer::TraceComment(const char* comment) const
{
    if (m_traceNode)
    {
        m_traceNode->AddComment(comment);
        m_traceNode->AddComment("\n");
    }
}

void SgSearchTracer::TraceValue(int value, SgBlackWhite toPlay) const
{
    // The value needs to be recorded in absolute terms, not relative to
    // the current player.
    int v = (toPlay == SG_WHITE) ? -value : +value;
    m_traceNode->Add(new SgPropValue(SG_PROP_VALUE, v));
    ostringstream comment;
    comment << "v=" << v;
    TraceComment(comment.str().c_str());
}

void SgSearchTracer::TraceValue(int value, SgBlackWhite toPlay,
								const char* comment, bool isExact) const
{
    TraceValue(value, toPlay);
    if (comment != 0)
        TraceComment(comment);
    if (isExact)
    {
        m_traceNode->Add(new SgPropMultiple(SG_PROP_CHECK, 1));
        TraceComment("exact");
    }
}

//----------------------------------------------------------------------------

