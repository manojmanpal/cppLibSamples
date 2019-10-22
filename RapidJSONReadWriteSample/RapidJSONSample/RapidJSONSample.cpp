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

	//Object inside object response string.
	try
	{
		WValue vObject;
		WValue errorRespObject;
		wstring testErr = L"testError";
		errorRespObject.SetString(testErr.c_str(),
			static_cast<SizeType>(wcslen((testErr).c_str())), allocator);
		vObject.SetObject(); //Comment to test RapidJSON exception
		vObject.AddMember(L"Errors", errorRespObject, allocator);		
		document.AddMember(L"ResultSet", vObject, allocator);
	}
	catch (...)
	{
		
	}

	WValue vString;
	document.AddMember(L"string1", vString, allocator);
	document[L"string1"].SetString(L"rapidjson");
	document.AddMember(L"string2", vString, allocator);
	document[L"string2"].SetString(L"rapidjson2");
	document.AddMember(L"Bool", WValue().SetBool(true), allocator);
	document.AddMember(L"string3", vString, allocator);
	document[L"string3"].SetString(L"test");

	wstring test = L"test";
	vString.SetString(test.c_str(), static_cast<SizeType>(wcslen((test).c_str())));
	document.AddMember(L"CurrentState", vString, allocator);

	WValue vArrayType(kArrayType);
	for (int i = 0; i < 2; i++)
	{
		WValue vArrayObj(kObjectType);
		vArrayObj.SetObject();
		WValue vValueType;
		vArrayObj.AddMember(L"stringInArrObj", vValueType, allocator);
		vArrayObj[L"stringInArrObj"].SetString(L"val1");

		vArrayObj.AddMember(L"Id", vValueType, allocator);
		vArrayObj[L"Id"].SetInt(i + 1);
		
		WValue vArrayDetails(kArrayType);
		for (int i = 0; i < 2; i++)
		{
			WValue vValueTypeDetails;
			vValueTypeDetails.SetInt64(i);
			vArrayDetails.PushBack(vValueTypeDetails, allocator);
		}
		vArrayObj.AddMember(L"ArrayElems", vArrayDetails, allocator);

		WValue vArrayTypeDetails(kArrayType);
		//Array inside array of objects.
		for (int i = 0; i < 2; i++)
		{
			WValue vArrayObjDetails(kObjectType);
			WValue vValueTypeDetails;
			vArrayObjDetails.AddMember(L"stringInArrObj", vValueTypeDetails, allocator);
			vArrayObjDetails[L"stringInArrObj"].SetString(L"val1");
			vArrayTypeDetails.PushBack(vArrayObjDetails.Move(), allocator);
		}
		vArrayObj.AddMember(L"DetailsObject", vArrayTypeDetails, allocator);

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
	// 2. Access values in document and change value inside array Id. 

	wprintf(L"\nAccess values in document:\n");
	assert(document.IsObject());    // Document is a JSON value represents the root of DOM. Root can be either an object or array.

	WValue& arrayRoot = document[L"arrayRoot"];
	assert(arrayRoot.IsArray());
	for (SizeType i = 0; i < arrayRoot.Size(); i++)
	{
		assert(arrayRoot[i].HasMember(L"Id"));
		const WValue& Id = arrayRoot[i][L"Id"];
		assert(Id.IsInt());
		if (Id.GetInt() == 2) //If  Id is 2 replace it
		{
			int oldId =  arrayRoot[i][L"Id"].GetInt();
			arrayRoot[i][L"Id"].SetInt(Id.GetInt() + 1);
			wprintf(L"Changed value for old Id = %d is : %d \n", oldId, arrayRoot[i][L"Id"].GetInt());

			assert(arrayRoot[i][L"stringInArrObj"].IsString());
			const WValue& strinForIdPrescribed = arrayRoot[i][L"stringInArrObj"];
			arrayRoot[i][L"stringInArrObj"].SetString(L"newVal");
		}
	}
	WStringBuffer sb2;
	PrettyWriter<WStringBuffer, rapidjson::UTF16<wchar_t>> writerNew(sb2);
	document.Accept(writerNew);    // Accept() traverses the DOM and generates Handler events.
	wprintf(L"Modified JSON:\n %s\n", sb2.GetString());

	return 0;
}