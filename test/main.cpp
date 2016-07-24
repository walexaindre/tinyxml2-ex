/*
tinyxml2ex - a set of add-on classes and helper functions bringing C++11/14 features, such as iterators, strings and exceptions, to tinyxml2


Copyright (c) 2016 Stan Thomas

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.


tinyxml2 is the work of Lee Thomason (www.grinninglizard.com) and others.
It can be found here: https://github.com/leethomason/tinyxml2 and has it's own licensing terms.

*/


#include <string>
#include <iostream>
#include <algorithm>
#include <conio.h>


// include the header for tinyxml2ex which includes tinyxml2, remember to put them on your include path
#include <tixml2ex.h>

using namespace std;
using namespace std::literals::string_literals;


int main()
{
	// a simple XML document
	string testXml {R"-(
<?xml version="1.0" encoding="UTF-8"?>
<A>
	<B id="one">
		<C code="1234">
			A-B(one)-C.1234
		</C>
		<C code="5678">
			<![CDATA[A-B(one)-C.5678]]>
		</C>
		<C code="9ABC">A-B(one)-C.9ABC</C>
		<D code="9ABC" id="dd" />
	</B>
	<B id="two">
		<D id="dd" />
	</B>
	<B id="three" org="extern">
		<C code="1234">
			A-B(three)-C.1234
		</C>
		<C code="9ABC">A-B(three)-C.9ABC</C>
		<D description="A-B(three)-D.9ABC" />
	</B>
	<B id="four">
	</B>
</A>
)-"s};


	// these three blocks are equivalent and demonstrate different ways to iterate over
	// all <C> element children of <B> element children of the document element <A>
	// 1) native TinyXML2
	{
		printf ("\n1)   <C> element children of <B> element children of the document element <A>\nnative TinyXML2\n");
		tinyxml2::XMLDocument doc;
		if (doc .Parse (testXml .c_str()) == 0/*XML_NO_ERORR*/)
		{
			tinyxml2::XMLElement * eA = doc .FirstChildElement();
			if (eA)
			{
				tinyxml2::XMLElement * eB = eA -> FirstChildElement("B");
				while (eB)
				{
					tinyxml2::XMLElement * eC = eB -> FirstChildElement ("C");
					while (eC)
					{
						printf ("%s = %s\n", eC -> Name(), eC -> GetText());
						eC = eC -> NextSiblingElement ("C");
					}
					eB = eB -> NextSiblingElement ("B");
				}
			}
			else
				printf ("unable to load XML document\n");
		}
	}
	cout << "----" << endl << endl;


	// 2) tixml2ex XPath selector
	cout << "2)   <C> element children of <B> element children of the document element <A>" << endl
		<< "tixml2ex XPath selector" << endl;
	try
	{
		auto doc = tinyxml2::load_document (testXml);
		for (auto eC : tinyxml2::Selector (*doc, "A/B/C"))
			cout << eC -> Name() << " = " << text (eC) << endl;
	}
	catch (tinyxml2::XmlException & e)
	{
		cout << "XmlException caught" << e .what() << endl;
	}
	cout << "----" << endl << endl;


	// 3) simple tixml2ex element iterator
	cout << endl << "3)   <C> element children of <B> element children of the document element <A>" << endl
		<< "simple tixml2ex element iterator" << endl;
	try
	{
		auto doc = tinyxml2::load_document (testXml);
		auto eA = doc -> FirstChildElement();
		if (eA)
			for (auto eB : eA)
			{
				// just for fun, use standard algorithm for_each to iterate over the children of <B>
				for_each (begin (eB), end (eB),
					[](auto e)
				{
						// simple iterators are just that, they iterate over all children
						// therefore we must test element name (type) to examine only <C> elements
					if (strcmp (e -> Name(), "C") == 0) cout << e -> Name() << " = " << text (e) << endl;
				});
			}
	}
	catch (tinyxml2::XmlException & e)
	{
		cout << "XmlException caught" << e .what() << endl;
	}

	cout << "=================================================" << endl << endl;


	// additional element selection and iteration using XPath syntax
	// illustrating helper functions
	try
	{
		auto doc = tinyxml2::load_document (testXml);

		// find first matching an element in the document
		cout << "find an element by attribute value" << endl;
		auto bThree = find_element (*doc, "A/B[@id='three']"s);
		cout << attribute_value (bThree, "id"s) << " - " << attribute_value (bThree, "org"s) << endl;
		cout << "=================================================" << endl << endl;


		// find the first instance of a child element of the selected type
		cout << "get description attribute of <D> element" << endl;
		if (auto ch1 = find_element (bThree, "D"s))
			cout << attribute_value (ch1, "description"s) << text (ch1) << endl << endl;
		cout << "=================================================" << endl << endl;

		// iterate over all <C> children of selected <B>
		cout << "iterate over all <C> children of selected <B>" << endl;
		for (auto const cc : tinyxml2::Selector (bThree, "C"s))
		{
			cout << cc -> Parent() -> ToElement() -> Name() << "[" << attribute_value (cc -> Parent() -> ToElement(), "id") << "] / " << cc -> Name() << "[@code='";
			if (!attribute_value (cc, "code") .empty())
				cout << attribute_value (cc, "code");
			else
				cout << "**element has no attribute - code**";
			cout << "']" << endl;
		}
		cout << "=================================================" << endl << endl;


		// iterate over all elements in the document along the selected path by starting with '/'
		// note that because path starts from the document, bThree is used only as an element in the document not the root of the search
		cout << "iterate over all <C> children : /A/B[@id='three']/C" << endl;
		int nC = 0;
		for (auto const cc : tinyxml2::Selector (bThree, "/A/B[@id='three']/C"s))
		{
			++nC;
			cout << cc -> Parent() -> ToElement() -> Name() << "[" << attribute_value (cc -> Parent() -> ToElement(), "id") << "] / " << cc -> Name() << "[@code='";
			if (!attribute_value (cc, "code") .empty())
				cout << attribute_value (cc, "code");
			else
				cout << "**element has no attribute - code**";
			cout << "']" << endl;
		}
		cout << nC << " Cs in B[@id='three']" << endl << endl;
		cout << "=================================================" << endl << endl;

		// iterate over all children, any name (type), with code attribute value
		cout << "iterate over all children of any name (type) : /A/B/[@code='9ABC']" << endl;
		for (auto const cc : tinyxml2::Selector (bThree, "/A/B/[@code='9ABC']"s))
		{
			cout << cc -> Parent() -> ToElement() -> Name() << "[" << attribute_value (cc -> Parent() -> ToElement(), "id") << "] / " << cc -> Name() << "[@code='";
			if (!attribute_value (cc, "code") .empty())
				cout << attribute_value (cc, "code");
			else
				cout << "**element has no attribute - code**";
			cout << "']" << endl;
		}
		cout << "=================================================" << endl << endl;


		// find the first instance of a child element of the selected type with a matching attribute value
		cout << "find C[@code='9ABC'] within B[@id='three']" << endl;
		if (auto const cc = find_element (bThree, "C[@code='9ABC']"s))
			cout << text (cc) << " , " << attribute_value (cc, "description") << endl << endl;
		else
			cout << "could not find C[@code='9ABC'] in B" << endl;
		cout << "=================================================" << endl << endl;

		// find the same element within document
		cout << "find B[@id='three']/C[@code='9ABC']" << endl;
		if (auto const cc = find_element (*doc, "/A/B[@id='three']/C[@code='9ABC']"s))
			cout << text (cc) << " , " << attribute_value (cc, "description") << endl << endl;
		else
			cout << "could not find A/B[@id='three']/C[@code='9ABC'] in document" << endl;
		cout << "=================================================" << endl << endl;


	}
	catch (tinyxml2::XmlException & e)
	{
		cout << e .what() << endl;
	}


	// hold console window open so we can see the output
	std::cout << "hit any key to close" << std::endl;
	auto c = _getch();

	return 0;
}
