// BoostLogSampleFromSinks.cpp : This file contains the 'main' function. Program execution begins and ends there.
// TODO: Boost deps has not been included here. 

#include "pch.h"
/**
 * A simple sample of Boost Log
 */
#include <windows.h>
#pragma warning(push)
//#pragma warning(disable:4819)

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/current_function.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/log/sinks.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
//
#pragma warning(pop)
//#pragma warning(disable:4503)
namespace attrs = boost::log::attributes;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
using namespace std;

//enum severity_level
//{
//	normal,
//	notification,
//	warning,
//	error,
//	critical
//};
boost::log::sources::severity_logger< boost::log::trivial::severity_level > lg;

// Convert file path to only the filename
std::string path_to_filename(std::string path) {
	return path.substr(path.find_last_of("/\\") + 1);
}

#define logInfo(message)                          \
  LOG_LOCATION;                                               \
  BOOST_LOG_SEV(lg, boost::log::trivial::severity_level::info) << message

#define LOG_LOCATION                            \
  boost::log::attribute_cast<boost::log::attributes::mutable_constant<int>>(boost::log::core::get()->get_global_attributes()["Line"]).set(__LINE__); \
  boost::log::attribute_cast<boost::log::attributes::mutable_constant<std::string>>(boost::log::core::get()->get_global_attributes()["File"]).set(path_to_filename(__FILE__)); 


// Logging macro that includes severity, filename, and line number.
// Copied and modified from
// https://stackoverflow.com/questions/24750218/boost-log-to-print-source-code-file-name-and-line-number
// Set attribute and return the new value
template <typename ValueType>
ValueType SetGetAttrib(const char* name, ValueType value) {
	auto attr = boost::log::attribute_cast<
		boost::log::attributes::mutable_constant<ValueType>>(
			boost::log::core::get()->get_global_attributes()[name]);
	attr.set(value);
	return attr.get();
}

static std::string PathToFilename(const std::string& path) {
	std::string sub_path = path.substr(path.find_last_of("/\\") + 1);
	return sub_path;
}


static std::wstring GetHxWinAgentLogsFolder()
{
	TCHAR tcRoot[MAX_PATH];
	GetWindowsDirectory(tcRoot, MAX_PATH);
	tcRoot[3] = 0;
	wstring OutputFolder = (wstring)tcRoot + L"Logs";
	if (CreateDirectory(OutputFolder.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		return OutputFolder;
	}
	else
		return L"";
}

static void init_log(void)
{
	/* init boost log
	 * 1. Add common attributes
	 * 2. set log filter to trace
	 */
	boost::log::add_common_attributes();
	boost::log::core::get()->add_global_attribute("Scope",
		boost::log::attributes::named_scope());
	boost::log::core::get()->set_filter(
		boost::log::trivial::severity >= boost::log::trivial::info
	);

	boost::log::core::get()->add_global_attribute("Line", boost::log::attributes::mutable_constant<int>(5));
	boost::log::core::get()->add_global_attribute("File", boost::log::attributes::mutable_constant<std::string>(""));

	/* log formatter:
	 * [TimeStamp] |(ProcessID)-(ThreadId)|[Severity Level] [Scope] Log message
	 */
	auto fmtTimeStamp = boost::log::expressions::
		format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S");
	auto fmtTProcId = boost::log::expressions::
		attr<boost::log::attributes::current_process_id::value_type>("ProcessID");
	auto fmtThreadId = boost::log::expressions::
		attr<boost::log::attributes::current_thread_id::value_type>("ThreadID");
	auto fmtSeverity = boost::log::expressions::
		attr<boost::log::trivial::severity_level>("Severity");
	auto fmtScope = boost::log::expressions::format_named_scope("Scope",
		boost::log::keywords::format = "%n(%f:%l)",
		boost::log::keywords::iteration = boost::log::expressions::reverse,
		boost::log::keywords::depth = 2);
	boost::log::formatter logFmt =
		boost::log::expressions::format("[%1%]|(%2%)-(%3%)| (%4%) [%5%]: %6%")
		% fmtTimeStamp % fmtThreadId % fmtTProcId % fmtSeverity % fmtScope
		% boost::log::expressions::smessage;

	/* console sink */
	/*auto consoleSink = boost::log::add_console_log(std::clog);
	consoleSink->set_formatter(logFmt);*/


	std::wstring fileName = GetHxWinAgentLogsFolder() + L"/Provider_%Y-%m-%d_%2N.log";

	auto fsSink = boost::log::add_file_log(
		boost::log::keywords::file_name = fileName,
		boost::log::keywords::rotation_size = 10 * 1024 * 1024,
		boost::log::keywords::target = GetHxWinAgentLogsFolder(),
		boost::log::keywords::open_mode = std::ios_base::app);
	fsSink->set_formatter(logFmt);
	fsSink->locked_backend()->auto_flush(true);
	
}

static void Test(void)
{
	BOOST_LOG_FUNCTION();
	BOOST_LOG_TRIVIAL(info) << "Info Log in Test()";
}

int main(int argc, char **argv)
{
	
	BOOST_LOG_FUNCTION();
	init_log();
	logInfo("Hello"); //Macro based wrapping for regular logging with line/file . Does not require BOOST_LOG_FUNCTION 
	
	int i = 10;
	while (i < 100)
	{
		Sleep(500);
		BOOST_LOG_TRIVIAL(info) << "Info Log";
		Test();
		BOOST_LOG_TRIVIAL(info) << "app exiting";
	}
	return 0;
}