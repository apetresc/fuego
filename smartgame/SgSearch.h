//----------------------------------------------------------------------------
/** @file SgSearch.h
    Search engine.

    SgSearch is the search engine of the Smart Game Board, providing depth-
    first search with iterative deepening and transposition tables.
*/
//----------------------------------------------------------------------------

#ifndef SG_SEARCH_H
#define SG_SEARCH_H

#include "SgBlackWhite.h"
#include "SgHash.h"
#include "SgList.h"
#include "SgMove.h"
#include "SgSearchStatistics.h"
#include "SgTimer.h"

template <class DATA> class SgHashTable;
class SgNode;
class SgSearchControl;

//----------------------------------------------------------------------------

/** Value used in class SgSearch.
    There's a range of values that indicate that the problem has been
    solved (at a certain depth), a range of values for solutions that involve
    ko, and a range of values for problems that have not been solved yet.
    Value is always stored with positive values being good for black, negative
    values being good for white.
    <pre>
    int v = Board().ToPlay() == SG_WHITE ? -value : +value;
    </pre>
*/
class SgValue
{
public:
    enum {
        /** @todo: could make it 512 if deep ladder search a problem */
        MAX_DEPTH = 256,

        MAX_LEVEL = 125,

        /** == 32000 */
        MAX_VALUE = MAX_LEVEL * MAX_DEPTH,

        MAX_KO_LEVEL = 3,

        KO_VALUE = MAX_VALUE - MAX_DEPTH,

        MAX_ESTIMATE = KO_VALUE - MAX_KO_LEVEL * MAX_DEPTH
    };

    SgValue();

    explicit SgValue(int v);

    SgValue(SgBlackWhite goodForPlayer, int depth);

    SgValue(SgBlackWhite goodForPlayer, int depth, int koLevel);

    /** Return current value as an integer. */
    operator int() const;

    int Depth() const;

    /** Convert 'string' to a value and set this value.
        Return true if the string could be converted to a valid value,
        otherwise false.
    */
    bool FromString(const std::string& s);

    bool IsEstimate() const;

    bool IsKoValue() const;

    bool IsPositive() const;

    bool IsSureValue() const;

    int KoLevel() const;

    void SetValueForPlayer(SgBlackWhite player);

    int ValueForBlack() const;

    int ValueForPlayer(SgBlackWhite player) const;

    int ValueForWhite() const;

    /** Set '*s' to the string for this value, e.g. "B+3.5", "W+20",
        or "W+(ko)[12]". The value is divided by 'unitPerPoint' to determine
        the number of points.
    */
    std::string ToString(int unitPerPoint = 1) const;

private:
    int m_v;
};

inline SgValue::SgValue()
    : m_v(0)
{ }

inline SgValue::SgValue(int v)
    : m_v(v)
{
    SG_ASSERT(-MAX_VALUE <= v && v <= MAX_VALUE);
}

inline SgValue::SgValue(SgBlackWhite goodForPlayer, int depth)
    : m_v(MAX_VALUE - depth)
{
    SG_ASSERT_BW(goodForPlayer);
    SG_ASSERT(0 <= depth && depth < MAX_DEPTH);
    SetValueForPlayer(goodForPlayer);
    // Make sure value gets encoded/decoded consistently.
    SG_ASSERT(KoLevel() == 0);
    SG_ASSERT(Depth() == depth);
}

inline SgValue::SgValue(SgBlackWhite goodForPlayer, int depth, int koLevel)
    : m_v(MAX_VALUE - depth - koLevel * MAX_DEPTH)
{
    SG_ASSERT_BW(goodForPlayer);
    SG_ASSERT(0 <= depth && depth < MAX_DEPTH);
    SG_ASSERT(0 <= koLevel && koLevel <= MAX_KO_LEVEL);
    SetValueForPlayer(goodForPlayer);
    // Make sure value gets encoded/decoded consistently.
    SG_ASSERT(KoLevel() == koLevel);
    SG_ASSERT(Depth() == depth);
}

