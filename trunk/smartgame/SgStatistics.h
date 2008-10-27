//----------------------------------------------------------------------------
/** @file SgStatistics.h
    Classes for computing mean, variances.
    Note that the classes SgStatisticsBase, SgStatistics, and SgStatisticsExt
    derive from each other for convenience of implementation only, they don't
    use virtual functions for efficiency and are not meant to be used
    polymorphically.
*/
//----------------------------------------------------------------------------

#ifndef SG_STATISTICS_H
#define SG_STATISTICS_H

#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "SgException.h"
#include "SgWrite.h"

//----------------------------------------------------------------------------

/** Computes mean of a statistical variable.
    The template parameters are the floating point type and the counter type,
    depending on the precision-memory tradeoff.
*/
template<typename VALUE, typename COUNT>
class SgStatisticsBase
{
public:
    SgStatisticsBase();

    /** Create statistics initialized with values.
        Equivalent to creating a statistics and calling @c count times
        Add(val)
    */
    SgStatisticsBase(VALUE val, COUNT count);

    void Add(VALUE val);

    void Clear();

    const COUNT& Count() const;

    /** Initialize with values.
        Equivalent to calling Clear() and calling @c count times
        Add(val)
    */
    void Initialize(VALUE val, COUNT count);

    const VALUE& Mean() const;

    /** Write in human readable format. */
    void Write(std::ostream& out) const;

    /** Save in a compact platform-independent text format.
        The data is written in a single line, without trailing newline.
    */
    void SaveAsText(std::ostream& out) const;

    /** Load from text format.
        See SaveAsText()
    */
    void LoadFromText(std::istream& in);

private:
    COUNT m_count;

    VALUE m_mean;
};

template<typename VALUE, typename COUNT>
SgStatisticsBase<VALUE,COUNT>::SgStatisticsBase()
{
    Clear();
}

template<typename VALUE, typename COUNT>
SgStatisticsBase<VALUE,COUNT>::SgStatisticsBase(VALUE val, COUNT count)
{
    Initialize(val, count);
}

template<typename VALUE, typename COUNT>
void SgStatisticsBase<VALUE,COUNT>::Add(VALUE val)
{
    // Write order dependency: at least on class (SgUctSearch in lock-free
    // mode) uses SgStatisticsBase concurrently without locking and assumes
    // that m_mean is valid, if m_count is greater zero
    COUNT count = m_count;
    ++count;
    SG_ASSERT(count > 0); // overflow
    val -= m_mean;
    m_mean +=  val / count;
    m_count = count;
}

template<typename VALUE, typename COUNT>
void SgStatisticsBase<VALUE,COUNT>::Clear()
{
    m_count = 0;
    m_mean = 0;
}

template<typename VALUE, typename COUNT>
const COUNT& SgStatisticsBase<VALUE,COUNT>::Count() const
{
    return m_count;
}

template<typename VALUE, typename COUNT>
void SgStatisticsBase<VALUE,COUNT>::Initialize(VALUE val, COUNT count)
{
    m_count = count;
    m_mean = val;
}

template<typename VALUE, typename COUNT>
void SgStatisticsBase<VALUE,COUNT>::LoadFromText(std::istream& in)
{
    in >> m_count >> m_mean;
}

template<typename VALUE, typename COUNT>
const VALUE& SgStatisticsBase<VALUE,COUNT>::Mean() const
{
    SG_ASSERT(m_count > 0);
    return m_mean;
}

template<typename VALUE, typename COUNT>
void SgStatisticsBase<VALUE,COUNT>::Write(std::ostream& out) const
{
    if (m_count == 0)
        out << '-';
    else
        out << Mean();
}

template<typename VALUE, typename COUNT>
void SgStatisticsBase<VALUE,COUNT>::SaveAsText(std::ostream& out) const
{
    out << m_count << ' ' << m_mean;
}

//----------------------------------------------------------------------------

