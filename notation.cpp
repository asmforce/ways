#include "notation.hpp"
#include <iostream> // FIXME: debug only

namespace notation
{
  namespace internals
  {
    bool spaces( char value, u32 index )  {
      return isspace( value );
    }

    bool id( char value, u32 index )  {
      return (index > 0 ? isalnum(value) : isalpha(value)) || value == '_';
    }
  }


  Source::Source( std::istream &stream )
    : mStream(stream), mRollbackable(false), mBegin(0), mFront(0), mLine(0), mColumn(0), mOk(true)
  {
  }


  void Source::begin( u32 &line, u32 &column, bool rollbackable )
  {
    begin( rollbackable );

    line = mLine + 1;
    column = mColumn + 1;
  }


  void Source::begin( bool rollbackable )
  {
    if( mRollbackable )
    {
      while( mBegin < mFront )
      {
        commit( mBuffer[mBegin] );
        mBegin++;
      }

      if( mBegin > 256 )
      {
        mBuffer.erase( mBuffer.begin(), mBuffer.begin()+mBegin );
        mBegin = mFront = 0;
      }
    }

    mRollbackable = rollbackable;
  }


  u32 Source::rollbackable() const
  {
    return mFront - mBegin;
  }


  bool Source::rollback( u32 count )
  {
    if( mFront - mBegin < count )
      return false;
    if( count )
      mFront -= count;
    else
      mFront = mBegin;
    return true;
  }


  bool Source::get( char &value )
  {
    if( mFront < mBuffer.size() )
    {
      value = mBuffer[mFront++];
      if( mRollbackable == false )
      {
        commit( value );
        mBegin = mFront;
      }
      return true;
    } else {
      if( mStream.get(value) )
      {
        if( mRollbackable )
        {
          mBuffer += value;
          mFront++;
        } else {
          commit( value );
        }
        return true;
      }
      return false;
    }
  }


  bool Source::get( std::string &value, predicate &p )
  {
    u32 index = 0;
    char c;

    value.clear();
    while( mFront < mBuffer.size() )
    {
      c = mBuffer[mFront];
      if( p(c,index) )
      {
        mFront++;
        index++;
        value += c;
      } else {
        return index;
      }
    }

    if( mRollbackable )
    {
      while( mStream.get(c) )
      {
        if( p(c,index) )
        {
          index++;
          value += c;
          mBuffer += c;
          mFront++;
        } else {
          mStream.unget();
          return index;
        }
      }
    } else {
      while( mBegin < mFront )
      {
        commit( mBuffer[mBegin] );
        mBegin++;
      }

      while( mStream.get(c) )
      {
        if( p(c,index) )
        {
          index++;
          value += c;
          commit( c );
        } else {
          mStream.unget();
          return index;
        }
      }
    }

    return index;
  }


  bool Source::end() const
  {
    return mStream.peek() < 0;
  }


  std::string Source::lexeme() const
  {
    return mBuffer.substr( mBegin, mFront-mBegin );
  }


  bool Source::ok() const
  {
    return mOk;
  }


  void Source::ok( bool success )
  {
    mOk = success;
  }


  void Source::commit( char c )
  {
    if( c == '\n' )
    {
      mLine++;
      mColumn = 0;
    } else {
      mColumn++;
    }
  }


  source &operator >> ( source &src, ws manip )
  {
    if( false == src.ok() )
      return src;

    std::string s;
    src.begin( false );
    src.get( s, internals::spaces );

    return src;
  }


  bool operator >> ( source &src, bool manip )
  {
    bool success = src.ok();
    src.ok( true );

    return success == manip;
  }


  source &operator >> ( source &src, pos manip )
  {
    src.begin( manip.line, manip.column );
    return src;
  }


  source &operator >> ( source &src, id manip )
  {
    if( false == src.ok() )
      return src;

    src.begin( false );
    if( false == src.get(manip.value, internals::id) )
    {
      src.rollback();
      src.ok( false );
    }

    return src;
  }


  source &operator >> ( source &src, keyword manip )
  {
    if( false == src.ok() )
      return src;

    std::string s;
    src.begin( true );
    src.get( s, internals::id );
    if( manip.value != s )
    {
      src.rollback();
      src.ok( false );
    }

    return src;
  }


  source &operator >> ( source &src, str manip )
  {
    if( false == src.ok() )
      return src;

    char c;

    manip.value.clear();
    src.begin( true );
    if( false == src.get(c) || c != '\"' )
    {
      src.rollback();
      src.ok( false );
      return src;
    }

    while( src.get(c) )
    {
      if( c == '\"' )
        return src;

      if( c == '\\' )
      {
        if( false == src.get(c) )
        {
          src.rollback();
          src.ok( false );
          return src;
        }

        switch( c )
        {
        case 'n':
          manip.value += '\n';
          continue;
        case 'r':
          manip.value += '\r';
          continue;
        case 't':
          manip.value += '\t';
          continue;
        case 'f':
          manip.value += '\f';
          continue;
        case 'v':
          manip.value += '\v';
          continue;
        }
      }
      
      manip.value += c;
    }

    src.rollback();
    src.ok( false );
    return src;
  }


  source &operator >> ( source &src, delim manip )
  {
    if( false == src.ok() )
      return src;

    char c;
    src.begin( true );
    if( src.get(c) )
    {
      if( manip.value != c )
      {
        src.rollback();
        src.ok( false );
      }
    } else {
      src.ok( false );
    }

    return src;
  }
}
