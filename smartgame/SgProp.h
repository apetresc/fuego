//----------------------------------------------------------------------------
/** @file SgProp.h
    Properties for nodes in a game tree.

    Module SgProp defines properties that are stored in each node of a game
    tree. Most properties correspond to items written in the SGF file format,
    but there are other properties that are hidden and only used by the
    system.
*/
//----------------------------------------------------------------------------

#ifndef SG_PROP_H
#define SG_PROP_H

#include <string>
#include <vector>
#include "SgBlackWhite.h"
#include "SgList.h"
#include "SgPoint.h"

class SgProp;

//----------------------------------------------------------------------------

/** The ID associated with this property */
typedef int SgPropID;

/** The flags describing this property */
typedef int SgPropFlags;

//----------------------------------------------------------------------------

/** Game dependent format for point values of SGF properties. */
enum SgPropPointFmt {
    /** Point format used in Go.
        Points are written as two letters. 'aa' is top left corner.
    */
    SG_PROPPOINTFMT_GO,

    /** Point format used in Hex and Reversi.
        Points are written as letter/number. 'a1' is top left corner.
        Both letters 'i' and 'j' are used ('i' is not skipped as in standard
        Go coordinates)
    */
    SG_PROPPOINTFMT_HEX
};

//----------------------------------------------------------------------------

namespace SgPropUtil
{
    std::string EscapeSpecialCharacters(const std::string& s,
                                        bool escapeColon);

    /** Return point format for a given game.
        Returns SG_PROPPOINTFMT_GO for unknown game numbers.
    */
    SgPropPointFmt GetPointFmt(int gameNumber);

    /** Convert point to SGF point string. */
    std::string PointToSgfString(SgMove p, int boardSize,
                                 SgPropPointFmt fmt, int fileFormat = 4);

    /** Convert SGF point string to point.
        @return The point or SG_PASS (only allowed if point format is
        SG_PROPPOINTFMT_GO) or SG_NULLMOVE, if s is not a valid point.
     */
    SgPoint SgfStringToPoint(const std::string& s, int boardSize,
                             SgPropPointFmt fmt);
}

//----------------------------------------------------------------------------

/** Maximum number of property classes defined */
const int MAX_PROP_CLASS = 150;

/** Contains information related to Black player */
const int fBlackProp = 1 << 0;

/** Contains information related to White player */
const int fWhiteProp = 1 << 1;

/** Game info */
const int fInfoProp = 1 << 2;

/** Annotation */
const int fAnnoProp = 1 << 3;

/** Statistics generated by the program */
const int fStatProp = 1 << 4;

/** Property can only be stored in root */
const int fRootProp = 1 << 5;

/** Move annotation */
const int fMoveAnno = 1 << 6;

/** Position annotation */
const int fPosAnno = 1 << 7;

/** Black or white move */
const int fMoveProp = 1 << 8;

/** Marks on board points */
const int fMarkProp = 1 << 9;

/** Time left info */
const int fTimeProp = 1 << 10;

/** Abstract property */
const int fAbstract = 1 << 11;

/** Property is not part of the FF[3] standard */
const int fNotFF3 = 1 << 12;

/** Property is not part of the FF[4] standard */
const int fNotFF4 = 1 << 13;

/** Custom Smart Go property */
const int fCustom = 1 << 14;

/** Don't write prop when publishing clean file */
const int fNotClean = 1 << 15;

/** Write out this property starting on a new line */
const int fNewLine = 1 << 16;

//----------------------------------------------------------------------------

/** Property list.
    Implemented as list of pointers to objects derived from Property.
*/
class SgPropList
{
public:
    SgPropList();

    ~SgPropList();

    /** Return whether this list contains zero elements. */
    bool IsEmpty() const;

    /** Remove all elements in this list, disposing each property. */
    void Clear();

    /** Return the first property in the list that matches the given ID.
        Note that SG_PROP_INFO, SG_PROP_ANNOTATE, SG_PROP_POS_ANNO,
        SG_PROP_MOVE_ANNO, and SG_PROP_COUNT match any property that has the
        corresponding flag set.
        @return 0 if there is no such property.
    */
    SgProp* Get(SgPropID id) const;

    /** Return the first property in the list that matches the given text. */
    SgProp* GetPropContainingText(const std::string& findText) const;