/** Computes mean and variance of a statistical variable.
    The template parameters are the floating point type and the counter type,
    depending on the precision-memory tradeoff.
*/
template<typename VALUE, typename COUNT>
class SgStatistics
    : public SgStatisticsBase<VALUE,COUNT>
{
public:
    SgStatistics();

    /** Create statistics initialized with values.
        Equivalent to creating a statistics and calling @c count times
        Add(val)
    */
    SgStatistics(VALUE val, COUNT count);

    void Add(VALUE val);

    void Clear();

    VALUE Deviation() const;

    VALUE Variance() const;

    /** Write in human readable format. */
    void Write(std::ostream& out) const;

    /** Save in a compact platform-independent text format.
        The data is written in a single line, without trailing newline.
    */
    void SaveAsText(std::ostream& out) const;

    /** Load from text format.
        See SaveAsText()
    */
    void LoadFromText(std::istream& in);

private:
    VALUE m_variance;
};

template<typename VALUE, typename COUNT>
SgStatistics<VALUE,COUNT>::SgStatistics()
{
    Clear();
}

template<typename VALUE, typename COUNT>
SgStatistics<VALUE,COUNT>::SgStatistics(VALUE val, COUNT count)
    : SgStatisticsBase<VALUE,COUNT>(val, count)
{
    m_variance = 0;
}

template<typename VALUE, typename COUNT>
void SgStatistics<VALUE,COUNT>::Add(VALUE val)
{
    COUNT countOld = SgStatisticsBase<VALUE,COUNT>::Count();
    if (countOld > 0)
    {
        VALUE meanOld = SgStatisticsBase<VALUE,COUNT>::Mean();
        SgStatisticsBase<VALUE,COUNT>::Add(val);
        VALUE mean = SgStatisticsBase<VALUE,COUNT>::Mean();
        COUNT count = SgStatisticsBase<VALUE,COUNT>::Count();
        m_variance = (countOld * (m_variance + meanOld * meanOld)
                      + val * val) / count  - mean * mean;
    }
    else
    {
        SgStatisticsBase<VALUE,COUNT>::Add(val);
        m_variance = 0;
    }
}

template<typename VALUE, typename COUNT>
void SgStatistics<VALUE,COUNT>::Clear()
{
    SgStatisticsBase<VALUE,COUNT>::Clear();
    m_variance = 0;
}

template<typename VALUE, typename COUNT>
VALUE SgStatistics<VALUE,COUNT>::Deviation() const
{
    return std::sqrt(m_variance);
}

template<typename VALUE, typename COUNT>
void SgStatistics<VALUE,COUNT>::LoadFromText(std::istream& in)
{
    SgStatisticsBase<VALUE,COUNT>::LoadFromText(in);
    in >> m_variance;
}

template<typename VALUE, typename COUNT>
VALUE SgStatistics<VALUE,COUNT>::Variance() const
{
    return m_variance;
}

template<typename VALUE, typename COUNT>
void SgStatistics<VALUE,COUNT>::Write(std::ostream& out) const
{
    if (SgStatisticsBase<VALUE,COUNT>::Count() == 0)
        out << '-';
    else
        out << SgStatisticsBase<VALUE,COUNT>::Mean() << " dev="
            << Deviation();
}

template<typename VALUE, typename COUNT>
void SgStatistics<VALUE,COUNT>::SaveAsText(std::ostream& out) const
{
    SgStatisticsBase<VALUE,COUNT>::SaveAsText(out);
    out << ' ' << m_variance;
}

//----------------------------------------------------------------------------

/** Extended version of SgStatistics.
    Also stores minimum and maximum values.
    The template parameters are the floating point type and the counter type,
    depending on the precision-memory tradeoff.
*/
template<typename VALUE, typename COUNT>
class SgStatisticsExt
    : public SgStatistics<VALUE,COUNT>
{
public:
    SgStatisticsExt();

    void Add(VALUE val);

    void Clear();

    VALUE Max() const;

    VALUE Min() const;

    void Write(std::ostream& out) const;

private:
    VALUE m_max;

    VALUE m_min;
};

template<typename VALUE, typename COUNT>
SgStatisticsExt<VALUE,COUNT>::SgStatisticsExt()
{
    Clear();
}

