#pragma once
	
#include <windows.h>
#include <ostream>
#include <sstream>
#include <string>


typedef std::basic_ostream<char, std::char_traits<char> > ostreamType;


#define REGISTER_TYPE( type )		\
	ostreamType& operator<<( type val ) { return (m_stream << val ); }

class Log
{
	std::stringstream	m_stream;

	Log() { }

	Log( const Log &_log );
	Log& operator=( const Log &_log );
public:

	static Log get()
		{ return Log(); }

	~Log() 
	{ 
		m_stream << std::endl; 
		::OutputDebugString( m_stream.str().c_str());
	}

	REGISTER_TYPE( int );
	REGISTER_TYPE( const std::string & );
	REGISTER_TYPE( const char *);
	REGISTER_TYPE( const wchar_t *);
	REGISTER_TYPE( bool );
	REGISTER_TYPE( short );
	REGISTER_TYPE( unsigned short );
	REGISTER_TYPE( unsigned int  );
	REGISTER_TYPE( long );
	REGISTER_TYPE( unsigned long );
	REGISTER_TYPE( float );
	REGISTER_TYPE( double );
	REGISTER_TYPE( long double );
	REGISTER_TYPE( void* );
	REGISTER_TYPE( __int64 );
};

#define L_ Log::get()

class TimeMeasure
{
public:
	TimeMeasure( const char * _pText = NULL ) : m_stopped( false )
	{	
		if( _pText )
			m_text = _pText;
		QueryPerformanceCounter( (LARGE_INTEGER*) &m_startTime ); 

	}

  static double now()
  {
  		__int64 time, period;
	  	QueryPerformanceCounter((LARGE_INTEGER*) &time);
		  QueryPerformanceFrequency((LARGE_INTEGER*) &period );
		  return time / (double) period;
  }

	double time()
	{
		__int64 endTime, period;
		QueryPerformanceCounter((LARGE_INTEGER*) &endTime);
		QueryPerformanceFrequency((LARGE_INTEGER*) &period );
		return (endTime - m_startTime ) / (double) period;
	}

	void stopNow()
	{
		if( m_stopped )
			assert( !"already stopped" );

		L_ << m_text.c_str() << " [sec] " << time();
		m_stopped = true;
	}

	~TimeMeasure()
	{	
		if( !m_stopped )
		{
			L_ << m_text.c_str() << " [sec] " << time();
		}
	}

	void reset( const char * _pText = NULL )
	{
		m_stopped = false;

		if( _pText )
			m_text = _pText;
		else
			m_text.clear();

		QueryPerformanceCounter( (LARGE_INTEGER*) &m_startTime ); 
	}

private:

	bool m_stopped;
	std::string m_text;
	__int64 m_startTime;
};