inline SgValue::operator int() const
{
    return m_v;
}

inline int SgValue::Depth() const
{
    if (IsEstimate())
        return 0;
    else return (MAX_DEPTH - 1) - (std::abs(m_v)-1) % MAX_DEPTH;
}

inline bool SgValue::IsEstimate() const
{
    return -MAX_ESTIMATE < m_v && m_v < MAX_ESTIMATE;
}

inline bool SgValue::IsKoValue() const
{
    return IsSureValue() && -KO_VALUE < m_v && m_v < KO_VALUE;
}

inline bool SgValue::IsPositive() const
{
    return 0 <= m_v;
}

inline bool SgValue::IsSureValue() const
{
    return m_v <= -MAX_ESTIMATE || MAX_ESTIMATE <= m_v;
}

inline void SgValue::SetValueForPlayer(SgBlackWhite player)
{
    if (player == SG_WHITE)
        m_v = -m_v;
}

inline int SgValue::ValueForBlack() const
{
    return +m_v;
}

inline int SgValue::ValueForPlayer(SgBlackWhite player) const
{
    SG_ASSERT_BW(player);
    return player == SG_WHITE ? -m_v : +m_v;
}

inline int SgValue::ValueForWhite() const
{
    return -m_v;
}

//----------------------------------------------------------------------------

class SgProbCut
{
public:
    static const int MAX_PROBCUT = 20;

    SgProbCut();

    struct Cutoff {
        float a, b, sigma;
        int shallow, deep;

        Cutoff() : shallow(-1), deep(-1) {}
    };

    void AddCutoff(const Cutoff &c);

    bool GetCutoff(int deep, int index, Cutoff &cutoff);

    void SetThreshold(float t);

    float GetThreshold() const;

    void SetEnabled(bool flag);

    bool IsEnabled() const;

private:
    float m_threshold;
    bool m_enabled;

    SgArray<SgArray<Cutoff, MAX_PROBCUT+1>, MAX_PROBCUT+1> m_cutoffs;
    SgArray<int, MAX_PROBCUT+1> m_cutoff_sizes;
};

inline SgProbCut::SgProbCut()
{
    m_threshold = 1.0;
    m_enabled = false;
    for (int i = 0; i < MAX_PROBCUT+1; ++i) m_cutoff_sizes[i] = 0;
}

inline void SgProbCut::AddCutoff(const Cutoff &c)
{
    int i = m_cutoff_sizes[c.deep];
    m_cutoffs[c.deep][i] = c;
    ++m_cutoff_sizes[c.deep];
}

inline bool SgProbCut::GetCutoff(int deep, int index, Cutoff &cutoff)
{
    if (deep > MAX_PROBCUT) return false;
    if (index >= m_cutoff_sizes[deep]) return false;
    cutoff = m_cutoffs[deep][index];
    return true;
}

inline void SgProbCut::SetThreshold(float t)
{
    m_threshold = t;
}

inline float SgProbCut::GetThreshold() const
{
    return m_threshold;
}

inline void SgProbCut::SetEnabled(bool flag)
{
    m_enabled = flag;
}

inline bool SgProbCut::IsEnabled() const
{
    return m_enabled;
}

//----------------------------------------------------------------------------

/** Used in class SgSearch to implement killer heuristic.
    Keeps track of two moves that have been successful at a particular level.
    The moves are sorted by frequency.
*/
class SgKiller
{
public:
    SgKiller();

    void MarkKiller(SgMove killer);

    void Clear();

    SgMove GetKiller1() const;

    SgMove GetKiller2() const;

private:
    SgMove m_killer1;

    SgMove m_killer2;

    int m_count1;

    int m_count2;
};

inline SgKiller::SgKiller()
    : m_killer1(SG_NULLMOVE),
      m_killer2(SG_NULLMOVE),
      m_count1(0),
      m_count2(0)
{
}

inline SgMove SgKiller::GetKiller1() const
{
    return m_killer1;
}