    /** Add the property to this property list.
        Enforces that no two properties of the same kind are added.
    */
    void Add(const SgProp* prop);

    /** If the property with the given ID exists, move it to the front of this
        property list.
    */
    void MoveToFront(SgPropID id);

    /** Remove the property from the property list.
        Return true if the property was in the list.
    */
    bool Remove(const SgProp* prop);

    /** Remove any properties that match 'id' from this list, and
        dispose them, except don't touch *protectProp if it's in
        the list.
    */
    void Remove(SgPropID id, const SgProp* protectProp);

    void RemoveProp(SgPropID id) { Remove(id, 0); }

    /** Add the move annotations at the end of '*string'.
        !! very good move SG_PROP_GOOD_MOVE[2] <br>
        !  good move SG_PROP_GOOD_MOVE[1] <br>
        !? interesting move SG_PROP_INTERESTING <br>
        ?! doubtful move SG_PROP_DOUBTFUL <br>
        ?  bad move SG_PROP_BAD_MOVE[1] <br>
        ?? very bad move SG_PROP_BAD_MOVE[2] <br>
        Return true if a move annotation was added.
    */
    bool AppendMoveAnnotation(std::string* s) const;

private:
    friend class SgPropListIterator;

    /** property list implemented as list of properties */
    SgListOf<SgProp> m_list;

    /** not implemented */
    SgPropList(const SgPropList&);

    /** not implemented */
    SgPropList& operator=(const SgPropList&);
};

inline bool SgPropList::IsEmpty() const
{
    return m_list.IsEmpty();
}

//----------------------------------------------------------------------------

/** Iterate through Properties in a PropList */
class SgPropListIterator
{
public:
    /** Create a list iterator to iterate through list. */
    SgPropListIterator(const SgPropList& propList);

    void operator++();

    SgProp* operator*() const;

    operator bool() const;

private:
    SgListIteratorOf<SgProp> m_listIterator;

    /** Not implemented */
    SgPropListIterator(const SgPropListIterator&);

    /** Not implemented */
    SgPropListIterator& operator=(const SgPropListIterator&);
};

inline SgPropListIterator::SgPropListIterator(const SgPropList& propList)
    : m_listIterator(propList.m_list)
{
}

inline void SgPropListIterator::operator++()
{
    m_listIterator.operator++();
}

inline SgProp* SgPropListIterator::operator*() const
{
    return m_listIterator.operator*();
}

inline SgPropListIterator::operator bool() const
{
    return m_listIterator.operator bool();
}

//----------------------------------------------------------------------------

/** Property base class */
class SgProp
{
public:
    explicit SgProp(SgPropID id);

    virtual ~SgProp();

    /** Override this function for each property class to return an exact
        duplicate of this property.
    */
    virtual SgProp* Duplicate() const = 0;

    /** Return the property type of this property. */
    SgPropID ID() const;

    /** Get the flags for this property type.
        Not normally overridden.
    */
    virtual SgPropFlags Flags() const;

    /** Get the the label for this property type.
        Overridden only by SgPropUnknown.
    */
    virtual std::string Label() const;

    /** Return whether any of the given flags are set for this property. */
    bool Flag(SgPropFlags flags) const;

    /** Convert the property into string representation.
        Escapes special characters if needed (this depends on the property,
        e.g. the colon needs to be escaped only by some properties)
        Use the default file format if 'fileFormat' is
        zero; use the proper version of the sgf file format if 'fileFormat'
        is 3 or greater.
        @return true if the property should be written to file
    */
    virtual bool ToString(std::vector<std::string>& values, int boardSize,
                          SgPropPointFmt fmt, int fileFormat) const = 0;

    /** Override this method to convert the string read from disk to the value
        of this property, and set the value of this property.
        @return true, if the string could be converted to a valid property.
    */
    virtual bool FromString(const std::vector<std::string>& values,
                            int boardSize, SgPropPointFmt fmt) = 0;

    /** Call this function with a property of the right type to register that
        property type.
        Ownership of 'prop' is passed to the Property class;
        it will dispose the property upon exit.
        Register returns the property identifier to be used to refer to this
        property.
        Abstract properties can be registered with 'prop' set to 0.
        No property object of that type can be instantiated.
        (?? useful for searching?)
        This method asserts and returns 0 if the registry is full.
    */
    static SgPropID Register(SgProp* prop, const char* label,
                             SgPropFlags flags = 0);

