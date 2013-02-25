#ifndef NOTATION_HPP
#define NOTATION_HPP

#include <istream>
#include <string>
#include <elib/aliases.hpp>

namespace notation
{
  using namespace elib::aliases;

  namespace internals
  {
    bool spaces( char value, u32 index );
    bool id( char value, u32 index );
  }

  class Source
  {
  public:
    typedef bool (predicate)( char value, u32 index );

  private:
    Source( const Source & );
    Source &operator = ( const Source & );

  public:
    Source( std::istream &stream );

    void begin( u32 &line, u32 &column, bool rollbackable = true );
    void begin( bool rollbackable = true );
    bool rollback( u32 count = 0 );
    u32 rollbackable() const;

    bool get( char &value );
    bool get( std::string &value, predicate &p );
    bool end() const;

    std::string lexeme() const;

    bool ok() const;
    void ok( bool success );

  protected:
    void commit( char c );

  protected:
    std::istream &mStream;
    std::string mBuffer;
    bool mRollbackable;
    u32 mBegin;
    u32 mFront;
    u32 mLine, mColumn;
    bool mOk;
  };

  typedef Source source;

  struct ws  {};

  source &operator >> ( source &src, ws manip );

  bool operator >> ( source &src, bool manip );

  struct pos
  {
    pos( u32 &line, u32 &column ) : line(line), column(column)  {}
    u32 &line, &column;
  };

  source &operator >> ( source &src, pos manip );

  struct id
  {
    id( std::string &v ) : value(v)  {}
    std::string &value;
  };

  source &operator >> ( source &src, id manip );

  struct keyword
  {
    keyword( const char *v ) : value(v)  {}
    std::string value;
  };

  source &operator >> ( source &src, keyword manip );

  struct str
  {
    str( std::string &v ) : value(v)  {}
    std::string &value;
  };

  source &operator >> ( source &src, str manip );

  struct delim
  {
    delim( char v ) : value(v)  {}
    char value;
  };

  source &operator >> ( source &src, delim manip );
}

#endif // NOTATION_HPP