inline SgMove SgKiller::GetKiller2() const
{
    return m_killer2;
}

//----------------------------------------------------------------------------

/** Hash data used in class SgSearch. */
class SgSearchHashData
{
public:
    SgSearchHashData();

    SgSearchHashData(int depth, signed value, SgMove bestMove,
                     bool isOnlyUpperBound = false,
                     bool isOnlyLowerBound = false,
                     bool isExactValue = false);

    ~SgSearchHashData();

    int Depth() const;

    int Value() const;

    SgMove BestMove() const;

    bool IsOnlyUpperBound() const;

    bool IsOnlyLowerBound() const;

    void AdjustBounds(int* lower, int* upper);

    bool IsBetterThan(const SgSearchHashData& data) const;

    bool IsValid() const;

    bool IsExactValue() const;

    void Invalidate();

    void AgeData();

private:
    unsigned m_depth : 12;

    unsigned m_isUpperBound : 1;

    unsigned m_isLowerBound : 1;

    unsigned m_isValid : 1;

    unsigned m_isExactValue : 1;

    signed m_value : 16;

    SgMove m_bestMove;
};

typedef SgHashTable<SgSearchHashData> SgSearchHashTable;

inline SgSearchHashData::SgSearchHashData()
    : m_depth(0),
      m_isUpperBound(false),
      m_isLowerBound(false),
      m_isValid(false),
      m_isExactValue(false),
      m_value(-1),
      m_bestMove(SG_NULLMOVE)
{ }

inline SgSearchHashData::SgSearchHashData(int depth, signed value,
                                          SgMove bestMove,
                                          bool isOnlyUpperBound,
                                          bool isOnlyLowerBound,
                                          bool isExactValue)
    : m_depth(depth),
      m_isUpperBound(isOnlyUpperBound),
      m_isLowerBound(isOnlyLowerBound),
      m_isValid(true),
      m_isExactValue(isExactValue),
      m_value(value),
      m_bestMove(bestMove)
{
    // Ensure value fits in 16 bits
    SG_ASSERT(m_value == value);
}

inline SgSearchHashData::~SgSearchHashData()
{
}

inline int SgSearchHashData::Depth() const
{
    return static_cast<int> (m_depth);
}

inline int SgSearchHashData::Value() const
{
    return static_cast<int> (m_value);
}

inline SgMove SgSearchHashData::BestMove() const
{
    return m_bestMove;
}

inline bool SgSearchHashData::IsOnlyUpperBound() const
{
    return m_isUpperBound != 0;
}

inline bool SgSearchHashData::IsOnlyLowerBound() const
{
    return m_isLowerBound != 0;
}

inline void SgSearchHashData::AdjustBounds(int* lower, int* upper)
{
    if (IsOnlyUpperBound())
        *upper = std::min(*upper, Value());
    else if (IsOnlyLowerBound())
        *lower = std::max(*lower, Value());
    else
    {   // If not just an upper or lower bound, then this is precise.
        *lower = Value();
        *upper = Value();
    }
}

inline bool SgSearchHashData::IsValid() const
{
    return m_isValid;
}

inline bool SgSearchHashData::IsExactValue() const
{
    return m_isExactValue;
}

inline void SgSearchHashData::Invalidate()
{
    m_isValid = false;
}

inline void SgSearchHashData::AgeData()
{
    m_depth = 0;
}

//----------------------------------------------------------------------------

/** Alpha-beta search.
    The problem-specific part of the search is isolated in five methods:
    move generation, position evaluation, executing and taking back moves,
    and time control. These need to be overridden for each kind of search.
    The evaluation may employ lookahead or a quiescence search to
    find the value. If so, the sequence from the current position
    to the position where that value is really reached is returned,
    otherwise the empty list is returned.

    @todo Why does AmaSearch::Evaluate need the hash table, shouldn't that be
    done in SgSearch?
    @todo Implement construction of and navigation in trace tree here, not in
    subclasses.
    @todo Remove m_depth, pass as argument to Evaluate instead
    @todo Use best-response as move ordering heuristic
    @todo remove sequence parameter from Evaluate
*/
class SgSearch
{
public:
    static const int DEPTH_UNIT = 100;