    /** Create a property with the given property ID. */
    static SgProp* CreateProperty(SgPropID id);

    /** Return the ID for a given label.
        Return SG_PROP_NONE if there is no property with that label.
    */
    static SgPropID GetIDOfLabel(const std::string& label);

    /** Convert the text specified in the Find dialog to special propIDs to
        search for.
        Return SG_PROP_NONE if the literal text should be searched for.
    */
    static SgPropID ConvertFindTextToPropID(const std::string& findText);

    /** Initialize properties.
        Registers most properties. Does not register SG_PROP_MOVE_BLACK ("B")
        and SG_PROP_MOVE_WHITE ("W"), because they are game dependent.
    */
    static void Init();

    /** Finalize properties. */
    static void Fini();

    /** If this property is marked as either fBlackProp or fWhiteProp,
        return that property.
        Otherwise the return value is undefined (checked with assertion).
    */
    SgBlackWhite Player() const;

    bool IsPlayer(SgBlackWhite player) const;

    /** If the given property is marked as either fBlackProp or fWhiteProp,
        return the property of the opposite color, otherwise return 'id'.
    */
    static SgPropID OpponentProp(SgPropID id);

    /** If the given property is marked as either fBlackProp or fWhiteProp,
        return the property of player's color, otherwise return 'id'.
    */
    static SgPropID PlayerProp(SgPropID id, SgBlackWhite player);

    /** Override this method to do something special when changing the color
        of a property (e.g. a value might need to be negated).
    */
    virtual void ChangeToOpponent();

    /** Return true if the given 'id' matches this property.
        The special properties SG_PROP_INFO, SG_PROP_ANNOTATE,
        SG_PROP_POS_ANNO, SG_PROP_MOVE_ANNO, and SG_PROP_COUNT match any
        property that has the corresponding flag set.
    */
    bool MatchesID(SgPropID id) const;

    /** Return true if this property matches the given text.
        Override for specific properties.
    */
    virtual bool ContainsText(const std::string& findText);

protected:
    SgPropID m_id;

    static bool Initialized();

private:
    /** Was SgProp::Init() called? */
    static bool s_initialized;

    static int s_numPropClasses;

    static SgPropFlags s_flags[MAX_PROP_CLASS];

    static std::string s_label[MAX_PROP_CLASS];

    static SgProp* s_prop[MAX_PROP_CLASS];

    /** not implemented */
    SgProp(const SgProp&);

    /** not implemented */
    SgProp& operator=(const SgProp&);
};

inline SgProp::SgProp(SgPropID id)
    : m_id(id)
{
}

inline SgPropID SgProp::ID() const
{
    return m_id;
}

inline bool SgProp::Flag(SgPropFlags flags) const
{
    return (Flags() & flags) != 0;
}

//----------------------------------------------------------------------------

/** Unknown property.
    Unknown properties are used to store properties read from file but not
    understood by this version. This property keeps the label and the string
    that were read in, so that it can be written out again in exactly the same
    way.
*/
class SgPropUnknown
    : public SgProp
{
public:
    explicit SgPropUnknown(SgPropID id);

    SgPropUnknown(SgPropID id, std::string label,
                  const std::vector<std::string>& values);

    SgProp* Duplicate() const;

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

    std::string Label() const;

private:
    std::string m_label;

    std::vector<std::string> m_values;
};

inline SgPropUnknown::SgPropUnknown(SgPropID id)
    : SgProp(id)
{
}

inline SgPropUnknown::SgPropUnknown(SgPropID id, std::string label,
                                    const std::vector<std::string>& values)
    : SgProp(id),
      m_label(label),
      m_values(values)
{
}

inline std::string SgPropUnknown::Label() const
{
    return m_label;
}

//----------------------------------------------------------------------------

/** A property with integer value. */
class SgPropInt
    : public SgProp
{
public:
    explicit SgPropInt(SgPropID id);

    SgPropInt(SgPropID id, int value);

    SgProp* Duplicate() const;

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

    /** Return the integer value of this property. */
    int Value() const;

    bool IsValue(int value) const;

    /** Set the integer value of this property. */
    void SetValue(int value);

protected:
    int m_value;
};

inline SgPropInt::SgPropInt(SgPropID id)
    : SgProp(id),
      m_value(0)
{
}