template<typename VALUE, typename COUNT>
void SgStatisticsExt<VALUE,COUNT>::Add(VALUE val)
{
    SgStatistics<VALUE,COUNT>::Add(val);
    if (val > m_max)
        m_max = val;
    if (val < m_min)
        m_min = val;
}

template<typename VALUE, typename COUNT>
void SgStatisticsExt<VALUE,COUNT>::Clear()
{
    SgStatistics<VALUE,COUNT>::Clear();
    m_min = std::numeric_limits<VALUE>::max();
    m_max = -std::numeric_limits<VALUE>::max();
}

template<typename VALUE, typename COUNT>
VALUE SgStatisticsExt<VALUE,COUNT>::Max() const
{
    return m_max;
}

template<typename VALUE, typename COUNT>
VALUE SgStatisticsExt<VALUE,COUNT>::Min() const
{
    return m_min;
}

template<typename VALUE, typename COUNT>
void SgStatisticsExt<VALUE,COUNT>::Write(std::ostream& out) const
{
    if (SgStatistics<VALUE,COUNT>::Count() == 0)
        out << '-';
    else
    {
        SgStatistics<VALUE,COUNT>::Write(out);
        out << " min=" << m_min << " max=" << m_max;
    }
}

//----------------------------------------------------------------------------

/** Set of named statistical variables.
    The template parameters are the floating point type and the counter type,
    depending on the precision-memory tradeoff.
*/
template<typename VALUE, typename COUNT>
class SgStatisticsCollection
{
public:
    /** Add the statistics of another collection.
        The collections must contain the same entries.
    */
    void Add(const SgStatisticsCollection<VALUE,COUNT>& collection);

    void Clear();

    bool Contains(const std::string& name) const;

    /** Create a new variable. */
    void Create(const std::string& name);

    const SgStatistics<VALUE,COUNT>& Get(const std::string& name) const;

    SgStatistics<VALUE,COUNT>& Get(const std::string& name);

    void Write(std::ostream& o) const;

private:
    typedef std::map<std::string,SgStatistics<VALUE,COUNT> > Map;

    typedef typename Map::iterator Iterator;

    typedef typename Map::const_iterator ConstIterator;

    Map m_map;
};

template<typename VALUE, typename COUNT>
void
SgStatisticsCollection<VALUE,COUNT>
::Add(const SgStatisticsCollection<VALUE,COUNT>& collection)
{
    if (m_map.size() != collection.m_map.size())
        throw SgException("Incompatible statistics collections");
    for (Iterator p = m_map.begin(); p != m_map.end(); ++p)
    {
        ConstIterator k = collection.m_map.find(p->first);
        if (k == collection.m_map.end())
            throw SgException("Incompatible statistics collections");
        p->second.Add(k->second);
    }
}

template<typename VALUE, typename COUNT>
void SgStatisticsCollection<VALUE,COUNT>::Clear()
{
    for (Iterator p = m_map.begin(); p != m_map.end(); ++p)
        p->second.Clear();
}

template<typename VALUE, typename COUNT>
bool SgStatisticsCollection<VALUE,COUNT>::Contains(const std::string& name)
    const
{
    return (m_map.find(name) != m_map.end());
}

template<typename VALUE, typename COUNT>
void SgStatisticsCollection<VALUE,COUNT>::Create(const std::string& name)
{
    m_map[name] = SgStatistics<VALUE,COUNT>();
}

template<typename VALUE, typename COUNT>
const SgStatistics<VALUE,COUNT>&
SgStatisticsCollection<VALUE,COUNT>::Get(const std::string& name) const
{
    ConstIterator p = m_map.find(name);
    if (p == m_map.end())
    {
        std::ostringstream o;
        o << "Unknown statistics name " << name << '.';
        throw SgException(o.str());
    }
    return p->second;
}

template<typename VALUE, typename COUNT>
SgStatistics<VALUE,COUNT>&
SgStatisticsCollection<VALUE,COUNT>::Get(const std::string& name)
{
    Iterator p = m_map.find(name);
    if (p == m_map.end())
    {
        std::ostringstream o;
        o << "Unknown statistics name " << name << '.';
        throw SgException(o.str());
    }
    return p->second;
}