    /** Infinity for windowed searches.
        Needs prefix SG_ even if not in global namespace, because there is a
        conflict with a global macro INFINITY.
    */
    static const int SG_INFINITY;

    /** Constructor.
        @param hash Hash table to use or 0, if no hash table should be used.
    */
    SgSearch(SgSearchHashTable* hash);

    virtual ~SgSearch();

    /** Add move property to node (game-dependent). */
    virtual void AddMoveProp(SgNode* node, SgMove move,
                             SgBlackWhite player) = 0;

    /** Stop search if depth limit was not reached in current iteration.
        Usually this should return true, but it depends on the move generation
        in the subclass. For example, if the move generation prunes some
        moves at lower depths, because the goal cannot be reached at the
        current depth, this function has to return false.
    */
    virtual bool CheckDepthLimitReached() const = 0;

    const SgSearchHashTable* HashTable() const;

    void SetHashTable(SgSearchHashTable* hashtable);

    const SgSearchControl* SearchControl() const;

    /** Search control.
        Set the abort checking function; pass 0 to disable abort checking.
        Caller keeps ownership of control.
    */
    void SetSearchControl(SgSearchControl* control);

    /** ProbCut.
        Set the ProbCut bounds; pass 0 to disable ProbCut.
        Caller keeps ownership of probcut.
    */
    void SetProbCut(SgProbCut* probcut);

    /** Convert move to string (game dependent). */
    virtual std::string MoveString(SgMove move) const = 0;

    virtual void SetToPlay(SgBlackWhite toPlay) = 0;

    /** Hook function called at the beginning of a search.
        Default implementation does nothing.
    */
    virtual void OnStartSearch();

    /** Looks 'depthLimit' moves ahead to find the value of the
        current position and the optimal sequence.
        No values outside the range ['boundLo'..'boundHi'] are expected;
        if values outside that range are encountered, a value <= 'boundLo'
        or >= 'boundHi' will be returned, and the search should be repeated
        with new bounds.
        If node is not 0, then add the whole search tree below '*node'.
        The hash table is seeded with the sequence passed in, so that partial
        results from a previous search can speed up a re-search.
    */
    int DepthFirstSearch(int depthLimit, int boundLo, int boundHi,
                         SgList<SgMove>* sequence, bool clearHash = true,
                         SgNode* traceNode = 0);

    /** Call DepthFirstSearch with window [-SG_INFINITY,+SG_INFINITY] */
    int DepthFirstSearch(int depthLimit, SgList<SgMove>* sequence,
                         bool clearHash = true, SgNode* traceNode = 0);

    /** Calls DepthFirstSearch repeatedly with the depth limit starting at
        'depthMin' and increasing with each iteration.
        Stops when the problem is solved, meaning that a result that is
        definitely good for one of the players is reached, or when 'depthMax'
        is reached. The bound parameters are just passed to DepthFirstSearch.
        If node is not 0, then add the whole search tree below '*node'.
        The hash table is seeded with the sequence passed in, so that partial
        results from a previous search can speed up a re-search.
    */
    int IteratedSearch(int depthMin, int depthMax, int boundLo,
                       int boundHi, SgList<SgMove>* sequence,
                       bool clearHash = true, SgNode* traceNode = 0);


    /** Call IteratedSearch with window [-SG_INFINITY,+SG_INFINITY] */
    int IteratedSearch(int depthMin, int depthMax, SgList<SgMove>* sequence,
                       bool clearHash = true, SgNode* traceNode = 0);

    /** During IteratedSearch or CombinedSearch, this returns the current
        depth that's being searched to.
    */
    int IteratedSearchDepthLimit() const;