inline SgPropInt::SgPropInt(SgPropID id, int value)
    : SgProp(id),
      m_value(value)
{
}

inline int SgPropInt::Value() const
{
    SG_ASSERT(Initialized());
    return m_value;
}

inline bool SgPropInt::IsValue(int value) const
{
    return m_value == value;
}

inline void SgPropInt::SetValue(int value)
{
    m_value = value;
}

//----------------------------------------------------------------------------

/** A property with a double value. Optionally can specify precision, too. */
class SgPropReal
    : public SgProp
{
public:
    explicit SgPropReal(SgPropID id);

    /** Create property with double value and given precision.
        @param id Property ID
        @param value Value
        @param precision Precision after dot.
        0 means default precision.
    */
    SgPropReal(SgPropID id, double value, int precision = 0);

    SgProp* Duplicate() const;

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

    double Value() const;

    void SetValue(double value);

protected:
    int m_precision;

    double m_value;
};

inline SgPropReal::SgPropReal(SgPropID id)
    : SgProp(id),
      m_precision(0),
      m_value(0)
{
}

inline SgPropReal::SgPropReal(SgPropID id, double value, int precision)
    : SgProp(id),
      m_precision(precision),
      m_value(value)
{
}

inline double SgPropReal::Value() const
{
    return m_value;
}

inline void SgPropReal::SetValue(double value)
{
    m_value = value;
}

//----------------------------------------------------------------------------

/** A property with no associated value.
    Works as a flag (property present/absent).
*/
class SgPropSimple
    : public SgProp
{
public:
    explicit SgPropSimple(SgPropID id);

    SgProp* Duplicate() const;

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);
};

inline SgPropSimple::SgPropSimple(SgPropID id)
    : SgProp(id)
{
}

//----------------------------------------------------------------------------

/** Multiple property.
    @todo AR: Make sure it's in range[1..2]. Should be deleted if it's 0.
    @todo AR: could do away with this after all, set flag instead?
*/
class SgPropMultiple
    : public SgPropInt
{
public:
    explicit SgPropMultiple(SgPropID id);

    SgPropMultiple(SgPropID id, int value);

    SgProp* Duplicate() const;
};

inline SgPropMultiple::SgPropMultiple(SgPropID id)
    : SgPropInt(id, 1)
{
}

inline SgPropMultiple::SgPropMultiple(SgPropID id, int value)
    : SgPropInt(id, value)
{
}

//----------------------------------------------------------------------------

/** Like SgPropInt but can change sign for opponent's value. */
class SgPropValue
    : public SgPropInt
{
public:
    explicit SgPropValue(SgPropID id);

    SgPropValue(SgPropID id, int value);

    SgProp* Duplicate() const;

    virtual void ChangeToOpponent();
};

inline SgPropValue::SgPropValue(SgPropID id)
    : SgPropInt(id)
{
}

inline SgPropValue::SgPropValue(SgPropID id, int value)
    : SgPropInt(id, value)
{
}

//----------------------------------------------------------------------------

/** A property with time value. */
class SgPropTime
    : public SgPropReal
{
public:
    SgPropTime(SgPropID id, double value = 0, int precision = 1);

    virtual ~SgPropTime();

    SgProp* Duplicate() const;
};

inline SgPropTime::SgPropTime(SgPropID id, double value, int precision)
    : SgPropReal(id, value, precision)
{
}

//----------------------------------------------------------------------------

/** Like SgPropTime, but gets stored with millisecond precision rather than
    tenths of a second.
*/
class SgPropMSec
    : public SgPropTime
{
public:
    explicit SgPropMSec(SgPropID id);

    SgPropMSec(SgPropID id, double value);

    virtual ~SgPropMSec();

    SgProp* Duplicate() const;
};

inline SgPropMSec::SgPropMSec(SgPropID id)
    : SgPropTime(id, 0, 3)
{
}

inline SgPropMSec::SgPropMSec(SgPropID id, double value)
    : SgPropTime(id, value, 3)
{
}

//----------------------------------------------------------------------------

/** A property storing a point-move for games.
    Only for games in which a move can be described by a point.
*/
class SgPropMove
    : public SgProp
{
public:
    explicit SgPropMove(SgPropID id);

    SgPropMove(SgPropID id, SgMove move);

    SgProp* Duplicate() const;

    /** Return the move value of this property. */
    SgPoint Value() const;

    bool IsValue(SgPoint move) const;

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

protected:
    SgPoint m_move;
};