template<typename VALUE, typename COUNT>
void SgStatisticsCollection<VALUE,COUNT>::Write(std::ostream& o) const
{
    for (ConstIterator p = m_map.begin(); p != m_map.end(); ++p)
        o << p->first << ": " << p->second.Write(o) << '\n';
}

//----------------------------------------------------------------------------

/** Histogram.
    The template parameters are the floating point type and the counter type,
    depending on the precision-memory tradeoff.
*/
template<typename VALUE, typename COUNT>
class SgHistogram
{
public:
    SgHistogram();

    SgHistogram(VALUE min, VALUE max, int bins);

    /** Reinitialize and clear histogram. */
    void Init(VALUE min, VALUE max, int bins);

    void Add(VALUE value);

    void Clear();

    int Bins() const;

    COUNT Count() const;

    /** Get count in a certain bin. */
    COUNT Count(int i) const;

    /** Write as x,y-table.
        Writes the historgram in a format that likely can be used by other
        programs. Writes one x,y pair per line. The separator is TAB.
        The x-values are the left border values of the bins, the y-values
        are the counts of the bins.
    */
    void Write(std::ostream& out) const;

    /** Write with labels.
        Example output with label "Value", the numbers in brackets are the
        left border of each bin:
        @verbatim
        Value[0]  100
        Value[10] 2000
        Value[20] 500
        @endverbatim
    */
    void WriteWithLabels(std::ostream& out, const std::string& label) const;

private:
    typedef std::vector<COUNT> Vector;

    int m_bins;

    COUNT m_count;

    VALUE m_binSize;

    VALUE m_min;

    VALUE m_max;

    Vector m_array;
};

template<typename VALUE, typename COUNT>
SgHistogram<VALUE,COUNT>::SgHistogram()
{
    Init(0, 1, 1);
}

template<typename VALUE, typename COUNT>
SgHistogram<VALUE,COUNT>::SgHistogram(VALUE min, VALUE max, int bins)
{
    Init(min, max, bins);
}

template<typename VALUE, typename COUNT>
void SgHistogram<VALUE,COUNT>::Add(VALUE value)
{
    ++m_count;
    int i = static_cast<int>((value - m_min) / m_binSize);
    if (i < 0)
        i = 0;
    if (i >= m_bins)
        i = m_bins - 1;
    ++m_array[i];
}

template<typename VALUE, typename COUNT>
int SgHistogram<VALUE,COUNT>::Bins() const
{
    return m_bins;
}

template<typename VALUE, typename COUNT>
void SgHistogram<VALUE,COUNT>::Clear()
{
    m_count = 0;
    for (typename Vector::iterator it = m_array.begin(); it != m_array.end();
         ++ it)
        *it = 0;
}

template<typename VALUE, typename COUNT>
COUNT SgHistogram<VALUE,COUNT>::Count() const
{
    return m_count;
}

template<typename VALUE, typename COUNT>
COUNT SgHistogram<VALUE,COUNT>::Count(int i) const
{
    SG_ASSERT(i >= 0);
    SG_ASSERT(i < m_bins);
    return m_array[i];
}

template<typename VALUE, typename COUNT>
void SgHistogram<VALUE,COUNT>::Init(VALUE min, VALUE max, int bins)
{
    m_array.resize(bins);
    m_min = min;
    m_max = max;
    m_bins = bins;
    m_binSize = (m_max - m_min) / m_bins;
    Clear();
}

template<typename VALUE, typename COUNT>
void SgHistogram<VALUE,COUNT>::Write(std::ostream& out) const
{
    for (int i = 0; i < m_bins; ++i)
        out << (m_min + i * m_binSize) << '\t' << m_array[i] << '\n';

}

template<typename VALUE, typename COUNT>
void SgHistogram<VALUE,COUNT>::WriteWithLabels(std::ostream& out,
                                             const std::string& label) const
{
    for (int i = 0; i < m_bins; ++i)
    {
        std::ostringstream binLabel;
        binLabel << label << '[' << (m_min + i * m_binSize) << ']';
        out << SgWriteLabel(binLabel.str()) << m_array[i] << '\n';
    }
}

//----------------------------------------------------------------------------

#endif // SG_STATISTICS_H