    /** Called at start of each depth level of IteratedSearch.
        Can be overridden to adapt search (or instrumentation) to current
        depth. Must call inherited.
    */
    virtual void StartOfDepth(int depthLimit);

    /** Return whether the search was aborted. */
    bool Aborted() const;

    /** Mark this search as aborted.
        Will terminate next time AbortSearch gets called. Or mark search as
        not having been aborted (e.g. when minimizing area and first search
        succeeds but second one doesn't have enough time to complete).
    */
    void SetAbortSearch(bool fAborted = true);

    void SetScout(bool flag = true);

    void SetKillers(bool flag = true);

    void SetOpponentBest(bool flag = true);

    void SetNullMove(bool flag = true);

    void SetNullMoveDepth(int depth);

    void SetMustReturnExactResult(bool flag);

    /** Get the current statistics.
        Can be called during search.
        Override for derived search and statistics.
    */
    virtual void GetStatistics(SgSearchStatistics* stat);

    /** Add the statistics to the given statistics.
        Can only be called after the search.
        Override for derived search and statistics.
    */
    virtual void AddStatisticsTo(SgSearchStatistics* stat);

    /** Return the number of nodes since starting the search. */
    int NumNodes() const;

    /** Starts the clock and clears the statistics.
        Can be nested; only the outermost call actually does anything.
    */
    void StartTime();

    /** Stops the clock and clears the statistics.
        Can be nested; only the outermost call actually does anything.
    */
    void StopTime();

    /** 0 if not tracing */
    SgNode* TraceNode() const;

    /** Generate moves.
        @param moves The returned list is the set of moves to be tried.
        These moves will be tested for legality, so illegal moves can also be
        included if that speeds up move generation. If the empty list
        is returned, the evaluation function will be called with the
        same position right away (thus an evaluation done during move
        generation can be saved and returned from the move evaluation).
        @param depth The remaining depth until a terminal node is reached
        (height > 0). Note that this is a fractional value: each move may be
        counted as less than one full move to look deeper in some variations.
        The value is expressed in DEPTH_UNIT rather than using float to be
        stored compactly in the hash table.
    */
    virtual void Generate(SgList<SgMove>* moves, int depth) = 0;

    /** The returned value reflects the value of the position, with
        positive values being good for the current player (ToPlay).
        @param sequence Return value. Should contain the move (or move
        sequence) that leads to the evaluation, if the evaluation function
        does more than a static evaluation.
        @param isExact Return value, if set, the value is exact, even if it is
        not a terminal positions and Generate would generate moves.
        @param depth See SgSearch::Generate()
        @return The evaluation of the current position.
    */
    virtual int Evaluate(SgList<SgMove>* sequence, bool* isExact,
                         int depth) = 0;

    /** Return true if the move was executed, false if it was illegal
        and could not be played.
        @param move
        @param delta The amount by which that move shall decrement the current
        depth. Normal is DEPTH_UNIT, but forcing moves may return a delta of
        zero to look deeper into one variation.
        @param depth See SgSearch::Generate() (Execute could need to know the
        depth to reject moves depending on the depth that were originally
        generated, e.g. used in ExCaptureTask::Execute)
    */
    virtual bool Execute(SgMove move, int* delta, int depth) = 0;

    /** Takes back the most recent move successfully executed by Execute. */
    virtual void TakeBack() = 0;

    /** Return the current player. */
    virtual SgBlackWhite GetToPlay() const = 0;

    /** Test whether search should be aborted.
        Return true to abort search. Default implementation checks Abort() of
        the installed search control.
    */
    virtual bool AbortSearch();

    /** Return the hash code for the current position. */
    virtual SgHashCode GetHashCode() const = 0;

    /** The current depth of the search, incremented by 1 for each
        move that's played. Value is 0 at root level of search.
    */
    int CurrentDepth() const;

    /** The value of the previous level of search,
        during iterative deepening. */
    int PrevValue(SgList<SgMove>* sequence) const;