inline SgPropMove::SgPropMove(SgPropID id)
    : SgProp(id),
      m_move(SG_NULLMOVE)
{
}

inline SgPropMove::SgPropMove(SgPropID id, SgMove move)
    : SgProp(id),
      m_move(move)
{
}

inline SgPoint SgPropMove::Value() const
{
    return m_move;
}

inline bool SgPropMove::IsValue(SgPoint move) const
{
    return m_move == move;
}

//----------------------------------------------------------------------------

/** Void pointer list property.
    @deprecated Using void pointers is generally discouraged in C++.
    This functionality is still used in lo/LoMath.
*/
class SgPropVoidList
    : public SgProp
{
public:
    explicit SgPropVoidList(SgPropID id);

    SgPropVoidList(SgPropID id, const SgList<void*>& list);

    virtual ~SgPropVoidList();

    SgProp* Duplicate() const;

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

    /** Return the list value of this property. */
    const SgList<void*>& Value() const;

    SgList<void*>& Value();

    /** Set the list value of this property. */
    void SetValue(const SgList<void*>& list);

    void Append(void* elt);

private:
    SgList<void*> m_list;
};

inline SgPropVoidList::SgPropVoidList(SgPropID id)
    : SgProp(id),
      m_list()
{
}

inline SgPropVoidList::SgPropVoidList(SgPropID id, const SgList<void*>& list)
    : SgProp(id),
      m_list(list)
{
}

inline const SgList<void*>& SgPropVoidList::Value() const
{
    return m_list;
}

inline SgList<void*>& SgPropVoidList::Value()
{
    return m_list;
}

inline void SgPropVoidList::SetValue(const SgList<void*>& list)
{
    m_list = list;
}

inline void SgPropVoidList::Append(void* elt)
{
    m_list.Append(elt);
}

//----------------------------------------------------------------------------

/** A property storing a list of points. */
class SgPropPointList
    : public SgProp
{
public:
    explicit SgPropPointList(SgPropID id);

    SgPropPointList(SgPropID id, const SgList<SgPoint>& list);

    virtual ~SgPropPointList();

    SgProp* Duplicate() const;

    /** Return the list value of this property. */
    const SgList<SgPoint>& Value() const;

    SgList<SgPoint>& Value();

    /** Set the list value of this property. */
    void SetValue(const SgList<SgPoint>& list);

    void Append(SgPoint p);

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

private:
    SgList<SgPoint> m_list;
};

inline SgPropPointList::SgPropPointList(SgPropID id)
    : SgProp(id)
{
}

inline SgPropPointList::SgPropPointList(SgPropID id,
                                        const SgList<SgPoint>& list)
    : SgProp(id),
      m_list(list)
{
}

inline const SgList<SgPoint>& SgPropPointList::Value() const
{
    return m_list;
}

inline SgList<SgPoint>& SgPropPointList::Value()
{
    return m_list;
}

inline void SgPropPointList::SetValue(const SgList<SgPoint>& list)
{
    m_list = list;
}

inline void SgPropPointList::Append(SgPoint p)
{
    m_list.Append(p);
}

//----------------------------------------------------------------------------

/** A property storing a text string. */
class SgPropText
    : public SgProp
{
public:
    explicit SgPropText(SgPropID id);

    SgPropText(SgPropID id, const std::string& text);

    SgProp* Duplicate() const;

    /** Return the string value of this property. */
    const std::string& Value() const;

    std::string& Value();

    void SetValue(const std::string& value);

    void AppendText(const std::string& text);

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

    virtual bool ContainsText(const std::string& findText);

private:
    std::string m_text;
};

inline SgPropText::SgPropText(SgPropID id)
    : SgProp(id),
      m_text()
{
}

inline SgPropText::SgPropText(SgPropID id, const std::string& text)
    : SgProp(id),
      m_text(text)
{
}

inline const std::string& SgPropText::Value() const
{
    return m_text;
}

inline std::string& SgPropText::Value()
{
    return m_text;
}

inline void SgPropText::SetValue(const std::string& value)
{
    m_text = value;
}

inline void SgPropText::AppendText(const std::string& text)
{
    m_text += text;
}

