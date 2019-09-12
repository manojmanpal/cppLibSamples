//
// [M]
// Makes use of UTF16 based value/document/stringbuffer and writes/modifies json document.
//

#include "pch.h"
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON ver. 1.10
#include <cstdio>
#include <string>

using namespace rapidjson;
using namespace std;

int main(int, wchar_t*[]) {
	////////////////////////////////////////////////////////////////////////////
	// 1. Parse a JSON text string to a document.

	typedef GenericDocument<UTF16<> > WDocument;
	typedef GenericValue<UTF16<> > WValue;
	typedef GenericStringBuffer<UTF16<>> WStringBuffer;
	WDocument document;  // Default template parameter uses UTF16 and MemoryPoolAllocator.
	document.SetObject();
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

	WValue vString;
	document.AddMember(L"string1", vString, allocator);
	document[L"string1"].SetString(L"rapidjson");
	document.AddMember(L"string2", vString, allocator);
	document[L"string2"].SetString(L"rapidjson2");
	document.AddMember(L"Bool", WValue().SetBool(true), allocator);

	WValue vArrayType(kArrayType);
	for (int i = 0; i < 2; i++)
	{
		WValue vArrayObj(kObjectType);
		vArrayObj.SetObject();
		WValue vValueType;
		vArrayObj.AddMember(L"stringInArrObj", vValueType, allocator);
		vArrayObj[L"stringInArrObj"].SetString(L"val1");

		vArrayObj.AddMember(L"JobId", vValueType, allocator);
		vArrayObj[L"JobId"].SetInt(i + 1);
		
		vArrayType.PushBack(vArrayObj, allocator);
	}
	document.AddMember(L"arrayRoot", vArrayType, allocator);

	WStringBuffer sb;
	PrettyWriter<WStringBuffer, rapidjson::UTF16<wchar_t>> writer(sb);
	document.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
	wprintf(L"New JSON:\n %s\n", sb.GetString());

	wstring strCharOut = sb.GetString();

	if(document.Parse(strCharOut.c_str()).HasParseError())
		return 1;

	wprintf(L"\nParsing to document succeeded.\n");

	////////////////////////////////////////////////////////////////////////////
	// 2. Access values in document and change value inside array. 

	wprintf(L"\nAccess values in document:\n");
	assert(document.IsObject());    // Document is a JSON value represents the root of DOM. Root can be either an object or array.

	WValue& arrayRoot = document[L"arrayRoot"];
	assert(arrayRoot.IsArray());
	for (SizeType i = 0; i < arrayRoot.Size(); i++)
	{
		assert(arrayRoot[i].HasMember(L"JobId"));
		const WValue& jobId = arrayRoot[i][L"JobId"];
		assert(jobId.IsInt());
		if (jobId.GetInt() == 2) //If job Id is 2 replace it
		{
			int oldJobId =  arrayRoot[i][L"JobId"].GetInt();
			arrayRoot[i][L"JobId"].SetInt(jobId.GetInt() + 1);
			wprintf(L"Changed value for old JobId = %d is : %d \n", oldJobId, arrayRoot[i][L"JobId"].GetInt());

			assert(arrayRoot[i][L"stringInArrObj"].IsString());
			const WValue& strinForJobIdPrescribed = arrayRoot[i][L"stringInArrObj"];
			arrayRoot[i][L"stringInArrObj"].SetString(L"newVal");
		}
	}
	WStringBuffer sb2;
	PrettyWriter<WStringBuffer, rapidjson::UTF16<wchar_t>> writerNew(sb2);
	document.Accept(writerNew);    // Accept() traverses the DOM and generates Handler events.
	wprintf(L"Modified JSON:\n %s\n", sb2.GetString());

	return 0;
}