    /** Indicates which move in the movelist at the previous level was
        executed.
        This may be necessary if the value or moves at a
        position depend on the sequence leading to that position.
    */
    SgMove PrevMove() const;

    /** The move prior to the previous move.
        Knowing both the last two moves is necessary to decide whether the
        current position is a seki, where it's best for both players to pass.
    */
    SgMove PrevMove2() const;

    virtual bool EndOfGame() const = 0;

    /** Initialize PrevMove, CurrentDepth and other variables so that they can
        be accessed when move generation/evaluation called directly,
        not as part of a search.
    */
    void InitSearch(int startDepth = 0);

    void UpdateTime();

    /** 0 if not tracing */
    SgNode* m_traceNode;

    void TraceComment(const char* comment) const;

    void TraceValue(int value) const;

    void TraceValue(int value, const char* comment, bool isExact) const;

    /** Add the given move as a new node to the trace tree and go to that
        node.
        Don't do anything if m_traceNode is null. To be called from the
        client's Execute method.
    */
    void AddTraceNode(SgMove move, SgBlackWhite player);

    /** Go one move up in the trace tree.
        Don't do anything if m_traceNode is null.
        To be called from the client's TakeBack method.
    */
    void TakeBackTraceNode();

    virtual bool TraceIsOn() const;

    void InitTracing(const std::string& type);

    void AppendTrace(SgNode* toNode);

    void SetAbortFrequency(int value);

private:
    /** Hash table */
    SgSearchHashTable* m_hash;

    int m_currentDepth;

    int m_depthLimit;

    SgList<SgMove> m_moveStack;

    bool m_useScout;

    bool m_useKillers;

    /** Move best move from parent to front */
    bool m_useOpponentBest;

    /** Use null move heuristic for forward pruning */
    bool m_useNullMove;

    /** How much less deep to search during null move pruning */
    int m_nullMoveDepth;

    /** True if search is in the process of being aborted. */
    bool m_aborted;

    /** Flag that new best move was found in current iteration. */
    bool m_foundNewBest;

    /** Keeps track of whether the depth limit was reached. */
    bool m_reachedDepthLimit;

    bool m_mustReturnExactResult;

    SgSearchStatistics m_stat;

    SgTimer m_timer;

    int m_timerLevel;

    int m_prevValue;

    SgList<SgMove> m_prevSequence;

    static const int MAX_KILLER_DEPTH = 10;

    /** Killer heuristic. */
    SgArray<SgKiller,MAX_KILLER_DEPTH + 1> m_killers;

    SgSearchControl* m_control;

    SgProbCut* m_probcut;

    int m_abortFrequency;

    /** Depth-first search (see implementation) */
    int DFS(int startDepth, int depthLimit, int boundLo, int boundHi,
            SgList<SgMove>* sequence, bool* isExactValue);

    bool LookupHash(SgSearchHashData& data) const;

    void MoveKillersToFront(SgList<SgMove>& moves);

    /** Alpha-beta search */
    int SearchEngine(int depth, int alpha, int beta, SgList<SgMove>* sequence,
                     bool* isExactValue, bool lastNullMove = false);

    bool ProbCut(int depth, int alpha, int beta, SgList<SgMove>* sequence,
                 bool* isExactValue, int* value);

    bool NullMovePrune(int depth, int delta, int beta);

    void StoreHash(int depth, int value, SgMove move, bool isUpperBound,
                   bool isLowerBound, bool isExact);

    /** Seed the hash table with the given sequence. */
    void AddSequenceToHash(const SgList<SgMove>& sequence, int depth);

    int  CallEvaluate(int depth, int alpha, int beta,
                      SgList<SgMove>* sequence, bool* isExact);

    bool CallExecute(SgMove move, int* delta, int depth);

    void CallGenerate(SgList<SgMove>* moves, int depth);

    void CallTakeBack();

    /** Not implemented */
    SgSearch(const SgSearch&);

    /** Not implemented */
    SgSearch& operator=(const SgSearch&);
};