//----------------------------------------------------------------------------

/** Keeps a string for each point in a set of points. */
class SgPropTextList
    : public SgProp
{
public:
    explicit SgPropTextList(SgPropID id);

    SgPropTextList(SgPropID id, const SgList<SgPoint>& points,
                   SgListOf<std::string> strings);

    virtual ~SgPropTextList();

    SgProp* Duplicate() const;

    /** Return a list with all the points that have text associated with
        them.
    */
    const SgList<SgPoint>& GetPointsWithText() const;

    /** If point 'p' has a string, copy that string into '*string' and return
        true.
        Otherwise return false and don't change '*string'.
    */
    bool GetStringAtPoint(SgPoint p, std::string* s) const;

    /** Set the string for point 'p' to 'string'.
        If that point already has a string, replace it with the new string.
    */
    void AddStringAtPoint(SgPoint p, const std::string& s);

    /** Append 'string' to the string for point 'p'.
        If that point has no string, create a new one.
    */
    void AppendToStringAtPoint(SgPoint p, const std::string& s);

    /** Remove any existing string for point 'p'. */
    void ClearStringAtPoint(SgPoint p);

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

    virtual bool ContainsText(const std::string& findText);

private:
    SgList<SgPoint> m_points;

    SgListOf<std::string> m_strings;
};

inline SgPropTextList::SgPropTextList(SgPropID id)
    : SgProp(id),
      m_points(),
      m_strings()
{
}

inline const SgList<SgPoint>& SgPropTextList::GetPointsWithText() const
{
    return m_points;
}

//----------------------------------------------------------------------------

/** A property storing a player color (Black or White). */
class SgPropPlayer
    : public SgProp
{
public:
    explicit SgPropPlayer(SgPropID id);

    SgPropPlayer(SgPropID id, int player);

    SgProp* Duplicate() const;

    SgBlackWhite Value() const;

    void SetValue(SgBlackWhite player);

    virtual void ChangeToOpponent();

    bool ToString(std::vector<std::string>& values, int boardSize,
                  SgPropPointFmt fmt, int fileFormat) const;

    bool FromString(const std::vector<std::string>& values,
                    int boardSize, SgPropPointFmt fmt);

private:
    SgBlackWhite m_player;
};

inline SgPropPlayer::SgPropPlayer(SgPropID id)
    : SgProp(id),
      m_player(SG_BLACK)
{
}

inline SgPropPlayer::SgPropPlayer(SgPropID id, int player)
    : SgProp(id),
      m_player(player)
{
}

inline SgBlackWhite SgPropPlayer::Value() const
{
    return m_player;
}

inline void SgPropPlayer::SetValue(SgBlackWhite player)
{
    m_player = player;
}

//----------------------------------------------------------------------------

/** A property storing a list of stones to add to the board,
    or points to make empty.
*/
class SgPropAddStone
    : public SgPropPointList
{
public:
    explicit SgPropAddStone(SgPropID id);

    SgPropAddStone(SgPropID id, const SgList<SgPoint>& list);

    virtual ~SgPropAddStone();

    SgProp* Duplicate() const;
};

inline SgPropAddStone::SgPropAddStone(SgPropID id)
    : SgPropPointList(id)
{
}

inline SgPropAddStone::SgPropAddStone(SgPropID id,
                                      const SgList<SgPoint>& list)
    : SgPropPointList(id, list)
{
}

//----------------------------------------------------------------------------

/** @name General */
//@{

/** Default value returned by some functions */
extern SgPropID SG_PROP_NONE;

/** Unknown property read from disk */
extern SgPropID SG_PROP_UNKNOWN;

//@}

/** @name Moves */
//@{

/** Generic property for black and white move */
extern SgPropID SG_PROP_MOVE;

/** Black move */
extern SgPropID SG_PROP_MOVE_BLACK;

/** White move */
extern SgPropID SG_PROP_MOVE_WHITE;

//@}

/** @name Board edits */
//@{

/** Add a list of black stones to the current position */
extern SgPropID SG_PROP_ADD_BLACK;

/** Add a list of white stones to the current position */
extern SgPropID SG_PROP_ADD_WHITE;

/** Remove stones from the board */
extern SgPropID SG_PROP_ADD_EMPTY;

/** Whose turn it is to move after executing the node */
extern SgPropID SG_PROP_PLAYER;