inline bool SgSearch::Aborted() const
{
    return m_aborted;
}

inline int SgSearch::CurrentDepth() const
{
    return m_currentDepth;
}

inline int SgSearch::DepthFirstSearch(int depthLimit,
                                      SgList<SgMove>* sequence,
                                      bool clearHash, SgNode* traceNode)
{
    return DepthFirstSearch(depthLimit, -SG_INFINITY, SG_INFINITY, sequence,
                            clearHash, traceNode);
}

inline const SgSearchHashTable* SgSearch::HashTable() const
{
    return m_hash;
}

inline void SgSearch::SetHashTable(SgSearchHashTable* hashtable)
{
    m_hash = hashtable;
}

inline int SgSearch::IteratedSearchDepthLimit() const
{
    return m_depthLimit;
}

inline int SgSearch::IteratedSearch(int depthMin, int depthMax,
                                    SgList<SgMove>* sequence, bool clearHash,
                                    SgNode* traceNode)
{
    return IteratedSearch(depthMin, depthMax, -SG_INFINITY, SG_INFINITY,
                          sequence, clearHash, traceNode);
}

inline int SgSearch::PrevValue(SgList<SgMove>* sequence) const
{
    if (sequence)
        *sequence = m_prevSequence;
    return m_prevValue;
}

inline SgMove SgSearch::PrevMove() const
{
    return m_moveStack[1];
}

inline SgMove SgSearch::PrevMove2() const
{
    return m_moveStack[2];
}

inline const SgSearchControl* SgSearch::SearchControl() const
{
    return m_control;
}

inline void SgSearch::SetAbortSearch(bool fAborted)
{
    m_aborted = fAborted;
}

inline void SgSearch::SetKillers(bool flag)
{
    m_useKillers = flag;
}

inline void SgSearch::SetOpponentBest(bool flag)
{
    m_useOpponentBest = flag;
}

inline void SgSearch::SetNullMove(bool flag)
{
    m_useNullMove = flag;
}

inline void SgSearch::SetNullMoveDepth(int depth)
{
    m_nullMoveDepth = depth;
}

inline void SgSearch::SetMustReturnExactResult(bool flag)
{
    m_mustReturnExactResult = flag;
}

inline void SgSearch::SetScout(bool flag)
{
    m_useScout = flag;
}

inline SgNode* SgSearch::TraceNode() const
{
    return m_traceNode;
}

inline void SgSearch::SetAbortFrequency(int value)
{
    m_abortFrequency = value;
}

//----------------------------------------------------------------------------

/** Resource control used in class SgSearch. */
class SgSearchControl
{
public:
    SgSearchControl();

    virtual ~SgSearchControl();

    /** Check if search should be aborted.
        Called at each node.
    */
    virtual bool Abort(double elapsedTime, int numNodes) = 0;

    /** Check if next iteration should be started.
        Called before each iteration.
        The default implementation always returns true.
        @param depth The depth of the next iteration.
        @param elapsedTime The elapsed time in seconds.
        @param numNodes The number of nodes visited.
    */
    virtual bool StartNextIteration(int depth, double elapsedTime,
                                    int numNodes);

private:
    /** Not implemented */
    SgSearchControl(const SgSearchControl&);

    /** Not implemented */
    SgSearchControl& operator=(const SgSearchControl&);
};

inline SgSearchControl::SgSearchControl()
{
}

//----------------------------------------------------------------------------

/** Example of a simple search abort class: abort when time has expired. */
class SgTimeSearchControl
    : public SgSearchControl
{
public:
    SgTimeSearchControl(double maxTime);

    virtual ~SgTimeSearchControl();

    virtual bool Abort(double elapsedTime, int numNodes);

    double GetMaxTime() const;

    void SetMaxTime(double maxTime);

private:
    double m_maxTime;

    /** Not implemented */
    SgTimeSearchControl(const SgTimeSearchControl&);

    /** Not implemented */
    SgTimeSearchControl& operator=(const SgTimeSearchControl&);
};