/** @name Value and territory */
//@{

/** Node value expressed as positive = good for Black */
extern SgPropID SG_PROP_VALUE;

/** Black surrounded territory and dead white stones */
extern SgPropID SG_PROP_TERR_BLACK;

/** White surrounded territory and dead black stones */
extern SgPropID SG_PROP_TERR_WHITE;

//@}

/** @name Marks drawn on the board */
//@{

/** Generic property for marked board points */
extern SgPropID SG_PROP_MARKS;

/** Selected points used to temporarily mark points */
extern SgPropID SG_PROP_SELECT;

/** Crosses displayed on stones and empty points */
extern SgPropID SG_PROP_MARKED;

/** Triangles displayed on stones and empty points */
extern SgPropID SG_PROP_TRIANGLE;

/** Small squares or square stones */
extern SgPropID SG_PROP_SQUARE;

/** Diamond marks */
extern SgPropID SG_PROP_DIAMOND;

/** Board points marked with small grey circles */
extern SgPropID SG_PROP_CIRCLE;

/** Dimmed points */
extern SgPropID SG_PROP_DIMMED;

/** Sequence of pairs: point, four letter label */
extern SgPropID SG_PROP_LABEL;

//@}

/** @name Time control */
//@{

/** Generic property for time left information */
extern SgPropID SG_PROP_TIMES;

/** Time left for the black player (TimeProp) */
extern SgPropID SG_PROP_TIME_BLACK;

/** Time left for the white player (TimeProp) */
extern SgPropID SG_PROP_TIME_WHITE;

/** Number of stones to play for black in this overtime period */
extern SgPropID SG_PROP_OT_BLACK;

/** Number of stones to play for white in this overtime period */
extern SgPropID SG_PROP_OT_WHITE;

/** Number of moves per overtime period (0 = no overtime) */
extern SgPropID SG_PROP_OT_NU_MOVES;

/** Length of each overtime period */
extern SgPropID SG_PROP_OT_PERIOD;

/** Seconds of operator overhead for each move */
extern SgPropID SG_PROP_OVERHEAD;

/** Added to root node if losing on time is enforced */
extern SgPropID SG_PROP_LOSE_TIME;

//@}

/** @name Statistics */
//@{

/** Generic property subsuming all in statistics category */
extern SgPropID SG_PROP_COUNT;

/** The time used to solve a problem */
extern SgPropID SG_PROP_TIME_USED;

/** The number of nodes looked at to solve a problem */
extern SgPropID SG_PROP_NUM_NODES;

/** The number of leaf nodes inspected */
extern SgPropID SG_PROP_NUM_LEAFS;

/** The maximal depth reached during the search */
extern SgPropID SG_PROP_MAX_DEPTH;

/** The number of plies searched */
extern SgPropID SG_PROP_DEPTH;

/** The number of top level moves at deepest search */
extern SgPropID SG_PROP_PART_DEPTH;

/** A value computed for a position */
extern SgPropID SG_PROP_EVAL;

/** The move expected from the current player */
extern SgPropID SG_PROP_EXPECTED;

/** Moves tried at a node in self-test mode */
extern SgPropID SG_PROP_SELF_TEST;

//@}

/** @name Root props */
//@{

/** The file format used to store the game */
extern SgPropID SG_PROP_FORMAT;

/** The board size */
extern SgPropID SG_PROP_SIZE;

/** The game (encoding see SgTypes.TheGame) */
extern SgPropID SG_PROP_GAME;

/** Species of the black player (human, computer, modem) */
extern SgPropID SG_PROP_SPEC_BLACK;

/** Species of the white player (human, computer, modem) */
extern SgPropID SG_PROP_SPEC_WHITE;

/** Number of Chinese handicap stones */
extern SgPropID SG_PROP_CHINESE;

/** The application that wrote this file */
extern SgPropID SG_PROP_APPLIC;

//@}

/** @name Annotations */
//@{

/** Generic property subsuming all annotation properties */
extern SgPropID SG_PROP_ANNOTATE;

/** The textual comment of a node */
extern SgPropID SG_PROP_COMMENT;

/** Short textual comment */
extern SgPropID SG_PROP_NAME;

/** Position marked with a check mark */
extern SgPropID SG_PROP_CHECK;

/** Position marked with a sigma icon */
extern SgPropID SG_PROP_SIGMA;

/** General position mark */
extern SgPropID SG_PROP_HOTSPOT;

/** Divides the game into sections to be printed */
extern SgPropID SG_PROP_FIGURE;

//@}

/** @name Position annotations */
//@{

/** Generic property subsuming all position annotations */
extern SgPropID SG_PROP_POS_ANNO;

/** Good position for Black */
extern SgPropID SG_PROP_GOOD_BLACK;

/** Good position for White */
extern SgPropID SG_PROP_GOOD_WHITE;

/** Even position */
extern SgPropID SG_PROP_EVEN_POS;

/** Unclear position */
extern SgPropID SG_PROP_UNCLEAR;

//@}

/** @name Move annotations */
//@{

/** Generic property subsuming all move annotations */
extern SgPropID SG_PROP_MOVE_ANNO;

/** Denotes an exceptionally good move (! or !!) */
extern SgPropID SG_PROP_GOOD_MOVE;

/** Denotes a bad move (? or ??) */
extern SgPropID SG_PROP_BAD_MOVE;

/** Denotes an interesting move (!?) */
extern SgPropID SG_PROP_INTERESTING;

/** Denotes a doubtful move (?!) */
extern SgPropID SG_PROP_DOUBTFUL;

//@}

/** @name Game info */
//@{

/** Generic property subsuming all game info props */
extern SgPropID SG_PROP_INFO;

/** The file name of the game */
extern SgPropID SG_PROP_GAME_NAME;

/** Comment pertaining to the whole game */
extern SgPropID SG_PROP_GAME_COMMENT;

/** Text describing the event */
extern SgPropID SG_PROP_EVENT;

/** The round of the tournament */
extern SgPropID SG_PROP_ROUND;

/** The date when the game was played */
extern SgPropID SG_PROP_DATE;

/** Where the game was played */
extern SgPropID SG_PROP_PLACE;

/** Name of the black player */
extern SgPropID SG_PROP_PLAYER_BLACK;

/** Name of the white player */
extern SgPropID SG_PROP_PLAYER_WHITE;

/** Who won the game */
extern SgPropID SG_PROP_RESULT;

/** The person who entered the game and comments */
extern SgPropID SG_PROP_USER;

/** The time allocated to each player */
extern SgPropID SG_PROP_TIME;

/** Where the game was copied from */
extern SgPropID SG_PROP_SOURCE;

/** Who has the copyright on the material */
extern SgPropID SG_PROP_COPYRIGHT;

/** Who analyzed the game */
extern SgPropID SG_PROP_ANALYSIS;

/** Ranking of black player */
extern SgPropID SG_PROP_RANK_BLACK;

/** Ranking of white player */
extern SgPropID SG_PROP_RANK_WHITE;

/** Team of black player */
extern SgPropID SG_PROP_TEAM_BLACK;

/** Team of white player */
extern SgPropID SG_PROP_TEAM_WHITE;

/** The opening played in the game */
extern SgPropID SG_PROP_OPENING;

/** Special rules (Go: Japanese or Chinese) */
extern SgPropID SG_PROP_RULES;

/** Number of handicap stones */
extern SgPropID SG_PROP_HANDICAP;

/** Komi value */
extern SgPropID SG_PROP_KOMI;

//@}

/** @name Abstract properties */
//@{

/** Indicates that depth-first traversal searches text */
extern SgPropID SG_PROP_FIND_MOVE;

/** Indicates that depth-first traversal searches move */
extern SgPropID SG_PROP_FIND_TEXT;

/** Used to search for branch points */
extern SgPropID SG_PROP_BRANCH;

/** A terminal node, only used for display */
extern SgPropID SG_PROP_TERMINAL;

//@}

/** @name Smart Go specific properties */
//@{

/** Move motive: why move was generated */
extern SgPropID SG_PROP_MOTIVE;

/** Sequence of moves expected by computer player */
extern SgPropID SG_PROP_SEQUENCE;

/** Constraint: points must not be empty */
extern SgPropID SG_PROP_NOT_EMPTY;

/** Constraint: points must not be black */
extern SgPropID SG_PROP_NOT_BLACK;

/** Constraint: points must not be white */
extern SgPropID SG_PROP_NOT_WHITE;

//@}

//----------------------------------------------------------------------------

#endif // SG_PROP_H