inline double SgTimeSearchControl::GetMaxTime() const
{
    return m_maxTime;
}

inline void SgTimeSearchControl::SetMaxTime(double maxTime)
{
    m_maxTime = maxTime;
}

//----------------------------------------------------------------------------

/** Example of a simple search abort class: abort when node limit
    is reached.
*/
class SgNodeSearchControl
    : public SgSearchControl
{
public:
    SgNodeSearchControl(int maxNumNodes);

    virtual ~SgNodeSearchControl();

    virtual bool Abort(double elapsedTime, int numNodes);

    void SetMaxNumNodes(int maxNumNodes);

private:
    int m_maxNumNodes;

    /** Not implemented */
    SgNodeSearchControl(const SgNodeSearchControl&);

    /** Not implemented */
    SgNodeSearchControl& operator=(const SgNodeSearchControl&);
};

inline void SgNodeSearchControl::SetMaxNumNodes(int maxNumNodes)
{
    m_maxNumNodes = maxNumNodes;
}

//----------------------------------------------------------------------------

/** Abort when either time or node limit is reached. */
class SgCombinedSearchControl
    : public SgSearchControl
{
public:
    SgCombinedSearchControl(double maxTime, int maxNumNodes);

    virtual ~SgCombinedSearchControl();

    virtual bool Abort(double elapsedTime, int numNodes);

private:
    double m_maxTime;

    int m_maxNumNodes;

    /** Not implemented */
    SgCombinedSearchControl(const SgCombinedSearchControl&);

    /** Not implemented */
    SgCombinedSearchControl& operator=(const SgCombinedSearchControl&);
};

inline SgCombinedSearchControl::SgCombinedSearchControl(double maxTime,
                                                        int maxNumNodes)
    : m_maxTime(maxTime),
      m_maxNumNodes(maxNumNodes)
{
}

//----------------------------------------------------------------------------

/** Abort when time limit is reached AND a number of nodes were searched. */
class SgRelaxedSearchControl
    : public SgSearchControl
{
public:
    static const int MIN_NODES_PER_SECOND = 1000;

    SgRelaxedSearchControl(double maxTime);

    virtual ~SgRelaxedSearchControl();

    virtual bool Abort(double elapsedTime, int numNodes);

private:
    double m_maxTime;

    /** Not implemented */
    SgRelaxedSearchControl(const SgRelaxedSearchControl&);

    /** Not implemented */
    SgRelaxedSearchControl& operator=(const SgRelaxedSearchControl&);
};

inline SgRelaxedSearchControl::SgRelaxedSearchControl(double maxTime)
    : m_maxTime(maxTime)
{
}

//----------------------------------------------------------------------------

const int MaxSearchDepth = 256;

/** The best possible search result - highest possible value.
    A win in n ply is encoded with a value of PosValue - n.
    
    A loss is encoded as -value if the win would be encoded as value.
    PosValue==31743. @todo: make them nicer numbers
*/
const int PosValue = MaxSearchDepth * 124 - 1;

/** The worst possible search result. 
    All search results are in the range [NegValue..PosValue].
*/
const int NegValue = -PosValue;

/** KoValue is the result for win by ko.
    Similarly with PosValue, a win by Ko in n ply is encoded as KoValue - n.
    KoValue==31488.
*/
const int KoValue = PosValue - (MaxSearchDepth - 1);

/** The maximum number of Ko recaptures allowed. */
const int MaxNuKo = 3;

/** SureValue is the lowest possible score that indicates a proven win.
    1. values in the range [NegValue..-SureValue] are proven losses 
       (possibly by ko).
    2. values in the range [-SureValue+1..SureValue-1] are heuristic scores.
    3. values in range [SureValue..PosValue] are proven wins (possibly by ko).
    SureValue==30719.
*/
const int SureValue = PosValue - (MaxNuKo + 1) * MaxSearchDepth;

//----------------------------------------------------------------------------

#endif // SG_SEARCH_H
